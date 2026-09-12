#include "BuildConfig.hpp"
#include "CursorOverlay.hpp"
#include "IpcServer.hpp"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QJsonObject>
#include <QLockFile>
#include <QStandardPaths>

using UltralightWebCursor::CursorOverlay;
using UltralightWebCursor::IpcServer;

int main(int argc, char **argv) {
#ifdef Q_OS_LINUX
  // The Linux overlay uses X11 APIs. On Wayland desktops it runs through
  // XWayland, so force Qt's xcb backend unless the user explicitly overrides
  // the platform for diagnostics.
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

  QGuiApplication app(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("UltralightWebCursor"));
  QCoreApplication::setApplicationName(
      QString::fromUtf8(UltralightWebCursor::BuildConfig::applicationName));
  QCoreApplication::setApplicationVersion(
      QString::fromUtf8(UltralightWebCursor::BuildConfig::version));
  app.setQuitOnLastWindowClosed(false);

  QCommandLineParser parser;
  parser.setApplicationDescription(
      QStringLiteral("Ultralight Web Cursor background engine"));
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addOption({QStringLiteral("silent"),
                    QStringLiteral("Run as a background process")});
  parser.addOption({QStringLiteral("debug"),
                    QStringLiteral("Enable per-frame diagnostics")});
  parser.process(app);
  if (parser.isSet(QStringLiteral("debug")))
    qputenv("ULTRALIGHTWEBCURSOR_DEBUG", "1");

  const QString lockPath =
      QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
          .filePath(QStringLiteral("ultralightwebcursor-engine.lock"));
  QLockFile instanceLock(lockPath);
  instanceLock.setStaleLockTime(0);
  if (!instanceLock.tryLock(100)) {
    qInfo() << "[Engine] Another instance is already running";
    return 0;
  }

  CursorOverlay overlay;
  if (!overlay.initialize()) {
    qCritical() << "[Engine] Cursor overlay initialization failed";
    return 1;
  }
  overlay.start();

  IpcServer ipcServer;
  if (!ipcServer.isListening()) {
    qWarning() << "[Engine] IPC unavailable:" << ipcServer.errorString();
  }

  QObject::connect(&ipcServer, &IpcServer::commandReceived,
                   [&](const QString &command, const QJsonObject &payload) {
                     if (command == QStringLiteral("reload")) {
                       overlay.reloadConfig();
                     } else if (command == QStringLiteral("theme")) {
                       overlay.switchTheme(
                           payload.value(QStringLiteral("name")).toString());
                     } else if (command == QStringLiteral("enable")) {
                       overlay.setEnabled(true);
                     } else if (command == QStringLiteral("disable")) {
                       overlay.setEnabled(false);
                     } else if (command == QStringLiteral("quit")) {
                       app.quit();
                     } else {
                       qWarning() << "[Engine] Unknown IPC command:" << command;
                     }
                   });

  qInfo() << "[Engine] Ultralight Web Cursor" << app.applicationVersion()
          << "is running";
  return app.exec();
}
