#include "X11MouseProvider.hpp"
#include <X11/Xlib.h>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

namespace UltralightWebCursorM
{

bool readX11CursorPosition(MousePoint& out){
    static Display* cached_display = nullptr;
    
    if (!cached_display) {
        cached_display = XOpenDisplay(nullptr);
        if (!cached_display) {
            qCritical() << "[X11MouseProvider] Failed to open X11 display connection!";
            return false;
        }
    }

    Window root = DefaultRootWindow(cached_display);
    Window root_return = 0;
    Window child_return = 0;
    int root_x = 0;
    int root_y = 0;
    int win_x = 0;
    int win_y = 0;
    unsigned int mask_return = 0;

    const Bool ok = XQueryPointer(
        cached_display,
        root,
        &root_return,
        &child_return,
        &root_x,
        &root_y,
        &win_x,
        &win_y,
        &mask_return
    );

    if (!ok)
        return false;

    qreal rawX = root_x;
    qreal rawY = root_y;

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        qreal devicePixelRatio = screen->devicePixelRatio();
        if (devicePixelRatio > 0.0) {
            rawX /= devicePixelRatio;
            rawY /= devicePixelRatio;
        }
    }

    out.x = rawX;
    out.y = rawY;
    out.pressed = (mask_return & Button1Mask) != 0;
    return true;
}


} // namespace UltralightWebCursorM
