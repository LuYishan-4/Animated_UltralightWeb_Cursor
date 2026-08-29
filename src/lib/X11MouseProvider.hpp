#pragma once

#include "../header/MouseProvider.hpp"

#if defined(__linux__) || defined(Q_OS_LINUX)

namespace UltralightWebCursorM {

bool readX11CursorPosition(MousePoint& out);

} // namespace UltralightWebCursorM

#endif // Q_OS_LINUX
