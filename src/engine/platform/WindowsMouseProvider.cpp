#include "../MouseProvider.hpp"

#include <QCursor>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace UltralightWebCursor {

MouseProvider::~MouseProvider() = default;

bool MouseProvider::initialize() { return true; }

void MouseProvider::update() {
  if (!callback_)
    return;

  const QPoint position = QCursor::pos();
  callback_({static_cast<qreal>(position.x()), static_cast<qreal>(position.y()),
             (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0});
}

} // namespace UltralightWebCursor
