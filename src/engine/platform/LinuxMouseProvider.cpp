#include "../MouseProvider.hpp"

#include <QCursor>
#include <QDebug>

#include <X11/Xlib.h>

#undef Bool
#undef Cursor
#undef None
#undef Status
#undef Window

namespace UltralightWebCursor {

MouseProvider::~MouseProvider() {
  if (nativeDisplay_)
    XCloseDisplay(static_cast<Display *>(nativeDisplay_));
}

bool MouseProvider::initialize() {
  nativeDisplay_ = XOpenDisplay(nullptr);
  if (!nativeDisplay_) {
    qCritical() << "[Mouse] Cannot connect to X11. Ensure Xorg/XWayland is "
                   "running and DISPLAY is set.";
    return false;
  }
  return true;
}

void MouseProvider::update() {
  if (!callback_ || !nativeDisplay_)
    return;

  auto *display = static_cast<Display *>(nativeDisplay_);
  ::Window rootReturn = 0;
  ::Window childReturn = 0;
  int rootX = 0;
  int rootY = 0;
  int windowX = 0;
  int windowY = 0;
  unsigned int buttons = 0;
  if (!XQueryPointer(display, DefaultRootWindow(display), &rootReturn,
                     &childReturn, &rootX, &rootY, &windowX, &windowY,
                     &buttons)) {
    return;
  }

  const QPoint position = QCursor::pos();
  callback_({static_cast<qreal>(position.x()), static_cast<qreal>(position.y()),
             (buttons & Button1Mask) != 0});
}

} // namespace UltralightWebCursor
