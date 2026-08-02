#include "PlatformMouseProvider.hpp"

#include "../lib/X11MouseProvider.hpp"

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
    if(UltralightWebCursorM::readX11CursorPosition(point))
        callback_(point);
}
