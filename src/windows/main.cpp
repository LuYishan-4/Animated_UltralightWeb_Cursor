#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>

#include "../header/QtCursorEffect.hpp"
#include "../lib/Autostart/Autostart.hpp"
#include "../lib/Ipc/IpcServer.hpp"

using UltralightWebCursorM::IpcServer;

namespace {

const QString kOrg = QStringLiteral("UltralightWebCursor");
const QString kApp = QStringLiteral("ultralightwebcursor");
const QString kSetupCompletedKey = QStringLiteral("Setup/completed");

bool isFirstRun() {
  QSettings settings(kOrg, kApp);
  return !settings.value(kSetupCompletedKey, false).toBool();
}

void markSetupCompleted() {
  QSettings settings(kOrg, kApp);
  settings.setValue(kSetupCompletedKey, true);
}

QString guiExecutablePath() {
  const QString name =
#if defined(_WIN32)
      QStringLiteral("ultralightwebcursor-gui.exe");
#else
      QStringLiteral("ultralightwebcursor-gui");
#endif
  return QCoreApplication::applicationDirPath() + QDir::separator() + name;
}

QProcess *launchSettingsGui() {
  const QString guiPath = guiExecutablePath();
  if (!QFile::exists(guiPath))
    return nullptr;

  auto *gui = new QProcess(qApp);
  gui->setProgram(guiPath);
  gui->start();

  if (!gui->waitForStarted(5000)) {
    qCritical() << "Failed to launch settings GUI:" << gui->errorString();
    gui->deleteLater();
    return nullptr;
  }
  return gui;
}

} // namespace

int main(int argc, char **argv) {
  QGuiApplication app(argc, argv);
  QCoreApplication::setOrganizationName(kOrg);
  QCoreApplication::setApplicationName(kApp);

  QCommandLineParser parser;
  QCommandLineOption silentOption(QStringLiteral("silent"),
                                  QStringLiteral("Background mode, skip GUI"));
  parser.addOption(silentOption);
  parser.addHelpOption();
  parser.process(app);
  const bool silentBoot = parser.isSet(silentOption);

  UltralightWebCursorM::QtCursorEffect effect;
  if (!effect.initialize()) {
    qCritical() << "Failed to initialize Windows standalone effect";
    return 1;
  }
  effect.start();

  IpcServer ipcServer;
  if (!ipcServer.isListening()) {
    qWarning() << "IPC server failed to start; the GUI will not be able to "
                  "control this process live";
  }

  QObject::connect(
      &ipcServer, &IpcServer::commandReceived,
      [&](const QString &cmd, const QJsonObject &payload) {
        if (cmd == QStringLiteral("completeSetup")) {
          markSetupCompleted();
          UltralightWebCursorM::registerAutostart();
        } else if (cmd == QStringLiteral("reload")) {
          effect.reloadUserConfig();
        } else if (cmd == QStringLiteral("reloadTheme")) {
          effect.switchTheme(
              payload.value(QStringLiteral("theme")).toString().toStdString());
        } else if (cmd == QStringLiteral("reloadBlacklist")) {
          effect.reloadBlacklist();
        } else if (cmd == QStringLiteral("enable")) {
          effect.setEffectEnabled(true);
        } else if (cmd == QStringLiteral("disable")) {
          effect.setEffectEnabled(false);
        } else if (cmd == QStringLiteral("setAutostart")) {
          if (payload.value(QStringLiteral("enabled")).toBool())
            UltralightWebCursorM::registerAutostart();
          else
            UltralightWebCursorM::unregisterAutostart();
        } else if (cmd == QStringLiteral("quit")) {
          app.quit();
        } else {
          qWarning() << "Unknown IPC command:" << cmd;
        }
      });

  if (isFirstRun()) {
    markSetupCompleted();
    UltralightWebCursorM::registerAutostart();
    if (!silentBoot)
      launchSettingsGui();
  }

  qDebug() << "Windows standalone cursor effect running";
  return app.exec();
}
