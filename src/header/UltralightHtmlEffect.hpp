#pragma once

#include <Ultralight/Ultralight.h>
#include <AppCore/AppCore.h>
#include "../config/UserConfig.hpp"
#include "../config/CursorJSON.hpp"
#include "../lib/WebCall/WebCall.hpp"
#include <filesystem>
#include <memory>
#include <vector>
#include <string>
#include "../lib/gl/GPUContextGL.h" 
namespace UltralightWebCursorM
{


class UltralightHtmlEffect
{

public:

    UltralightHtmlEffect();

    ~UltralightHtmlEffect();


bool initialize(const ConfigValues& uconfig,const JSONConf& data);


    bool load(
        const std::string& path
    );


    void update();



    void move(
        int x,
        int y,
        bool pressed
    );

ultralight::View* view() const;
void reload(const ConfigValues& uconfig,const JSONConf& data);

bool resize(const int&  width,const int&  height);

    const uint8_t* pixels() const;
    unsigned int textureId() const;



    int width() const
    {
        return html_value_.width_;
    }


    int height() const
    {
        return html_value_.height_;
    }


    int stride() const
    {
        return html_value_.stride_;
    }

    int hotspotX() const { return html_value_.hotspot_x_; }
    int hotspotY() const { return html_value_.hotspot_y_; }

    void setEnabled(bool enabled);


    bool isEnabled() const;


    bool hasNewFrame() const;


    void clearNewFrame();



private:
    struct Html_Value{
        int width_ = 128;
        int height_ = 128;
        int stride_ = 0;
        int minwidth = 128;
        int minheight = 128;
        int hotspot_x_ = 64;
        int hotspot_y_ = 64;
        std::string m_permanentSdkPath; 
        std::filesystem::path html_path_;
        bool use_gpu_ = true;
    };
    Html_Value html_value_; 
    




    std::unique_ptr<ultralight::GPUContextGL> context_;
    ultralight::RefPtr<ultralight::Renderer> renderer_;
    std::shared_ptr<WebCall> webcall; 


    ultralight::RefPtr<ultralight::View> view_;


    std::unique_ptr<ultralight::LoadListener> listener_;



    bool is_loaded_ = false;


    bool enabled_ = true;


    bool new_frame_ = false;
  
    bool platform_initialized_=false;

    std::vector<uint8_t> pixel_buffer_;




    std::filesystem::file_time_type html_time_;

};


}