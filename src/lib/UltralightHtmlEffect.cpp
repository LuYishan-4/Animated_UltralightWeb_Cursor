#include "../header/UltralightHtmlEffect.hpp"

#include "UltralightPl/WebListener.hpp"
#include "gl/GPUDriverGL.h"
#include <AppCore/Platform.h>
#include <QDBusConnection>
#include <QDebug>
#include <QProcess>
#include <Ultralight/Ultralight.h>
#include <Ultralight/platform/GPUDriver.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdatomic.h>

namespace UltralightWebCursorM {
UltralightHtmlEffect::UltralightHtmlEffect() {}

UltralightHtmlEffect::~UltralightHtmlEffect() {
  listener_.reset();
  view_ = nullptr;
  renderer_ = nullptr;
  webcall = nullptr;
  context_ = nullptr;
}

// initialize
bool UltralightHtmlEffect::initialize(const ConfigValues &uconfig,
                                      const JSONConf &data) {
  html_value_ = {.width_ = data.minWidth,
                 .height_ = data.minHeight,
                 .stride_ = 0,
                 .minwidth = data.minWidth,
                 .minheight = data.minHeight,
                 .hotspot_x_ = data.hotspotX,
                 .hotspot_y_ = data.hotspotY,
                 .m_permanentSdkPath = uconfig.sdk,
                 .html_path_ = uconfig.html,
                 .use_gpu_ = uconfig.EnableGPU};
  pending_gpu_init_ = html_value_.use_gpu_;
  std::filesystem::path sdk_dir(html_value_.m_permanentSdkPath);
  std::filesystem::path resources_dir = sdk_dir / "resources";
  if (!std::filesystem::exists(resources_dir))
    return false;

  std::vector<std::string> required_files = {"cacert.pem", "icudt67l.dat"};

  for (const auto &file_name : required_files)
    if (!std::filesystem::exists(resources_dir / file_name))
      return false;
  if (!platform_initialized_) {
    ultralight::Config config;
    if (pending_gpu_init_)
      config.face_winding = ultralight::FaceWinding::CounterClockwise;
    config.resource_path_prefix = ultralight::String("resources/");
    auto &platform = ultralight::Platform::instance();

    platform.set_config(config);
    platform.set_font_loader(ultralight::GetPlatformFontLoader());
    platform.set_file_system(ultralight::GetPlatformFileSystem(
        ultralight::String(html_value_.m_permanentSdkPath.c_str())));
    platform_initialized_ = true;
  }

  if (std::filesystem::exists(html_value_.html_path_))
    html_time_ = std::filesystem::last_write_time(html_value_.html_path_);

  return true;
}

bool UltralightHtmlEffect::ensureInitialized() {
  if (renderer_ && view_)
    return true;

  auto &platform = ultralight::Platform::instance();
  if (pending_gpu_init_ && !context_) {
    qDebug() << "[UltralightCursorEffect] intiglglglglg";
    context_ = std::make_unique<ultralight::GPUContextGL>(
        ultralight::GPUContextGL::Mode::ExternalCurrent, false, false);
  }

  if (pending_gpu_init_ && context_ && !context_->is_glad_ready()) {
    qDebug() << "[UltralightCursorEffect] glad not initialized on current "
                "Linux/KWin path";
  }

  if (pending_gpu_init_ && context_ && !context_->has_current_context()) {
    qDebug() << "[UltralightCursorEffect] GPU init deferred: no current GL "
                "context yet";
    return false;
  }

  if (pending_gpu_init_ && context_) {
    platform.set_gpu_driver(context_->driver());
  }
  renderer_ = ultralight::Renderer::Create();
  if (!renderer_)
    return false;

  ultralight::ViewConfig vc;
  vc.is_accelerated = pending_gpu_init_;
  vc.is_transparent = true;
  vc.enable_images = true;
  vc.enable_javascript = true;
  view_ = renderer_->CreateView(html_value_.width_, html_value_.height_, vc,
                                nullptr);
  if (!view_)
    return false;

  listener_ = std::make_unique<LocalLoadListener>(&is_loaded_);
  view_->set_load_listener(listener_.get());
  webcall = std::make_shared<WebCall>();
  webcall->view_ = view_;
  return load(html_value_.html_path_);
}

bool UltralightHtmlEffect::load(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    qDebug() << "[UltralightCursorEffect] Failed to open file:"
             << QString::fromStdString(path);
    return false;
  }
  qDebug() << "[UltralightCursorEffect] 5";
  std::filesystem::path p(path);
  std::string folderName = p.parent_path().filename().string();
  std::string fileUrl = "file:///" + folderName + "/index.html";
  is_loaded_ = false;
  qDebug() << "[UltralightCursorEffect] load request"
           << " | htmlPath:" << QString::fromStdString(path)
           << " | fileUrl:" << QString::fromStdString(fileUrl);
  view_->LoadURL(fileUrl.c_str());
  qDebug() << "[UltralightCursorEffect] LoadURL submitted";
  view_->set_needs_paint(true);
  return true;
}

