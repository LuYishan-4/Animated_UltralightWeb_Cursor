#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "../src/config/UserConfig.hpp"



class SettingsBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString htmlPath READ htmlPath WRITE setHtmlPath NOTIFY htmlPathChanged)
    Q_PROPERTY(QString sdkPath READ sdkPath WRITE setSdkPath NOTIFY sdkPathChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

    Q_PROPERTY(QStringList blacklist READ blacklist NOTIFY blacklistChanged)
    Q_PROPERTY(QStringList themeList READ themeList NOTIFY themeListChanged)
    Q_PROPERTY(QString currentTheme READ currentTheme NOTIFY currentThemeChanged)

    Q_PROPERTY(int cursorWidth READ cursorWidth WRITE setCursorWidth NOTIFY cursorWidthChanged)
    Q_PROPERTY(int cursorHeight READ cursorHeight WRITE setCursorHeight NOTIFY cursorHeightChanged)

     Q_PROPERTY(bool autoHide READ autoHide WRITE setAutoHide NOTIFY autoHideChanged)
 
public:

    explicit SettingsBackend(QObject* parent = nullptr);




    QString htmlPath() const;
    QString sdkPath() const;
    bool enabled() const;
    QString statusMessage() const;

    QStringList blacklist() const;
    QStringList themeList() const;
    QString currentTheme() const;

    int cursorWidth() const;
    int cursorHeight() const;
 bool autoHide() const;


    void setHtmlPath(const QString& path);
    void setSdkPath(const QString& path);
    void setEnabled(bool value);
    void setCursorWidth(int value);
    void setCursorHeight(int value);

  void setAutoHide(bool value);
    Q_INVOKABLE void save();

    Q_INVOKABLE void reload();

    Q_INVOKABLE bool pathExists(const QString& path) const;




    Q_INVOKABLE bool uploadTheme(const QString& path);

    Q_INVOKABLE void useTheme(const QString& name);

    Q_INVOKABLE void removeTheme(const QString& name);

    Q_INVOKABLE void openThemeFolder(const QString& name);



    Q_INVOKABLE void addBlacklist(const QString& app);

    Q_INVOKABLE void removeBlacklist(const QString& app);


public Q_SLOTS:

    void enable();

    void disable();

    void reconfigureKWin();


Q_SIGNALS:

    void htmlPathChanged();
    void sdkPathChanged();
    void enabledChanged();
    void statusMessageChanged();

    void blacklistChanged();
    void themeListChanged();
    void currentThemeChanged();
    void autoHideChanged();
    void cursorWidthChanged();
    void cursorHeightChanged();


private:

    void setStatusMessage(const QString& message);

    void loadThemes();

    QString htmlPath_;
    QString sdkPath_;
    bool enabled_ = true;

    QStringList blacklist_;
    QStringList themeList_;
    QString currentTheme_;
    bool autoHide_ = true;
    int cursorWidth_ = 128;
    int cursorHeight_ = 128;

    QString statusMessage_;

};