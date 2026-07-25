#pragma once

#include <memory>


namespace UltralightWebCursorM
{


struct MousePoint
{
    int x = 0;
    int y = 0;

    bool pressed = false;
};


class IMouseUploader
{

public:

    virtual ~IMouseUploader() = default;

    virtual bool initialize() = 0;

    virtual bool poll(MousePoint& out) = 0;

};


std::unique_ptr<IMouseUploader> createKdeMouseUploader();


}
