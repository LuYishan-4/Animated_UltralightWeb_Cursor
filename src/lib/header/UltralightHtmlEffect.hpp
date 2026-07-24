#pragma once

#include <Ultralight/Ultralight.h>
#include <AppCore/AppCore.h>
#include "UserConfig.hpp"
#include <filesystem>
#include <memory>
#include <vector>
#include <string>


namespace UltralightWebCursorM
{


class UltralightHtmlEffect
{

public:

    UltralightHtmlEffect();

    ~UltralightHtmlEffect();


    bool initialize(const ConfigValues& uconfig );


    bool load(
        const std::string& path
    );


    void update();


    void move(
        int x,
        int y,
        bool pressed
    );


    void reload(const ConfigValues& uconfig);

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



    // 開關效果
    void setEnabled(bool enabled);


    bool isEnabled() const;



    // 避免每 frame upload
    bool hasNewFrame() const;


    void clearNewFrame();



private:


    ultralight::RefPtr<ultralight::Renderer> renderer_;


    ultralight::RefPtr<ultralight::View> view_;


    std::unique_ptr<ultralight::LoadListener> listener_;



    bool is_loaded_ = false;


    bool enabled_ = true;


    bool new_frame_ = false;



    // cursor size
    int width_ = 128;

    int height_ = 128;

    int stride_ = 0;

    bool platform_initialized_=false;



    std::vector<uint8_t> pixel_buffer_;



    std::filesystem::path html_path_;


    std::filesystem::file_time_type html_time_;

};


}