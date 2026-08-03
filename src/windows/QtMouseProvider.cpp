#include "../header/QtMouseProvider.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <QCursor>
#endif
#include <QGuiApplication>
#include <QScreen>

QtMouseProvider::QtMouseProvider(QObject* parent) : QObject(parent) {}

bool QtMouseProvider::initialize() {
    return true;
}

void QtMouseProvider::setCallback(Callback callback) {
    callback_ = std::move(callback);
}

void QtMouseProvider::updateMouseState() {
    if (!callback_) return;

    UltralightWebCursorM::MousePoint mousePoint;

#if defined(_WIN32)
    POINT point{};
    if (!GetCursorPos(&point)) return;

    qreal rawX = point.x;
    qreal rawY = point.y;


    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        qreal devicePixelRatio = screen->devicePixelRatio();
        if (devicePixelRatio > 0.0) {
            rawX /= devicePixelRatio;
            rawY /= devicePixelRatio;
        }
    }

    mousePoint.x = rawX;
    mousePoint.y = rawY;
    mousePoint.pressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
#else

    const QPoint globalPos = QCursor::pos();
    mousePoint.x = globalPos.x();
    mousePoint.y = globalPos.y();
    mousePoint.pressed = false;
#endif

    callback_(mousePoint);

void QtMouseProvider::onTimer() {
    updateMouseState();
}
