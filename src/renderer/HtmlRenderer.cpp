#include "HtmlRenderer.hpp"

#include "LoadListener.hpp"

#include <AppCore/Platform.h>
#include <Ultralight/Ultralight.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <cstring>

namespace UltralightWebCursor {

HtmlRenderer::~HtmlRenderer() {
  if (view_)
    view_->set_load_listener(nullptr);
  loadListener_.reset();
  view_ = nullptr;
  renderer_ = nullptr;
}

bool HtmlRenderer::initialize(const ConfigValues &config,
                              const ThemeMetadata &metadata) {
  dataRoot_ = config.dataRoot;
  htmlPath_ = config.htmlPath;
  width_ = config.width;
  height_ = config.height;
  hotspotX_ = metadata.hotspotX;
  hotspotY_ = metadata.hotspotY;
  enabled_ = config.enabled;

  const QDir resources(QDir(dataRoot_).filePath(QStringLiteral("resources")));
  if (!resources.exists(QStringLiteral("cacert.pem")) ||
      !resources.exists(QStringLiteral("icudt67l.dat"))) {
    qCritical() << "[Renderer] Missing Ultralight resources in" << dataRoot_;
    return false;
  }
  if (!QFileInfo::exists(htmlPath_)) {
    qCritical() << "[Renderer] Theme entry point does not exist:" << htmlPath_;
    return false;
  }

  if (!platformInitialized_) {
    ultralight::Config ultralightConfig;
    ultralightConfig.resource_path_prefix = ultralight::String("resources/");
    auto &platform = ultralight::Platform::instance();
    platform.set_config(ultralightConfig);
    platform.set_font_loader(ultralight::GetPlatformFontLoader());
    const QByteArray rootUtf8 = dataRoot_.toUtf8();
    platform.set_file_system(ultralight::GetPlatformFileSystem(
        ultralight::String(rootUtf8.constData())));
    platformInitialized_ = true;
  }

  renderer_ = sharedRenderer();
  return renderer_ ? true : false;
}

ultralight::RefPtr<ultralight::Renderer> HtmlRenderer::sharedRenderer() {
  static ultralight::RefPtr<ultralight::Renderer> renderer =
      ultralight::Renderer::Create();
  return renderer;
}

bool HtmlRenderer::ensureView() {
  if (view_ && viewInitialized_)
    return true;
  if (!renderer_)
    renderer_ = sharedRenderer();
  if (!renderer_)
    return false;

  ultralight::ViewConfig viewConfig;
  viewConfig.is_accelerated = false;
  viewConfig.is_transparent = true;
  viewConfig.enable_images = true;
  viewConfig.enable_javascript = true;
  view_ = renderer_->CreateView(width_, height_, viewConfig, nullptr);
  if (!view_)
    return false;

  loadListener_ = std::make_unique<LoadListener>(&loaded_);
  view_->set_load_listener(loadListener_.get());
  webBridge_.setView(view_);
  viewInitialized_ = true;
  return load(htmlPath_);
}

bool HtmlRenderer::load(const QString &path) {
  if (!view_ || !QFileInfo::exists(path))
    return false;

  const QString themeName = QFileInfo(path).dir().dirName();
  const QByteArray url =
      QStringLiteral("file:///%1/index.html").arg(themeName).toUtf8();
  loaded_ = false;
  qInfo() << "[Renderer] Loading theme:" << themeName;
  view_->LoadURL(url.constData());
  view_->set_needs_paint(true);
  return true;
}

void HtmlRenderer::reload(const ConfigValues &config,
                          const ThemeMetadata &metadata) {
  const bool sizeChanged = width_ != config.width || height_ != config.height;
  dataRoot_ = config.dataRoot;
  htmlPath_ = config.htmlPath;
  width_ = config.width;
  height_ = config.height;
  hotspotX_ = metadata.hotspotX;
  hotspotY_ = metadata.hotspotY;
  enabled_ = config.enabled;

  if (!view_)
    return;
  if (sizeChanged)
    view_->Resize(width_, height_);
  load(htmlPath_);
}

void HtmlRenderer::move(int x, int y, bool pressed) {
  if (!view_)
    return;
  webBridge_.move(x, y, pressed);
}

void HtmlRenderer::update() {
  if (!enabled_ || !ensureView() || !renderer_ || !view_)
    return;

  if (!view_->needs_paint())
    view_->set_needs_paint(true);

  renderer_->Update();
  renderer_->RefreshDisplay(0);
  renderer_->Render();

  auto *surface = view_->surface();
  auto *bitmapSurface =
      surface ? dynamic_cast<ultralight::BitmapSurface *>(surface) : nullptr;
  if (!bitmapSurface)
    return;

  auto bitmap = bitmapSurface->bitmap();
  if (!bitmap)
    return;

  bitmap->LockPixels();
  const auto *raw = static_cast<const uint8_t *>(bitmap->raw_pixels());
  if (raw) {
    width_ = static_cast<int>(bitmap->width());
    height_ = static_cast<int>(bitmap->height());
    stride_ = static_cast<int>(bitmap->row_bytes());
    const size_t byteCount = static_cast<size_t>(stride_) * height_;
    pixelBuffer_.resize(byteCount);
    std::memcpy(pixelBuffer_.data(), raw, byteCount);
    newFrame_ = true;
  }
  bitmap->UnlockPixels();
}

const uint8_t *HtmlRenderer::pixels() const {
  return pixelBuffer_.empty() ? nullptr : pixelBuffer_.data();
}

} // namespace UltralightWebCursor
