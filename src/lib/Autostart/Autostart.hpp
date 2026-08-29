#pragma once

#include <string>

namespace UltralightWebCursorM {

// Registers this application to launch on user login.
// Returns true on success.
bool registerAutostart();

// Removes the login autostart registration.
// Returns true on success (or if it was not registered).
bool unregisterAutostart();

// Returns true if the application is currently registered to autostart.
bool isAutostartEnabled();

} // namespace UltralightWebCursorM
