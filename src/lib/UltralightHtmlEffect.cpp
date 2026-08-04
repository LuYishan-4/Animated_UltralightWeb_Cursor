#include "../header/UltralightHtmlEffect.hpp"

#include <QDebug>
#include <QDir>
#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include "UltralightPl/WebListener.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <QDBusConnection>
#include <QProcess>

namespace UltralightWebCursorM
{
UltralightHtmlEffect::UltralightHtmlEffect() {}

UltralightHtmlEffect::~UltralightHtmlEffect()
{
    std::cout << "[Ultralight] destroy\n";
    listener_.reset();
    view_ = nullptr;
    renderer_ = nullptr;
    webcall = nullptr;
}

// initialize
bool UltralightHtmlEffect::initialize(const ConfigValues& uconfig, const JSONConf& data){
    width_ = data.minWidth;
    height_ = data.minHeight;
    html_path_ = uconfig.html;
    m_permanentSdkPath = uconfig.sdk;
    enabled_ = uconfig.enabled;
    minheight = data.minHeight;
    minwidth = data.minWidth;
    hotspot_x_ = data.hotspotX;
    hotspot_y_ = data.hotspotY;

    std::filesystem::path sdk_dir(m_permanentSdkPath);
    std::filesystem::path resources_dir = sdk_dir / "resources";
    if(!std::filesystem::exists(resources_dir)) {
        return false;
    }

    std::vector<std::string> required_files = {
        "cacert.pem",
        "icudt67l.dat"
    };

    for(const auto& file_name : required_files) {
        if(!std::filesystem::exists(resources_dir / file_name)) {
            qDebug() << "[UltralightCursorEffect] missing resource" << QString::fromStdString(file_name);
            return false;
        }
    }
    qDebug() << "[UltralightCursorEffect] init3" <<html_path_.c_str() << m_permanentSdkPath.c_str();
    if(!platform_initialized_) {
        ultralight::Config config;
        config.resource_path_prefix = ultralight::String("resources/");
        auto& platform = ultralight::Platform::instance();
        platform.set_config(config);
        platform.set_font_loader(ultralight::GetPlatformFontLoader());
        platform.set_file_system(
            ultralight::GetPlatformFileSystem(
                ultralight::String(m_permanentSdkPath.c_str())
            )
        );
        platform_initialized_ = true;
    }
    qDebug() << "[UltralightCursorEffect] init4" <<html_path_.c_str() << m_permanentSdkPath.c_str();
    renderer_ = ultralight::Renderer::Create();
    if(!renderer_)return false;
    ultralight::ViewConfig vc;
    vc.is_accelerated = false;
    vc.is_transparent = true;
    view_ = renderer_->CreateView(width_, height_, vc, nullptr);
    if(!view_)return false;
    listener_ = std::make_unique<LocalLoadListener>(&is_loaded_);
    view_->set_load_listener(listener_.get());
    webcall = std::make_shared<WebCall>();
    webcall->view_ = view_;
    qDebug() << "[UltralightCursorEffect] 3";
    if(std::filesystem::exists(html_path_))html_time_ = std::filesystem::last_write_time(html_path_);
    return load(html_path_);
}

bool UltralightHtmlEffect::load(const std::string& path)
{
    qDebug() << "[UltralightCursorEffect] 4";
    std::ifstream file(path);
    if(!file) {
        qDebug() << "[UltralightCursorEffect] Failed to open file:" << QString::fromStdString(path);
        return false;
    }
    qDebug() << "[UltralightCursorEffect] 5";
    std::filesystem::path p(path);
    std::string folderName = p.parent_path().filename().string();
    std::string fileUrl = "file:///" + folderName + "/index.html";
    is_loaded_ = false;
    qDebug() << "[UltralightCursorEffect] 6";
    view_->LoadURL(fileUrl.c_str());
    qDebug() << "[UltralightCursorEffect] 7";
    view_->set_needs_paint(true);
    return true;
}

bool UltralightHtmlEffect::resize(const int& width, const int& height)
{
    if(width > minwidth || height > minheight)
        return false;
    view_->Resize(width, height);
    return true;
}

void UltralightHtmlEffect::reload(const ConfigValues& uconfig, const JSONConf& data){
    width_ = data.minWidth;
    height_ = data.minHeight;
    html_path_ = uconfig.html;
    m_permanentSdkPath = uconfig.sdk;
    enabled_ = uconfig.enabled;
    hotspot_x_ = data.hotspotX;
    hotspot_y_ = data.hotspotY;
    UltralightHtmlEffect::load(html_path_);
    UltralightHtmlEffect::resize(width_, height_);
}
void UltralightHtmlEffect::move(int x, int y, bool pressed)
{
    if(!view_)return;
    if(webcall)webcall->move(x, y, pressed);
    view_->set_needs_paint(true);
}

void UltralightHtmlEffect::update()
{
    if(!enabled_)return;
    if(!renderer_ || !view_)return;
    renderer_->Update();
    view_->set_needs_paint(true);
    renderer_->Render();
    auto surface = view_->surface();
    if(!surface)
        return;

    auto bitmap_surface = dynamic_cast<ultralight::BitmapSurface*>(surface);
    if(!bitmap_surface)
        return;

    auto bitmap = bitmap_surface->bitmap();
    if(!bitmap)
        return;

    bitmap->LockPixels();
    uint8_t* raw = static_cast<uint8_t*>(bitmap->raw_pixels());
    if(raw) {
        width_ = bitmap->width();
        height_ = bitmap->height();
        stride_ = bitmap->row_bytes();
        const size_t size = stride_ * height_;

        pixel_buffer_.resize(size);
        memcpy(pixel_buffer_.data(), raw, size);
        new_frame_ = true;
    }
    bitmap->UnlockPixels();
}

void UltralightHtmlEffect::setEnabled(bool enabled)
{
    enabled_ = enabled;
}

bool UltralightHtmlEffect::isEnabled() const
{
    return enabled_;
}

bool UltralightHtmlEffect::hasNewFrame() const
{
    return new_frame_;
}

void UltralightHtmlEffect::clearNewFrame()
{
    new_frame_ = false;
}

const uint8_t* UltralightHtmlEffect::pixels() const
{
    if(pixel_buffer_.empty())
        return nullptr;
    return pixel_buffer_.data();
}

}
