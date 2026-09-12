#pragma once

#include <QString>

namespace UltralightWebCursor {

bool registerAutostart(const QString &engineExecutable,
                       QString *errorMessage = nullptr);
bool unregisterAutostart(QString *errorMessage = nullptr);
bool isAutostartEnabled(const QString &expectedExecutable = {});

} // namespace UltralightWebCursor
