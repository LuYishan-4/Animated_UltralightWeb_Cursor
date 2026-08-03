#include <QGuiApplication>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "../header/QtCursorEffect.hpp"
#include "IpcServer.hpp"

using UltralightWebCursorM::IpcServer;

namespace {

constexpr auto kOrg  = "UltralightWebCursor";
constexpr auto kApp  = "ultralightwebcursor";
constexpr auto kSetupCompletedKey = "Setup/completed";
constexpr auto kRunKeyPath   = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr auto kRunValueName = "UltralightWebCursor";

bool isFirstRun(){
    QSettings settings(kOrg, kApp);
    return !settings.value(kSetupCompletedKey, false).toBool();
}

void markSetupCompleted(){
    QSettings settings(kOrg, kApp);
    settings.setValue(kSetupCompletedKey, true);
}

void registerAutostart(){
    QSettings runKey(kRunKeyPath, QSettings::NativeFormat);
    const QString exePath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    runKey.setValue(kRunValueName, QString("\"%1\" --silent").arg(exePath));
}
QProcess* launchSettingsGui(bool firstRun){
    const QString guiPath = QCoreApplication::applicationDirPath() + "/settings_windows.exe";
    if (!QFile::exists(guiPath))return nullptr;

    auto *gui = new QProcess(qApp);
    gui->setProgram(guiPath);
    if (firstRun) {
        gui->setArguments({"--first-run"});
    }
    gui->start();

    if (!gui->waitForStarted(5000)) {
        qCritical() << "無法啟動設定 GUI：" << gui->errorString();
        gui->deleteLater();
        return nullptr;
    }
    return gui;
}

} // namespace

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(kOrg);
    QCoreApplication::setApplicationName(kApp);

    QCommandLineParser parser;
    QCommandLineOption silentOption("silent", "以背景模式啟動，不開設定介面");
    parser.addOption(silentOption);
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
        qWarning() << "IPC 伺服器未能啟動，GUI 將無法即時通知本程式（仍可正常渲染游標效果）";
    }

    QObject::connect(&ipcServer, &IpcServer::commandReceived,
        [&](const QString &cmd, const QJsonObject &payload) {
            qDebug() << "IPC 收到指令：" << cmd;

            if (cmd == "completeSetup") {
                markSetupCompleted();
                registerAutostart();
                qDebug() << "初始設定完成，已註冊開機自動啟動。";
            } else if (cmd == "reloadConfig") {
                effect.reloadUserConfig();
            } else if (cmd == "reloadBlacklist") {
                effect.reloadBlacklist();
            } else if (cmd == "reloadTheme") {
                const QString theme = payload.value("theme").toString();
                effect.switchTheme(theme.toStdString());
            } else {
                qWarning() << "未知的 IPC 指令：" << cmd;
            }
        });

    if (isFirstRun() && !silentBoot) {
        qDebug() << "第一次執行，開啟設定介面...";
        launchSettingsGui(/*firstRun=*/true);
    }

    qDebug() << "Windows standalone cursor effect running";
    return app.exec();
}