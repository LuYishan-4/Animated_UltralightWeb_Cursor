#include "PlatformMouseProvider.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <QCursor>
#endif

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

    UltralightWebCursorM::MousePoint mousePoint;

#if defined(_WIN32)
    POINT point{};
    if(!GetCursorPos(&point))
        return;

    mousePoint.x = point.x;
    mousePoint.y = point.y;
    mousePoint.pressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
#else
    const QPoint globalPos = QCursor::pos();
    mousePoint.x = globalPos.x();
    mousePoint.y = globalPos.y();
    mousePoint.pressed = false;
#endif

    callback_(mousePoint);
}
