#include "header/UltralightHtmlEffect.hpp"
#include <QDir>
#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include "lib/UltralightPl/WebListener.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <QDBusConnection>
#include <QProcess>
namespace UltralightWebCursorM{
UltralightHtmlEffect::UltralightHtmlEffect(){}

UltralightHtmlEffect::~UltralightHtmlEffect(){
    std::cout<< "[Ultralight] destroy\n";
    listener_.reset();
    view_ = nullptr;
    renderer_ = nullptr;
    webcall = nullptr;
}


//initialize
bool UltralightHtmlEffect::initialize(const ConfigValues& uconfig,const JSONConf& data){
    width_ = uconfig.width;
    height_ = uconfig.height;
    html_path_ = uconfig.html;
    m_permanentSdkPath =  uconfig.sdk;
    enabled_ = uconfig.enabled;
    WebType = data.WebType;
    minheight = data.minHeight;
    minwidth = data.minWidth;
    localserver = data.localServer;
    mainboot = data.main;
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
                 m_permanentSdkPath.c_str()
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
    if(!view_)return false;
    listener_ =std::make_unique<LocalLoadListener>(&is_loaded_);
    view_->set_load_listener(listener_.get());
   webcall = std::make_shared<WebCall>();
    webcall->view_ = view_; 
    if(std::filesystem::exists(html_path_ )) html_time_ =std::filesystem::last_write_time(html_path_ );
      compileTypeScript(html_path_.parent_path().string());
    return load(html_path_);
}


bool UltralightHtmlEffect::load(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        qWarning() << "[UltralightCursorEffect] Failed to open file:" << QString::fromStdString(path);
        return false;
    }
    std::filesystem::path p(path);
    std::string folderName = p.parent_path().filename().string();
    std::string fileUrl = "file:///" + folderName + "/index.html";
    is_loaded_ = false;
    view_->LoadURL(fileUrl.c_str());
    view_->set_needs_paint(true);
    return true;
}
bool UltralightHtmlEffect::resize(const int&  width,const int&  height){
    if (width>1920 || height > 1080)return false;
    view_->Resize(width,height);
    return true;
}

void UltralightHtmlEffect::reload(const ConfigValues& uconfig,const JSONConf& data){
    width_ = uconfig.width;
    height_ = uconfig.height;
    html_path_ = uconfig.html;
    m_permanentSdkPath =  uconfig.sdk;
    enabled_ = uconfig.enabled;
    UltralightHtmlEffect::load(html_path_ );
    UltralightHtmlEffect::resize(width_,height_);
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

bool UltralightHtmlEffect::compileTypeScript(const std::string& projectDir) {
    QDir dir(QString::fromStdString(projectDir));
    if (!dir.exists()) {
        qWarning() << "[UltralightCursorEffect] TS project dir not found:"
                    << QString::fromStdString(projectDir);
        return false;
    }

    if (!dir.exists(QStringLiteral("node_modules"))) {
        QProcess installProcess;
        installProcess.setWorkingDirectory(dir.absolutePath());
        installProcess.setProgram(QStringLiteral("npm"));
        installProcess.setArguments({QStringLiteral("install")});
        installProcess.start();
        if (!installProcess.waitForFinished(120000)) { 
            qWarning() << "[UltralightCursorEffect] npm install timed out";
            return false;
        }
        if (installProcess.exitCode() != 0) {
            qWarning() << "[UltralightCursorEffect] npm install failed:\n"
                        << installProcess.readAllStandardError();
            return false;
        }
    }

    QProcess buildProcess;
    buildProcess.setWorkingDirectory(dir.absolutePath());
    buildProcess.setProgram(QStringLiteral("npm"));
    buildProcess.setArguments({QStringLiteral("run"), QStringLiteral("build")});
    buildProcess.start();
    if (!buildProcess.waitForFinished(60000)) { 
        qWarning() << "[UltralightCursorEffect] npm run build timed out";
        return false;
    }
    if (buildProcess.exitCode() != 0) {
        qWarning() << "[UltralightCursorEffect] TypeScript compile failed:\n"
                    << buildProcess.readAllStandardError();
        return false;
    }

    std::cout << "[Ultralight] TypeScript compiled successfully in "
              << projectDir << "\n";
    return true;
}

}