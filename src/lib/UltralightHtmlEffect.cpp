#include "../header/UltralightHtmlEffect.hpp"

#include <QDebug>
#include <QDir>
#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include "UltralightPl/WebListener.hpp"
#include <memory> 
#include "gl/GPUDriverGL.h"
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
    context_ = nullptr;
}

// initialize
bool UltralightHtmlEffect::initialize(const ConfigValues& uconfig, const JSONConf& data){
 html_value_ = {
    .width_=data.minWidth,
    .height_= data.minHeight,
    .stride_= 0, 
    .minwidth  = data.minWidth,
    .minheight= data.minHeight,
    .hotspot_x_= data.hotspotX,
    .hotspot_y_ = data.hotspotY,
    .m_permanentSdkPath  = uconfig.sdk,
    .html_path_ = uconfig.html,
    .use_gpu_ = uconfig.EnableGPU
};
    std::filesystem::path sdk_dir(html_value_.m_permanentSdkPath);
    std::filesystem::path resources_dir = sdk_dir / "resources";
    if(!std::filesystem::exists(resources_dir)) return false;
    

    std::vector<std::string> required_files = {"cacert.pem",  "icudt67l.dat" };

    for(const auto& file_name : required_files)if(!std::filesystem::exists(resources_dir / file_name))return false;
    if(!platform_initialized_) {
        ultralight::Config config;
        config.resource_path_prefix = ultralight::String("resources/");
        auto& platform = ultralight::Platform::instance();
        platform.set_config(config);
        platform.set_font_loader(ultralight::GetPlatformFontLoader());
        platform.set_file_system(
            ultralight::GetPlatformFileSystem(
                ultralight::String(html_value_.m_permanentSdkPath.c_str())
            )
        );
        if (html_value_.use_gpu_){
            context_ = std::make_unique<ultralight::GPUContextGL>(false, false); 
            platform.set_gpu_driver(context_->driver());
        }
        platform_initialized_ = true;
    }
    qDebug() << "[UltralightCursorEffect] init4" <<html_value_.html_path_.c_str() << html_value_.m_permanentSdkPath.c_str();
    renderer_ = ultralight::Renderer::Create();
    if(!renderer_)return false;
    ultralight::ViewConfig vc;
        vc.is_accelerated = true;
    vc.is_transparent = true;
    vc.enable_images = true;
    vc.enable_javascript = true;
    view_ = renderer_->CreateView(html_value_.width_, html_value_.height_, vc, nullptr);
    if(!view_)return false;
    listener_ = std::make_unique<LocalLoadListener>(&is_loaded_);
    view_->set_load_listener(listener_.get());
    webcall = std::make_shared<WebCall>();
    webcall->view_ = view_;
    qDebug() << "[UltralightCursorEffect] 3";
    if(std::filesystem::exists(html_value_.html_path_))html_time_ = std::filesystem::last_write_time(html_value_.html_path_);
    return load(html_value_.html_path_);
}

bool UltralightHtmlEffect::load(const std::string& path){
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
    if(width > html_value_.minwidth || height > html_value_.minheight)
        return false;
    view_->Resize(width, height);
    return true;
}

void UltralightHtmlEffect::reload(const ConfigValues& uconfig, const JSONConf& data){
    html_value_ = {
        .width_=data.minWidth,
        .height_= data.minHeight,
        .stride_= 0, 
        .minwidth  = data.minWidth,
        .minheight= data.minHeight,
        .hotspot_x_= data.hotspotX,
        .hotspot_y_ = data.hotspotY,
        .m_permanentSdkPath  = uconfig.sdk,
        .html_path_ = uconfig.html,
        .use_gpu_ = uconfig.EnableGPU
    };
    UltralightHtmlEffect::load(html_value_.html_path_);
    UltralightHtmlEffect::resize(html_value_.width_, html_value_.height_);
}
void UltralightHtmlEffect::move(int x, int y, bool pressed)
{
    if(!view_)return;
    if(webcall)webcall->move(x, y, pressed);
    view_->set_needs_paint(true);
}

void UltralightHtmlEffect::update(){
    if(!enabled_) return;
    if(!renderer_ || !view_) return;
    
    static int updateLoopCounter = 0;
    updateLoopCounter++;

    if (!view_->needs_paint() && updateLoopCounter % 15 == 0) {
        view_->set_needs_paint(true);
    }

    renderer_->Update();
    
    bool target_wants_paint = view_->needs_paint();
    
    renderer_->Render();

    if (context_) {
        new_frame_ = true; 
        if (updateLoopCounter % 60 == 0) {

            ultralight::String title_script = "document.title";
            ultralight::String element_script = "document.body ? document.body.innerHTML.length : -1";
            ultralight::JSValue title_res = view_->EvaluateScript(title_script);
            ultralight::JSValue elem_res = view_->EvaluateScript(element_script);

            qDebug() << "[UltralightHtmlDebug] [GPU Pulse] VRAM Tex:" << view_->render_target().texture_id
                     << " | needs_paint():" << target_wants_paint
                     << " | Body InnerHTML Len:" << (elem_res.IsNumber() ? (int)elem_res.ToNumber() : -1);
        }
    } else {
        auto surface = view_->surface();
        if(!surface) return;
        auto bitmap_surface = dynamic_cast<ultralight::BitmapSurface*>(surface);
        if(!bitmap_surface) return;
        auto bitmap = bitmap_surface->bitmap();
        if(!bitmap) return;
        
        bitmap->LockPixels();
        uint8_t* raw = static_cast<uint8_t*>(bitmap->raw_pixels());
        if(raw) {
            html_value_.width_ = bitmap->width();
            html_value_.height_ = bitmap->height();
            html_value_.stride_ = bitmap->row_bytes();
            const size_t size = html_value_.stride_ * html_value_.height_;

            pixel_buffer_.resize(size);
            memcpy(pixel_buffer_.data(), raw, size);
            new_frame_ = true;

            if (updateLoopCounter % 60 == 0) {
                qDebug() << "[UltralightHtmlDebug] [CPU Memory Map Pulse] Mapped Pixel Size Buffer:" << size 
                         << "bytes | Stride bounds alignment:" << html_value_.stride_;
            }
        }
        bitmap->UnlockPixels();
    }
}

void UltralightHtmlEffect::setEnabled(bool enabled)
{
    qDebug() << "[UltralightHtmlDebug] Core active status update toggled to:" << enabled;
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


const uint8_t* UltralightHtmlEffect::pixels() const{
    if(pixel_buffer_.empty())
        return nullptr;
    return pixel_buffer_.data();
}


unsigned int UltralightHtmlEffect::textureId() const {
    if (!view_ || !context_) return 0;
    uint32_t ultralight_tex_id = view_->render_target().texture_id;
    auto* gl_driver = static_cast<ultralight::GPUDriverGL*>(context_->driver());
    if (!gl_driver) return 0;
    auto it = gl_driver->texture_map.find(ultralight_tex_id);
    if (it == gl_driver->texture_map.end()) return 0;
    return it->second.tex_id;
}


ultralight::View* UltralightHtmlEffect::view() const {
    return view_.get();
}

}
