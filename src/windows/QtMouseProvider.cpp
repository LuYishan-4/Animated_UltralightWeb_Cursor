#include "../header/QtMouseProvider.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <QCursor>
#endif

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
