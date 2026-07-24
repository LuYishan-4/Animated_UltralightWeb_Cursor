#include "header/UltralightHtmlEffect.hpp"

#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include "UltralightPl/WebListener.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <QDBusConnection>

namespace UltralightWebCursorM
{

UltralightHtmlEffect::UltralightHtmlEffect(){}

UltralightHtmlEffect::~UltralightHtmlEffect(){
    std::cout<< "[Ultralight] destroy\n";
    listener_.reset();
    view_ = nullptr;
    renderer_ = nullptr;
}


//initialize
bool UltralightHtmlEffect::initialize(const ConfigValues& uconfig){
    width_ = uconfig.width;
    height_ = uconfig.height;
    if (!platform_initialized_){
    ultralight::Config config;
    config.resource_path_prefix =
        ultralight::String(
            "resources/"
        );
    auto& platform =ultralight::Platform::instance();
    platform.set_config(config);
    platform.set_font_loader(ultralight::GetPlatformFontLoader());

    platform.set_file_system(
        ultralight::GetPlatformFileSystem(
            ultralight::String(
                uconfig.sdk.c_str()
            )
        )
    );
    platform_initialized_= true;
}
    renderer_ =ultralight::Renderer::Create();
    if(!renderer_) return false;

    ultralight::ViewConfig vc;
    vc.is_accelerated = false;
    vc.is_transparent = true;

    view_ =
        renderer_->CreateView(
            width_,
            height_,
            vc,
            nullptr
        );
    qDebug() << "[UltralightCursorEffect] width done";
    html_path_ = uconfig.html;
    if(!view_)return false;
    listener_ =std::make_unique<LocalLoadListener>(&is_loaded_);
    view_->set_load_listener(listener_.get());
    if(std::filesystem::exists(html_path_ )) html_time_ =std::filesystem::last_write_time(html_path_ );
    qDebug() << "[UltralightCursorEffect] c14242";
    return load(html_path_);
}


bool UltralightHtmlEffect::load(const std::string& path){
    std::ifstream file(path);
    if(!file)return false;

    std::string html(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );



    std::filesystem::path p(path);

    std::string base ="file://" +p.parent_path().string()+"/";

    is_loaded_ = false;
    qDebug() << "[UltralightCursorEffect] "<<html;
    view_->LoadHTML(
        ultralight::String(
            html.c_str()
        ),
        ultralight::String(
            base.c_str()
        )
    );
    view_->set_needs_paint(true);
    return true;
}
bool UltralightHtmlEffect::resize(const int&  width,const int&  height){
    if (width>1920 || height > 1080)return false;
    view_->Resize(width,height);
    return true;
}

void UltralightHtmlEffect::reload(const ConfigValues& uconfig){
       UltralightHtmlEffect::load(uconfig.html);
       UltralightHtmlEffect::resize(uconfig.width,uconfig.height);
}

void UltralightHtmlEffect::move( int x, int y,bool pressed){
    if(!view_)return;
    
    std::string js ="if(window.moveCursor)"
        "{window.moveCursor("
        +
        std::to_string(x)
        +
        ","
        +
        std::to_string(y)
        +
        ","
        +
        (pressed ? "true":"false")
        +
        ");}";
    view_->EvaluateScript(
        ultralight::String(
            js.c_str()
        )
    );
    view_->set_needs_paint(true);
}

void UltralightHtmlEffect::update(){
    if(!enabled_)return;
    if(!renderer_ || !view_)return;
    renderer_->Update();
    view_->set_needs_paint(true);
    renderer_->Render();
    auto surface =  view_->surface();
    if(!surface)return;

    auto bitmap_surface =dynamic_cast<ultralight::BitmapSurface*>(surface);
    if(!bitmap_surface)return;

    auto bitmap =bitmap_surface->bitmap();

    if(!bitmap) return;
    bitmap->LockPixels();
    uint8_t* raw =
        static_cast<uint8_t*>(
            bitmap->raw_pixels()
        );
    if(raw){
        width_ =
            bitmap->width();
        height_ =
            bitmap->height();
        stride_ =
            bitmap->row_bytes();
        size_t size =
            stride_ * height_;

        pixel_buffer_.resize(size);

        memcpy(
            pixel_buffer_.data(),
            raw,
            size
        );
        new_frame_ = true;
    }
    bitmap->UnlockPixels();
}
void UltralightHtmlEffect::setEnabled(
    bool enabled
)
{
    enabled_ = enabled;
}

bool UltralightHtmlEffect::isEnabled() const{
    return enabled_;
}

bool UltralightHtmlEffect::hasNewFrame() const{
    return new_frame_;
}

void UltralightHtmlEffect::clearNewFrame(){
    new_frame_ = false;
}
const uint8_t* UltralightHtmlEffect::pixels() const
{

    if(pixel_buffer_.empty())
        return nullptr;
    return pixel_buffer_.data();
}

}