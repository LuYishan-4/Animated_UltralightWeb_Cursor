#pragma once

#include <functional>


namespace UltralightWebCursorM
{


struct MousePoint
{
    int x = 0;
    int y = 0;

    bool pressed = false;
};



class IMouseProvider
{

public:

    using Callback = std::function<void(const MousePoint&)>;


    virtual ~IMouseProvider() = default;


    virtual bool initialize() = 0;


    virtual void setCallback(Callback callback) = 0;

};


}
