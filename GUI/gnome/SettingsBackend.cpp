#include "SettingsBackend.hpp"
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QDebug>

using namespace UltralightWebCursorM;

SettingsBackend::SettingsBackend(QObject* parent) : QObject(parent) {
    reload();
}

bool SettingsBackend::enabled() const { return enabled_; }
QString SettingsBackend::statusMessage() const { return statusMessage_; }
QStringList SettingsBackend::blacklist() const { return blacklist_; }
QStringList SettingsBackend::themeList() const { const_cast<SettingsBackend*>(this)->loadThemes(); return themeList_; }
QString SettingsBackend::currentTheme() const { return currentTheme_; }
int SettingsBackend::cursorWidth() const { return cursorWidth_; }
int SettingsBackend::cursorHeight() const { return cursorHeight_; }

void SettingsBackend::setEnabled(bool value){ if(enabled_==value) return; enabled_ = value; Q_EMIT enabledChanged(); }
void SettingsBackend::setCursorWidth(int value){ if(cursorWidth_==value) return; cursorWidth_ = value; save(); Q_EMIT cursorWidthChanged(); }
void SettingsBackend::setCursorHeight(int value){ if(cursorHeight_==value) return; cursorHeight_ = value; save(); Q_EMIT cursorHeightChanged(); }

void SettingsBackend::setStatusMessage(const QString& msg){ statusMessage_ = msg; Q_EMIT statusMessageChanged(); }

void SettingsBackend::reload(){
    UltralightWebCursorM::UserConfig::instance()->load();
    enabled_      = UserConfigimp.enabled;
    cursorWidth_  = UserConfigimp.width;
    cursorHeight_ = UserConfigimp.height;
    currentTheme_ = QString::fromStdString(UserConfig::instance()->currentTheme());

    blacklist_.clear();
    for(const auto& item : UserConfigimp.blacklist){ blacklist_ << QString::fromStdString(item); }

    loadThemes();

    Q_EMIT enabledChanged();
    Q_EMIT blacklistChanged();
    Q_EMIT currentThemeChanged();
    Q_EMIT cursorWidthChanged();
    Q_EMIT cursorHeightChanged();

    setStatusMessage(QStringLiteral("Loaded"));
}

void SettingsBackend::save(){
    auto* userConfig = UltralightWebCursorM::UserConfig::instance();
    userConfig->setKeyValue("enabled", enabled_ ? "true" : "false");
    userConfig->setKeyValue("width", std::to_string(cursorWidth_));
    userConfig->setKeyValue("height", std::to_string(cursorHeight_));
    if(userConfig->save()){
        reconfigureSystem();
        setStatusMessage(QStringLiteral("Saved"));
    } else setStatusMessage(QStringLiteral("Save failed"));
}

void SettingsBackend::addBlacklist(const QString& app){ UltralightWebCursorM::UserConfig::instance()->appendBlacklist(app.toStdString()); reload(); }
void SettingsBackend::removeBlacklist(const QString& app){ UltralightWebCursorM::UserConfig::instance()->removeBlacklist(app.toStdString()); reload(); }

void SettingsBackend::loadThemes(){
    QStringList newThemes;
    std::filesystem::path path = g_sdkInitialPath;
    if (std::filesystem::exists(path)) {
        for (const auto& item : std::filesystem::directory_iterator(path)){
            if (item.is_directory()){
                bool hasJson = false;
                for (const auto& subItem : std::filesystem::directory_iterator(item.path())){
                    if (subItem.is_regular_file() && subItem.path().filename() == "CursorData.json"){
                        hasJson = true; break;
                    }
                }
                if (hasJson) newThemes << QString::fromStdString(item.path().filename().string());
            }
        }
    }
    if (themeList_ != newThemes){ themeList_ = newThemes; Q_EMIT themeListChanged(); }
}

bool SettingsBackend::uploadTheme(const QString& path){
    QDir dir(QDir::cleanPath(path));
    if(!dir.exists()){ setStatusMessage(QStringLiteral("Folder not found")); return false; }
    QString name = dir.dirName();
    bool ok = UltralightWebCursorM::UserConfig::instance()->uploadTheme(path.toStdString(), name.toStdString());
    if(ok){ loadThemes(); setStatusMessage(QStringLiteral("Theme uploaded")); return true; }
    setStatusMessage(QStringLiteral("Upload failed")); return false;
}

void SettingsBackend::useTheme(const QString& name){ UltralightWebCursorM::UserConfig::instance()->setTheme(name.toStdString()); reload(); }

bool SettingsBackend::removeTheme(const QString& name){
    bool ok = UltralightWebCursorM::UserConfig::instance()->removeTheme(name.toStdString());
    if(ok){ loadThemes(); setStatusMessage(QStringLiteral("Theme removed")); return true; }
    setStatusMessage(QStringLiteral("Remove failed")); return false;
}

void SettingsBackend::openThemeFolder(const QString& name){
    std::filesystem::path p = g_sdkInitialPath / name.toStdString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(p.string())));
}

QVariantMap SettingsBackend::getThemeDetails(const QString& name){
    QVariantMap details;
    std::filesystem::path p = g_sdkInitialPath / name.toStdString();
    std::filesystem::path jsonPath = p / "CursorData.json";
    if(!std::filesystem::exists(jsonPath)) return details;

    // minimal parsing via CursorJSON
    CursorJSON::instance()->load(p.string());
    details[QStringLiteral("iconPath")] = QString::fromStdString(CursorJSONImp.IconPath);
    details[QStringLiteral("author")] = QString::fromStdString(CursorJSONImp.Author);
    details[QStringLiteral("describe")] = QString::fromStdString(CursorJSONImp.describe);
    details[QStringLiteral("minWidth")] = CursorJSONImp.minWidth;
    details[QStringLiteral("minHeight")] = CursorJSONImp.minHeight;
    return details;
}

void SettingsBackend::reconfigureSystem(){ /* no-op for non-KDE platforms */ }

bool SettingsBackend::pathExists(const QString& path) const{ return QDir(path).exists(); }
