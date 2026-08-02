#include "../header/QtMouseProvider.hpp"

#include "../lib/X11MouseProvider.hpp"

QtMouseProvider::QtMouseProvider(QObject* parent) : QObject(parent) {}

bool QtMouseProvider::initialize()
{
    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, &QtMouseProvider::onTimer);
    timer_.start();
    return true;
}

void QtMouseProvider::setCallback(Callback callback)
{
    callback_ = std::move(callback);
}

void QtMouseProvider::onTimer()
{
    if(!callback_)
        return;

    UltralightWebCursorM::MousePoint point;
    if(UltralightWebCursorM::readX11CursorPosition(point))
        callback_(point);
}
