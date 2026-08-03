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
#include "../lib/SharedCursorRender.hpp"

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


void reload(const ConfigValues& uconfig,const JSONConf& data);

bool resize(const int&  width,const int&  height);

    const uint8_t* pixels() const;



    int width() const
    {
        return width_;
    }


    int height() const
    {
        return height_;
    }


    int stride() const
    {
        return stride_;
    }

    int hotspotX() const { return hotspot_x_; }
    int hotspotY() const { return hotspot_y_; }

      CursorRenderState getRenderState(const QPointF& currentMousePos) const
    {
        CursorRenderState state;
        state.pos = currentMousePos;
        state.hotspot = QPointF(hotspot_x_, hotspot_y_);
        state.visible = enabled_ && is_loaded_;
        return state;
    }



    void setEnabled(bool enabled);


    bool isEnabled() const;


    bool hasNewFrame() const;


    void clearNewFrame();



private:


    ultralight::RefPtr<ultralight::Renderer> renderer_;
    std::shared_ptr<WebCall> webcall; 


    ultralight::RefPtr<ultralight::View> view_;


    std::unique_ptr<ultralight::LoadListener> listener_;



    bool is_loaded_ = false;


    bool enabled_ = true;


    bool new_frame_ = false;
    int width_ = 128;
    int height_ = 128;
    int stride_ = 0;
    bool platform_initialized_=false;
    int minwidth = 128;
    int minheight = 128;
    std::string m_permanentSdkPath; 
     int hotspot_x_ = 0;
    int hotspot_y_ = 0;


    std::vector<uint8_t> pixel_buffer_;



    std::filesystem::path html_path_;


    std::filesystem::file_time_type html_time_;

};


}