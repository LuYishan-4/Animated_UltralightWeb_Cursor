#include "SettingsBackend.hpp"
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <filesystem>
#include <iostream>

using namespace UltralightWebCursorM;

SettingsBackend::SettingsBackend(QObject* parent)
    : QObject(parent)
{
    reload();
}

QString SettingsBackend::htmlPath() const
{
    return htmlPath_;
}

QString SettingsBackend::sdkPath() const
{
    return sdkPath_;
}

bool SettingsBackend::enabled() const
{
    return enabled_;
}

QString SettingsBackend::statusMessage() const
{
    return statusMessage_;
}

QStringList SettingsBackend::blacklist() const
{
    return blacklist_;
}

QStringList SettingsBackend::themeList() const
{
    return themeList_;
}

QString SettingsBackend::currentTheme() const
{
    return currentTheme_;
}

int SettingsBackend::cursorWidth() const
{
    return cursorWidth_;
}

int SettingsBackend::cursorHeight() const
{
    return cursorHeight_;
}
 
bool SettingsBackend::autoHide() const
{
    return autoHide_;
}

void SettingsBackend::setHtmlPath(const QString& path)
{
    if(htmlPath_ == path)
        return;

    htmlPath_ = path;

    QFileInfo info(path);
    QString themeFolder = info.absolutePath();
    QString themeName = QFileInfo(themeFolder).fileName();

    if(!themeFolder.isEmpty() && !themeName.isEmpty()){
        UltralightWebCursorM::UserConfig::instance()->uploadTheme(
            themeFolder.toStdString(),
            themeName.toStdString()
        );
    }

    Q_EMIT htmlPathChanged();
}

void SettingsBackend::setSdkPath(const QString& path)
{
    if(sdkPath_ == path)
        return;

    sdkPath_ = path;
    Q_EMIT sdkPathChanged();
}

void SettingsBackend::setEnabled(bool value)
{
    if(enabled_ == value)
        return;

    enabled_ = value;
    Q_EMIT enabledChanged();
}

void SettingsBackend::setCursorWidth(int value)
{
    if(cursorWidth_ == value)
        return;

    cursorWidth_ = value;
    Q_EMIT cursorWidthChanged();
}

void SettingsBackend::setCursorHeight(int value)
{
    if(cursorHeight_ == value)
        return;

    cursorHeight_ = value;
    Q_EMIT cursorHeightChanged();
}

void SettingsBackend::setStatusMessage(const QString& msg)
{
    statusMessage_ = msg;
    Q_EMIT statusMessageChanged();
}

void SettingsBackend::reload()
{
    UltralightWebCursorM::UserConfig::instance()->load();

    htmlPath_     = QString::fromStdString(UserConfigimp.html);
    sdkPath_      = QString::fromStdString(UserConfigimp.sdk);
    enabled_      = UserConfigimp.enabled;
    autoHide_     = UserConfigimp.isautohide; 
    cursorWidth_  = UserConfigimp.width; 
    cursorHeight_ = UserConfigimp.height;   
    currentTheme_ = QString::fromStdString(UserConfig::instance()->currentTheme());

    blacklist_.clear();
    for(const auto& item : UserConfigimp.blacklist)
    {
        blacklist_ << QString::fromStdString(item);
    }

    loadThemes();

    Q_EMIT htmlPathChanged();
    Q_EMIT sdkPathChanged();
    Q_EMIT enabledChanged();
    Q_EMIT blacklistChanged();
    Q_EMIT currentThemeChanged();
    Q_EMIT cursorWidthChanged();
    Q_EMIT cursorHeightChanged();
    Q_EMIT autoHideChanged();

    setStatusMessage(
        QStringLiteral("Loaded")
    );
}

void SettingsBackend::save()
{
    auto* userConfig = UltralightWebCursorM::UserConfig::instance();

    userConfig->setKeyValue(
        "html",
        htmlPath_.toStdString()
    );

    userConfig->setKeyValue(
        "sdk",
        sdkPath_.toStdString()
    );

    userConfig->setKeyValue(
        "enabled",
        enabled_ ? "true" : "false"
    );

    userConfig->setKeyValue(
        "width",
        std::to_string(cursorWidth_)
    );

    userConfig->setKeyValue(
        "height",
        std::to_string(cursorHeight_)
    );


    userConfig->setKeyValue(
        "isautohide",
        autoHide_ ? "true" : "false"
    );


    if(userConfig->save()){
        reconfigureKWin();
        setStatusMessage(
            QStringLiteral("Saved")
        );
    }
    else
    {
        setStatusMessage(
            QStringLiteral("Save failed")
        );
    }
}

void SettingsBackend::addBlacklist(const QString& app){
    UltralightWebCursorM::UserConfig::instance()->appendBlacklist(
        app.toStdString()
    );

    reload();
}

