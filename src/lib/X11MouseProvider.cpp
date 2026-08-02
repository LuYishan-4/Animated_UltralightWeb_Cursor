#include "X11MouseProvider.hpp"

#include <X11/Xlib.h>

namespace UltralightWebCursorM
{

bool readX11CursorPosition(MousePoint& out)
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
