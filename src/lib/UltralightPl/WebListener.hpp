#pragma once

#include <iostream>       
#include <Ultralight/Ultralight.h>
#include <AppCore/AppCore.h>
#include <QDBusConnection>
#include <QDebug>
class LocalLoadListener : public ultralight::LoadListener 
{
public:
    explicit LocalLoadListener(bool* flag) : loaded_(flag) {}

    void OnFinishLoading(
        ultralight::View* view,
        uint64_t frame_id,
        bool main_frame,
        const ultralight::String& url
    ) override 
    {
        qDebug() << "========== OnFinishLoading ==========";
        qDebug() << "view      =" << view;
        qDebug() << "frame_id  =" << frame_id;
        qDebug() << "mainFrame =" << main_frame;
        qDebug() << "url       =" << url.utf8().data();
        qDebug() << "loaded_   =" << loaded_;

        if (main_frame) {
            std::cout << "[Ultralight] loaded " << url.utf8().data() << "\n";
            if (loaded_) {
                *loaded_ = true;
            }
        }
    }

    void OnFailLoading(
        ultralight::View* view,
        uint64_t frame_id,
        bool main_frame,
        const ultralight::String& url,
        const ultralight::String& desc,
        const ultralight::String& error_domain,
        int error_code
    ) override 
    {
        qDebug() << "========== OnFailLoading ==========";
        qDebug() << "view         =" << view;
        qDebug() << "frame_id     =" << frame_id;
        qDebug() << "mainFrame    =" << main_frame;
        qDebug() << "url          =" << url.utf8().data();
        qDebug() << "description  =" << desc.utf8().data();
        qDebug() << "errorDomain  =" << error_domain.utf8().data();
        qDebug() << "errorCode    =" << error_code;
        qDebug() << "loaded_      =" << loaded_;


        if (main_frame) {
            std::cerr << "[Ultralight] load failed " 
                      << url.utf8().data() << " " 
                      << desc.utf8().data() << "\n";
            if (loaded_) {
                *loaded_ = false;
            }
        }
    }

private:
    bool* loaded_ = nullptr;
};
