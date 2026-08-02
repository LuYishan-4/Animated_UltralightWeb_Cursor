#include "PlatformMouseProvider.hpp"

#include <X11/Xlib.h>

namespace {

bool readX11CursorPosition(UltralightWebCursorM::MousePoint& out)
{
    Display* display = XOpenDisplay(nullptr);
    if(!display)
        return false;

    Window root = DefaultRootWindow(display);
    Window root_return = 0;
    Window child_return = 0;
    int root_x = 0;
    int root_y = 0;
    int win_x = 0;
    int win_y = 0;
    unsigned int mask_return = 0;

    const Bool ok = XQueryPointer(
        display,
        root,
        &root_return,
        &child_return,
        &root_x,
        &root_y,
        &win_x,
        &win_y,
        &mask_return
    );

    XCloseDisplay(display);

    if(!ok)
        return false;

    out.x = root_x;
    out.y = root_y;
    out.pressed = (mask_return & Button1Mask) != 0;
    return true;
}

}

PlatformMouseProvider::PlatformMouseProvider(QObject* parent) : QObject(parent) {}

bool PlatformMouseProvider::initialize()
{
    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, &PlatformMouseProvider::onTimer);
    timer_.start();
    return true;
}

void PlatformMouseProvider::setCallback(Callback callback)
{
    callback_ = std::move(callback);
}

void PlatformMouseProvider::onTimer()
{
    if(!callback_)
        return;

    UltralightWebCursorM::MousePoint point;
    if(readX11CursorPosition(point))
        callback_(point);
}
