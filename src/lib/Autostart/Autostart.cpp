#include "Autostart.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>

#include <filesystem>

#if defined(_WIN32)
#include <QSettings>
#endif

namespace UltralightWebCursorM {

namespace {

#if defined(_WIN32)

constexpr auto kRunKeyPath =
    "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr auto kRunValueName = "UltralightWebCursor";

#endif

QString executableCommand() {
  const QString exePath =
      QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
  return QStringLiteral("\"%1\" --silent").arg(exePath);
}

} // namespace

bool registerAutostart() {
#if defined(_WIN32)
  QSettings runKey(QLatin1String(kRunKeyPath), QSettings::NativeFormat);
  runKey.setValue(QLatin1String(kRunValueName), executableCommand());
  return runKey.status() == QSettings::NoError;
#else
  // XDG autostart desktop entry (X11 and any non-Windows standalone build).
  const QString autostartDir =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/autostart");
  QDir().mkpath(autostartDir);

  const QString filePath =
      autostartDir + QStringLiteral("/ultralightwebcursor.desktop");

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    return false;

  file.write("[Desktop Entry]\n");
  file.write("Type=Application\n");
  file.write("Name=Ultralight Web Cursor\n");
  file.write("Comment=Animated HTML/CSS cursor\n");
  file.write("Exec=");
  file.write(executableCommand().toUtf8());
  file.write("\n");
  file.write("X-GNOME-Autostart-enabled=true\n");
  file.write("Terminal=false\n");
  file.close();
  return true;
#endif
}

bool unregisterAutostart() {
#if defined(_WIN32)
  QSettings runKey(QLatin1String(kRunKeyPath), QSettings::NativeFormat);
  runKey.remove(QLatin1String(kRunValueName));
  return runKey.status() == QSettings::NoError;
#else
  const QString autostartDir =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/autostart");
  const QString filePath =
      autostartDir + QStringLiteral("/ultralightwebcursor.desktop");
  if (QFile::exists(filePath))
    return QFile::remove(filePath);
  return true;
#endif
}

bool isAutostartEnabled() {
#if defined(_WIN32)
  QSettings runKey(QLatin1String(kRunKeyPath), QSettings::NativeFormat);
  return runKey.contains(QLatin1String(kRunValueName));
#else
  const QString autostartDir =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/autostart");
  return QFile::exists(autostartDir +
                       QStringLiteral("/ultralightwebcursor.desktop"));
#endif
}

} // namespace UltralightWebCursorM
