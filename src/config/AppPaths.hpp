#pragma once

#include <QString>
#include <QStringList>

namespace UltralightWebCursor {

class AppPaths final {
public:
  static QString configFile();
  static QString userDataDir();
  static QString bundledDataDir();
  static QString engineExecutablePath();

  // Synchronizes required Ultralight resources and bundled themes into the
  // writable per-user data directory. Existing imported themes are preserved.
  static bool provisionUserData(QString *errorMessage = nullptr);

  static bool isThemeDirectory(const QString &path);
  static bool isBuiltInTheme(const QString &name);
  static QStringList builtInThemes();

private:
  AppPaths() = delete;
};

} // namespace UltralightWebCursor
