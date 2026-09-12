#include "Autostart.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

namespace UltralightWebCursor {
namespace {

void setError(QString *destination, const QString &message) {
  if (destination)
    *destination = message;
}

QString quotedCommand(const QString &executable) {
  QString escaped = QDir::toNativeSeparators(executable);
  escaped.replace(QLatin1Char('"'), QStringLiteral("\\\""));
  return QStringLiteral("\"%1\" --silent").arg(escaped);
}

#ifndef Q_OS_WIN
QString autostartFile() {
  return QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
      .filePath(QStringLiteral("autostart/ultralightwebcursor.desktop"));
}
#endif

} // namespace

bool registerAutostart(const QString &engineExecutable, QString *errorMessage) {
  if (engineExecutable.isEmpty() || !QFile::exists(engineExecutable)) {
    setError(errorMessage,
             QStringLiteral("Cursor engine executable not found"));
    return false;
  }

#ifdef Q_OS_WIN
  QSettings runKey(
      QStringLiteral("HKEY_CURRENT_"
                     "USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
      QSettings::NativeFormat);
  runKey.setValue(QStringLiteral("UltralightWebCursor"),
                  quotedCommand(engineExecutable));
  if (runKey.status() != QSettings::NoError) {
    setError(errorMessage, QStringLiteral("Could not update Windows startup"));
    return false;
  }
  return true;
#else
  const QString fileName = autostartFile();
  if (!QDir().mkpath(QFileInfo(fileName).absolutePath())) {
    setError(errorMessage,
             QStringLiteral("Could not create autostart directory"));
    return false;
  }

  QSaveFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    setError(errorMessage, QStringLiteral("Could not write autostart entry"));
    return false;
  }

  const QByteArray desktopEntry =
      QByteArray("[Desktop Entry]\nType=Application\n") +
      "Name=Ultralight Web Cursor Engine\n" +
      "Comment=Animated HTML/CSS cursor engine\n" +
      "Exec=" + quotedCommand(engineExecutable).toUtf8() + '\n' +
      "Terminal=false\nX-GNOME-Autostart-enabled=true\n" + "NoDisplay=true\n";
  if (file.write(desktopEntry) != desktopEntry.size() || !file.commit()) {
    setError(errorMessage, QStringLiteral("Could not save autostart entry"));
    return false;
  }
  return true;
#endif
}

bool unregisterAutostart(QString *errorMessage) {
#ifdef Q_OS_WIN
  QSettings runKey(
      QStringLiteral("HKEY_CURRENT_"
                     "USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
      QSettings::NativeFormat);
  runKey.remove(QStringLiteral("UltralightWebCursor"));
  if (runKey.status() != QSettings::NoError) {
    setError(errorMessage, QStringLiteral("Could not update Windows startup"));
    return false;
  }
  return true;
#else
  const QString fileName = autostartFile();
  if (!QFile::exists(fileName) || QFile::remove(fileName))
    return true;
  setError(errorMessage, QStringLiteral("Could not remove autostart entry"));
  return false;
#endif
}

bool isAutostartEnabled(const QString &expectedExecutable) {
#ifdef Q_OS_WIN
  QSettings runKey(
      QStringLiteral("HKEY_CURRENT_"
                     "USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
      QSettings::NativeFormat);
  if (!runKey.contains(QStringLiteral("UltralightWebCursor")))
    return false;
  return expectedExecutable.isEmpty() ||
         runKey.value(QStringLiteral("UltralightWebCursor"))
             .toString()
             .contains(QDir::toNativeSeparators(expectedExecutable),
                       Qt::CaseInsensitive);
#else
  const QString fileName = autostartFile();
  if (!QFile::exists(fileName))
    return false;
  if (expectedExecutable.isEmpty())
    return true;
  QFile file(fileName);
  return file.open(QIODevice::ReadOnly | QIODevice::Text) &&
         QString::fromUtf8(file.readAll()).contains(expectedExecutable);
#endif
}

} // namespace UltralightWebCursor