bool UltralightHtmlEffect::resize(const int &width, const int &height) {
  if (width > html_value_.minwidth || height > html_value_.minheight)
    return false;
  view_->Resize(width, height);
  return true;
}

void UltralightHtmlEffect::reload(const ConfigValues &uconfig,
                                  const JSONConf &data) {
  html_value_ = {.width_ = data.minWidth,
                 .height_ = data.minHeight,
                 .stride_ = 0,
                 .minwidth = data.minWidth,
                 .minheight = data.minHeight,
                 .hotspot_x_ = data.hotspotX,
                 .hotspot_y_ = data.hotspotY,
                 .m_permanentSdkPath = uconfig.sdk,
                 .html_path_ = uconfig.html,
                 .use_gpu_ = uconfig.EnableGPU};
  UltralightHtmlEffect::load(html_value_.html_path_);
  UltralightHtmlEffect::resize(html_value_.width_, html_value_.height_);
}
void UltralightHtmlEffect::move(int x, int y, bool pressed) {
  if (!view_)
    return;
  if (webcall)
    webcall->move(x, y, pressed);
  view_->set_needs_paint(true);
}

void UltralightHtmlEffect::update() {
  if (!enabled_)
    return;
  if (!ensureInitialized())
    return;
  if (!renderer_ || !view_)
    return;

  renderer_->Update();
  renderer_->RefreshDisplay(0);
  renderer_->Render();

  if (context_) {
    if (auto *driver =
            dynamic_cast<ultralight::GPUDriverGL *>(context_->driver())) {
      // The View's render target is the texture consumed by KWin. Supplying
      // it here keeps driver diagnostics away from temporary filter targets.
      driver->SetDebugOutputTextureId(view_->render_target().texture_id);
      context_->BeginDrawing();
      driver->DrawCommandList();
      context_->EndDrawing();
    }
    return;
  }

  auto surface = view_->surface();
  if (!surface)
    return;
  auto bitmap_surface = dynamic_cast<ultralight::BitmapSurface *>(surface);
  if (!bitmap_surface)
    return;
  auto bitmap = bitmap_surface->bitmap();
  if (!bitmap)
    return;

  bitmap->LockPixels();
  uint8_t *raw = static_cast<uint8_t *>(bitmap->raw_pixels());
  if (raw) {
    html_value_.width_ = bitmap->width();
    html_value_.height_ = bitmap->height();
    html_value_.stride_ = bitmap->row_bytes();
    const size_t size = html_value_.stride_ * html_value_.height_;

    pixel_buffer_.resize(size);
    memcpy(pixel_buffer_.data(), raw, size);
    new_frame_ = true;
  }
  bitmap->UnlockPixels();
}

void UltralightHtmlEffect::setEnabled(bool enabled) {
  qDebug() << "[UltralightHtmlDebug] Core active status update toggled to:"
           << enabled;
  enabled_ = enabled;
}

bool UltralightHtmlEffect::isEnabled() const { return enabled_; }

bool UltralightHtmlEffect::hasNewFrame() const { return new_frame_; }

void UltralightHtmlEffect::clearNewFrame() { new_frame_ = false; }

const uint8_t *UltralightHtmlEffect::pixels() const {
  if (pixel_buffer_.empty())
    return nullptr;
  return pixel_buffer_.data();
}

unsigned int UltralightHtmlEffect::textureId() const {
  if (!context_ || !view_)
    return 0;

  auto *driver = dynamic_cast<ultralight::GPUDriverGL *>(context_->driver());
  if (!driver)
    return 0;

  const auto render_target = view_->render_target();
  if (render_target.texture_id == 0)
    return 0;

  const unsigned int resolved =
      driver->GetGLTextureId(render_target.texture_id);
  return glIsTexture(resolved) ? resolved : 0;
}

ultralight::View *UltralightHtmlEffect::view() const { return view_.get(); }

} // namespace UltralightWebCursorM
