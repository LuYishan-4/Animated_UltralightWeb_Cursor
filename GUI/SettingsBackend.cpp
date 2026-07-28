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
    const_cast<SettingsBackend*>(this)->loadThemes(); 
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
    save();
    Q_EMIT cursorWidthChanged();
}

void SettingsBackend::setCursorHeight(int value)
{
    if(cursorHeight_ == value)
        return;

    cursorHeight_ = value;
    save();
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
    enabled_      = UserConfigimp.enabled;
    cursorWidth_  = UserConfigimp.width; 
    cursorHeight_ = UserConfigimp.height;   
    currentTheme_ = QString::fromStdString(UserConfig::instance()->currentTheme());

    blacklist_.clear();
    for(const auto& item : UserConfigimp.blacklist)
    {
        blacklist_ << QString::fromStdString(item);
    }

    loadThemes();

    Q_EMIT enabledChanged();
    Q_EMIT blacklistChanged();
    Q_EMIT currentThemeChanged();
    Q_EMIT cursorWidthChanged();
    Q_EMIT cursorHeightChanged();


    setStatusMessage(
        QStringLiteral("Loaded")
    );
}

void SettingsBackend::save()
{
    auto* userConfig = UltralightWebCursorM::UserConfig::instance();


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
    QStringList newThemes;
    std::filesystem::path path = g_sdkInitialPath;

    if (std::filesystem::exists(path)) {
        for (const auto& item : std::filesystem::directory_iterator(path)) {
            if (item.is_directory()) {
                bool hasJson = false;
                for (const auto& subItem : std::filesystem::directory_iterator(item.path())) {
                if (subItem.is_regular_file() && subItem.path().filename() == "CursorData.json") {
                            hasJson = true;
                            break;
                    }
                }
                if (hasJson) {
                    newThemes << QString::fromStdString(item.path().filename().string());
                }
            }
        }
    }
    if (themeList_ != newThemes) {
        themeList_ = newThemes;
        Q_EMIT themeListChanged();
    }
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

    return false;
}

qDebug()
    << "KAuth result:"
    << job->data();

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
    reconfigureKWin();
}

bool SettingsBackend::removeTheme(const QString& name)
{
    qDebug() << "removeTheme name =" << name;

    if (name.isEmpty())
    {
        setStatusMessage(QStringLiteral("Theme name is empty"));
        return false;
    }
    KAuth::Action action(QStringLiteral("org.ultralightwebcursor.install"));
    action.setHelperId(QStringLiteral("org.ultralightwebcursor"));
    QVariantMap args;
    args.insert(QStringLiteral("name"), name);
    args.insert(QStringLiteral("path"), QString());
    args.insert(QStringLiteral("action"), QStringLiteral("uninstall")); 

    action.setArguments(args);

    KAuth::ExecuteJob *job = action.execute(); 
    if (!job)
    {
        setStatusMessage(QStringLiteral("Failed to create auth job"));
        return false;
    }

    if (!job->exec())
    {
        qDebug() << "KAuth error:" << job->errorText();
        setStatusMessage(QStringLiteral("Authorization failed"));
        return false;
    }

    qDebug() << "KAuth result:" << job->data();
    QVariantMap result = job->data();

    bool success = result.value(QStringLiteral("success")).toBool();

    if (success)
    {
        loadThemes();
        setStatusMessage(QStringLiteral("Theme removed"));
        return true;
    }

    setStatusMessage(QStringLiteral("Remove failed"));
    return false;
}
QVariantMap SettingsBackend::getThemeDetails(const QString& name)
{
    QVariantMap details;
    
    qDebug() << "[UltralightCursorEffect] Loading cursor config.1111111111111111111111..";
    details[QStringLiteral("iconPath")] = QString();
    details[QStringLiteral("author")] = QStringLiteral("Unknown");
    details[QStringLiteral("describe")] = QString();
    details[QStringLiteral("minWidth")] = 128;
    details[QStringLiteral("minHeight")] = 128;

    if (name.isEmpty()) {
        return details;
    }
    std::filesystem::path themePath = g_sdkInitialPath / name.toStdString();
    bool loadSuccess = UltralightWebCursorM::CursorJSON::instance()->load(themePath.string());
    
    if (loadSuccess) {
        auto values = UltralightWebCursorM::CursorJSON::instance()->values; 
        QString rawIconPath = QString::fromStdString(values.IconPath);
        QString fullIconUrl;

        if (!rawIconPath.isEmpty()) {
            if (rawIconPath.startsWith(QLatin1String("/")) || rawIconPath.startsWith(QLatin1String("file://"))) {
                fullIconUrl = rawIconPath.startsWith(QLatin1String("/")) ? QStringLiteral("file://") + rawIconPath : rawIconPath;
            } else {
                QString absoluteThemeDir = QString::fromStdString(themePath.string());
                fullIconUrl = QStringLiteral("file://") + absoluteThemeDir + QStringLiteral("/") + rawIconPath;
            }
        }
        qDebug() << "[UltralightCursorEffect] sssssssssor config...";
        details[QStringLiteral("iconPath")] = fullIconUrl;
        details[QStringLiteral("author")] = QString::fromStdString(values.Author);
        details[QStringLiteral("describe")] = QString::fromStdString(values.describe);
        details[QStringLiteral("minWidth")] = values.minWidth;
        details[QStringLiteral("minHeight")] = values.minHeight;
    }

    return details;
}


void SettingsBackend::openThemeFolder(const QString& name)
{
    QString path = QString::fromStdString(
        (g_sdkInitialPath  / name.toStdString()).string()
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