void SettingsBackend::removeBlacklist(const QString& app){
    UltralightWebCursorM::UserConfig::instance()->removeBlacklist(
        app.toStdString()
    );

    reload();
}

void SettingsBackend::loadThemes()
{
    themeList_.clear();

    std::filesystem::path path = g_sdkInitialPath / "resources";

    if(!std::filesystem::exists(path))
    {
        Q_EMIT themeListChanged();
        return;
    }
    for(auto& item : std::filesystem::directory_iterator(path))
    {
        if(item.is_directory())
        {
            themeList_ << QString::fromStdString(
                item.path().filename().string()
            );
        }
    }

    Q_EMIT themeListChanged();
}

void SettingsBackend::setAutoHide(bool value)
{
    if(autoHide_ == value)
        return;
 
    autoHide_ = value;
    Q_EMIT autoHideChanged();
}


bool SettingsBackend::uploadTheme(const QString& path)
{
    qDebug() << "uploadTheme path =" << path;


    QDir dir(
        QDir::cleanPath(path)
    );


    if(!dir.exists())
    {
        setStatusMessage(
            QStringLiteral("Folder not found")
        );

        return false;
    }



    QString name =
        dir.dirName();


    qDebug()
        << "uploadTheme name ="
        << name;



    KAuth::Action action(
        QStringLiteral(
            "org.ultralightwebcursor.install"
        )
    );


    action.setHelperId(
        QStringLiteral(
            "org.ultralightwebcursor"
        )
    );



    QVariantMap args;


    args.insert(
        QStringLiteral("path"),
        path
    );


    args.insert(
        QStringLiteral("name"),
        name
    );


    action.setArguments(args);



    KAuth::ExecuteJob *job =
        action.execute();



    if(!job)
    {
        setStatusMessage(
            QStringLiteral(
                "Failed to create auth job"
            )
        );

        return false;
    }



    if(!job->exec())
    {
        qDebug()
            << "KAuth error:"
            << job->errorText();


        setStatusMessage(
            QStringLiteral(
                "Authentication failed"
            )
        );

        return false;
    }



    QVariantMap result =
        job->data();



    bool success =
        result.value(
            QStringLiteral("success")
        ).toBool();



    if(success)
    {
        loadThemes();


        setStatusMessage(
            QStringLiteral(
                "Theme uploaded"
            )
        );

        return true;
    }



    setStatusMessage(
        QStringLiteral(
            "Upload failed"
        )
    );


    return false;
}

void SettingsBackend::useTheme(const QString& name)
{
    UltralightWebCursorM::UserConfig::instance()->setTheme(
        name.toStdString()
    );

    reload();
}

void SettingsBackend::removeTheme(const QString& name)
{
    std::filesystem::remove_all(
        g_sdkInitialPath / "resources" / name.toStdString()
    );

    loadThemes();
    reload();
}

void SettingsBackend::openThemeFolder(const QString& name)
{
    QString path = QString::fromStdString(
        (g_sdkInitialPath / "resources" / name.toStdString()).string()
    );

    QDesktopServices::openUrl(
        QUrl::fromLocalFile(path)
    );
}

bool SettingsBackend::pathExists(const QString& path) const
{
    return QFileInfo::exists(path);
}

void SettingsBackend::enable()
{
    QDBusInterface effect(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/UltralightCursor"),
        QStringLiteral("org.kde.kwin.KWin.UltralightCursorEffect"),
        QDBusConnection::sessionBus()
    );

    auto reply = effect.call(QStringLiteral("enable"));

    if(reply.type() == QDBusMessage::ErrorMessage)
    {
        setStatusMessage(
            QStringLiteral("Enable failed")
        );
    }
    else
    {
        enabled_ = true;
        Q_EMIT enabledChanged();
        setStatusMessage(
            QStringLiteral("Enabled")
        );
    }
}

void SettingsBackend::disable()
{
    QDBusInterface effect(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/UltralightCursor"),
        QStringLiteral("org.kde.kwin.KWin.UltralightCursorEffect"),
        QDBusConnection::sessionBus()
    );

    auto reply = effect.call(QStringLiteral("disable"));

    if(reply.type() == QDBusMessage::ErrorMessage)
    {
        setStatusMessage(
            QStringLiteral("Disable failed")
        );
    }
    else
    {
        enabled_ = false;
        Q_EMIT enabledChanged();
        setStatusMessage(
            QStringLiteral("Disabled")
        );
    }
}
void SettingsBackend::reconfigureKWin()
{
    QDBusInterface effect(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/UltralightCursor"),
        QStringLiteral("org.kde.kwin.KWin.UltralightCursorEffect"),
        QDBusConnection::sessionBus()
    );
    effect.call(QStringLiteral("reloadHtml"));
}
