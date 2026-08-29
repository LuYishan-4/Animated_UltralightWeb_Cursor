#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QVariant>
#include <QEvent>
#include "X11MouseProvider.hpp"
#if defined(__linux__) || defined(Q_OS_LINUX)
#  include <X11/Xlib.h>
#  undef Status
#  undef Bool
#  undef None
#  undef KeyPress
#  undef KeyRelease
#  undef FocusIn
#  undef FocusOut
#  undef FontChange
#  undef Cursor
#  undef Screen
#  undef Window
#  undef Event
#endif
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

    const bool ok = XQueryPointer(
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
