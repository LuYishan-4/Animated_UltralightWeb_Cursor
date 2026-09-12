#pragma once

#include <QJsonObject>
#include <QList>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>

class SettingsBackend final : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
  Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusChanged)
  Q_PROPERTY(QString statusLevel READ statusLevel NOTIFY statusChanged)
  Q_PROPERTY(QStringList themeList READ themeList NOTIFY themeListChanged)
  Q_PROPERTY(QString currentTheme READ currentTheme NOTIFY currentThemeChanged)
  Q_PROPERTY(int cursorWidth READ cursorWidth WRITE setCursorWidth NOTIFY
                 cursorWidthChanged)
  Q_PROPERTY(int cursorHeight READ cursorHeight WRITE setCursorHeight NOTIFY
                 cursorHeightChanged)
  Q_PROPERTY(bool autostart READ autostart NOTIFY autostartChanged)
  Q_PROPERTY(bool mainProcessConnected READ mainProcessConnected NOTIFY
                 mainProcessConnectedChanged)
  Q_PROPERTY(QString platformName READ platformName CONSTANT)
  Q_PROPERTY(QString dataDirectory READ dataDirectory CONSTANT)
  Q_PROPERTY(bool canUninstall READ canUninstall CONSTANT)

public:
  explicit SettingsBackend(QObject *parent = nullptr);

  bool enabled() const { return enabled_; }
  QString statusMessage() const { return statusMessage_; }
  QString statusLevel() const { return statusLevel_; }
  QStringList themeList() const { return themeList_; }
  QString currentTheme() const { return currentTheme_; }
  int cursorWidth() const { return cursorWidth_; }
  int cursorHeight() const { return cursorHeight_; }
  bool autostart() const { return autostart_; }
  bool mainProcessConnected() const;
  QString platformName() const;
  QString dataDirectory() const;
  bool canUninstall() const;

  void setCursorWidth(int value);
  void setCursorHeight(int value);

  Q_INVOKABLE void reload();
  Q_INVOKABLE void enable();
  Q_INVOKABLE void disable();
  Q_INVOKABLE void startEngine();
  Q_INVOKABLE void setAutostart(bool value);
  Q_INVOKABLE void uploadTheme(const QUrl &folderUrl);
  Q_INVOKABLE void useTheme(const QString &name);
  Q_INVOKABLE void removeTheme(const QString &name);
  Q_INVOKABLE void openThemeFolder(const QString &name);
  Q_INVOKABLE void openDataDirectory();
  Q_INVOKABLE QVariantMap getThemeDetails(const QString &name) const;
  Q_INVOKABLE void uninstall();

Q_SIGNALS:
  void enabledChanged();
  void statusChanged();
  void themeListChanged();
  void currentThemeChanged();
  void cursorWidthChanged();
  void cursorHeightChanged();
  void autostartChanged();
  void mainProcessConnectedChanged();

private:
  void setStatus(const QString &message,
                 const QString &level = QStringLiteral("info"));
  void loadThemes();
  void connectToEngine();
  void sendCommand(const QString &command, const QJsonObject &payload = {},
                   bool startIfStopped = true);
  void flushPendingCommands();
  void persistSize();

  bool enabled_ = true;
  bool autostart_ = false;
  int cursorWidth_ = 128;
  int cursorHeight_ = 128;
  QString currentTheme_;
  QStringList themeList_;
  QString statusMessage_;
  QString statusLevel_ = QStringLiteral("info");

  QLocalSocket ipcSocket_;
  QTimer reconnectTimer_;
  QTimer sizeSaveTimer_;
  QList<QJsonObject> pendingCommands_;
  int reconnectAttempts_ = 0;
  bool engineStartRequested_ = false;
};
