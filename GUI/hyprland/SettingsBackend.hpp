#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <QVariantMap>
#include <filesystem>
#include "../../src/config/UserConfig.hpp"
#include "../../src/config/CursorJSON.hpp"

class SettingsBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QStringList blacklist READ blacklist NOTIFY blacklistChanged)
    Q_PROPERTY(QStringList themeList READ themeList NOTIFY themeListChanged)
    Q_PROPERTY(QString currentTheme READ currentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(int cursorWidth READ cursorWidth WRITE setCursorWidth NOTIFY cursorWidthChanged)
    Q_PROPERTY(int cursorHeight READ cursorHeight WRITE setCursorHeight NOTIFY cursorHeightChanged)
public:
    explicit SettingsBackend(QObject* parent = nullptr);
    bool enabled() const;
    QString statusMessage() const;
    QStringList blacklist() const;
    QStringList themeList() const;
    QString currentTheme() const;
    int cursorWidth() const;
    int cursorHeight() const;
    void setEnabled(bool value);
    void setCursorWidth(int value);
    void setCursorHeight(int value);
    Q_INVOKABLE void save();
    Q_INVOKABLE void reload();
    Q_INVOKABLE bool pathExists(const QString& path) const;
    Q_INVOKABLE bool uploadTheme(const QString& path);
    Q_INVOKABLE void useTheme(const QString& name);
    Q_INVOKABLE bool removeTheme(const QString& name);
    Q_INVOKABLE void openThemeFolder(const QString& name);
    Q_INVOKABLE QVariantMap getThemeDetails(const QString& name);
    Q_INVOKABLE void addBlacklist(const QString& app);
    Q_INVOKABLE void removeBlacklist(const QString& app);
public Q_SLOTS:
    void enable();
    void disable();
    void reconfigureSystem();
Q_SIGNALS:
    void enabledChanged();
    void statusMessageChanged();
    void blacklistChanged();
    void themeListChanged();
    void currentThemeChanged();
    void cursorWidthChanged();
    void cursorHeightChanged();
private:
    void setStatusMessage(const QString& message);
    void loadThemes();
    bool enabled_ = true;
    QStringList blacklist_;
    QStringList themeList_;
    QString currentTheme_;
    int cursorWidth_ = 128;
    int cursorHeight_ = 128;
    QString statusMessage_;
};
