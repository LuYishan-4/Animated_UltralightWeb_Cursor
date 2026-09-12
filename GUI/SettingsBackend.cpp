#include "SettingsBackend.hpp"

#include "AppPaths.hpp"
#include "Autostart.hpp"
#include "BuildConfig.hpp"
#include "ThemeMetadata.hpp"
#include "UserConfig.hpp"

#include <QCollator>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QProcess>

#include <algorithm>
#include <utility>

using namespace UltralightWebCursor;

SettingsBackend::SettingsBackend(QObject *parent) : QObject(parent) {
  reconnectTimer_.setInterval(250);
  reconnectTimer_.setSingleShot(false);
  sizeSaveTimer_.setInterval(220);
  sizeSaveTimer_.setSingleShot(true);

  connect(&ipcSocket_, &QLocalSocket::connected, this, [this] {
    engineStartRequested_ = false;
    reconnectAttempts_ = 0;
    reconnectTimer_.stop();
    Q_EMIT mainProcessConnectedChanged();
    flushPendingCommands();
    setStatus(QStringLiteral("Cursor engine is running"),
              QStringLiteral("success"));
  });
  connect(&ipcSocket_, &QLocalSocket::disconnected, this,
          [this] { Q_EMIT mainProcessConnectedChanged(); });
  connect(&reconnectTimer_, &QTimer::timeout, this, [this] {
    if (mainProcessConnected()) {
      reconnectTimer_.stop();
      return;
    }
    if (++reconnectAttempts_ > 20) {
      reconnectTimer_.stop();
      engineStartRequested_ = false;
      pendingCommands_.clear();
      setStatus(QStringLiteral("The cursor engine did not respond"),
                QStringLiteral("error"));
      return;
    }
    connectToEngine();
  });
  connect(&sizeSaveTimer_, &QTimer::timeout, this,
          &SettingsBackend::persistSize);

  reload();
  connectToEngine();
  if (enabled_)
    QTimer::singleShot(0, this, &SettingsBackend::startEngine);
}

bool SettingsBackend::mainProcessConnected() const {
  return ipcSocket_.state() == QLocalSocket::ConnectedState;
}

QString SettingsBackend::platformName() const {
#ifdef Q_OS_WIN
  return QStringLiteral("Windows");
#else
  return QStringLiteral("Linux · X11/XWayland");
#endif
}

QString SettingsBackend::dataDirectory() const {
  return AppPaths::userDataDir();
}

bool SettingsBackend::canUninstall() const {
#ifdef Q_OS_WIN
  return QFileInfo::exists(QDir(QCoreApplication::applicationDirPath())
                               .filePath(QStringLiteral("../uninstall.exe")));
#else
  // Package-managed Linux installs must be removed with pacman/paru/yay.
  return false;
#endif
}

void SettingsBackend::setStatus(const QString &message, const QString &level) {
  if (statusMessage_ == message && statusLevel_ == level)
    return;
  statusMessage_ = message;
  statusLevel_ = level;
  Q_EMIT statusChanged();
}

void SettingsBackend::reload() {
  QString error;
  UserConfig &config = UserConfig::instance();
  if (!config.load(&error)) {
    setStatus(error, QStringLiteral("error"));
    return;
  }

  const ConfigValues &values = config.values();
  enabled_ = values.enabled;
  cursorWidth_ = values.width;
  cursorHeight_ = values.height;
  currentTheme_ = config.currentTheme();
  autostart_ = isAutostartEnabled(AppPaths::engineExecutablePath());
  if (!autostart_ && isAutostartEnabled())
    unregisterAutostart();
  loadThemes();

  Q_EMIT enabledChanged();
  Q_EMIT cursorWidthChanged();
  Q_EMIT cursorHeightChanged();
  Q_EMIT currentThemeChanged();
  Q_EMIT autostartChanged();

  setStatus(QStringLiteral("Settings are ready"), QStringLiteral("success"));
}

void SettingsBackend::loadThemes() {
  QStringList themes;
  const QDir data(AppPaths::userDataDir());
  for (const QString &name :
       data.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
    if (AppPaths::isThemeDirectory(data.filePath(name)))
      themes.append(name);
  }

  QCollator collator;
  collator.setNumericMode(true);
  std::sort(themes.begin(), themes.end(),
            [&collator](const QString &left, const QString &right) {
              return collator.compare(left, right) < 0;
            });

  if (themeList_ != themes) {
    themeList_ = themes;
    Q_EMIT themeListChanged();
  }
}

void SettingsBackend::setCursorWidth(int value) {
  value = std::clamp(value, 16, 4096);
  if (cursorWidth_ == value)
    return;
  cursorWidth_ = value;
  Q_EMIT cursorWidthChanged();
  sizeSaveTimer_.start();
}

void SettingsBackend::setCursorHeight(int value) {
  value = std::clamp(value, 16, 4096);
  if (cursorHeight_ == value)
    return;
  cursorHeight_ = value;
  Q_EMIT cursorHeightChanged();
  sizeSaveTimer_.start();
}

void SettingsBackend::persistSize() {
  QString error;
  if (!UserConfig::instance().setSize(cursorWidth_, cursorHeight_, &error)) {
    setStatus(error, QStringLiteral("error"));
    return;
  }
  if (enabled_)
    sendCommand(QStringLiteral("reload"));
  setStatus(QStringLiteral("Cursor size updated"), QStringLiteral("success"));
}

void SettingsBackend::enable() {
  QString error;
  if (!UserConfig::instance().setEnabled(true, &error)) {
    setStatus(error, QStringLiteral("error"));
    return;
  }
  if (!enabled_) {
    enabled_ = true;
    Q_EMIT enabledChanged();
  }
  sendCommand(QStringLiteral("enable"));
  setStatus(QStringLiteral("Starting cursor engine…"));
}

void SettingsBackend::disable() {
  QString error;
  if (!UserConfig::instance().setEnabled(false, &error)) {
    setStatus(error, QStringLiteral("error"));
    return;
  }
  if (enabled_) {
    enabled_ = false;
    Q_EMIT enabledChanged();
  }
  if (mainProcessConnected())
    sendCommand(QStringLiteral("disable"), {}, false);
  setStatus(QStringLiteral("Cursor disabled"), QStringLiteral("success"));
}

void SettingsBackend::startEngine() {
  if (mainProcessConnected()) {
    setStatus(QStringLiteral("Cursor engine is already running"),
              QStringLiteral("success"));
    return;
  }
  if (engineStartRequested_)
    return;

  const QString executable = AppPaths::engineExecutablePath();
  if (!QFileInfo::exists(executable)) {
    setStatus(QStringLiteral("Cursor engine executable was not found"),
              QStringLiteral("error"));
    return;
  }

  if (!QProcess::startDetached(executable, {QStringLiteral("--silent")})) {
    setStatus(QStringLiteral("Could not start the cursor engine"),
              QStringLiteral("error"));
    return;
  }

  engineStartRequested_ = true;
  reconnectAttempts_ = 0;
  reconnectTimer_.start();
  connectToEngine();
  setStatus(QStringLiteral("Connecting to cursor engine…"));
}

void SettingsBackend::connectToEngine() {
  if (ipcSocket_.state() == QLocalSocket::ConnectedState ||
      ipcSocket_.state() == QLocalSocket::ConnectingState)
    return;
  ipcSocket_.abort();
  ipcSocket_.connectToServer(QString::fromUtf8(BuildConfig::ipcSocketName),
                             QIODevice::WriteOnly);
}

void SettingsBackend::sendCommand(const QString &command,
                                  const QJsonObject &payload,
                                  bool startIfStopped) {
  const QJsonObject message{{QStringLiteral("command"), command},
                            {QStringLiteral("payload"), payload}};
  if (mainProcessConnected()) {
    ipcSocket_.write(QJsonDocument(message).toJson(QJsonDocument::Compact));
    ipcSocket_.write("\n");
    ipcSocket_.flush();
    return;
  }

  pendingCommands_.append(message);
  connectToEngine();
  if (startIfStopped)
    startEngine();
}

void SettingsBackend::flushPendingCommands() {
  if (!mainProcessConnected())
    return;
  for (const QJsonObject &message : std::as_const(pendingCommands_)) {
    ipcSocket_.write(QJsonDocument(message).toJson(QJsonDocument::Compact));
    ipcSocket_.write("\n");
  }
  pendingCommands_.clear();
  ipcSocket_.flush();
}

void SettingsBackend::setAutostart(bool value) {
  QString error;
  const bool success =
      value ? registerAutostart(AppPaths::engineExecutablePath(), &error)
            : unregisterAutostart(&error);
  if (!success) {
    setStatus(error, QStringLiteral("error"));
    Q_EMIT autostartChanged();
    return;
  }

  autostart_ = value;
  Q_EMIT autostartChanged();
  setStatus(value ? QStringLiteral("Launch on login enabled")
                  : QStringLiteral("Launch on login disabled"),
            QStringLiteral("success"));
}

void SettingsBackend::uploadTheme(const QUrl &folderUrl) {
  if (!folderUrl.isLocalFile()) {
    setStatus(QStringLiteral("Only local theme folders can be imported"),
              QStringLiteral("error"));
    return;
  }

  QString importedName;
  QString error;
  UserConfig &config = UserConfig::instance();
  if (!config.importTheme(folderUrl.toLocalFile(), &importedName, &error) ||
      !config.setTheme(importedName, &error)) {
    setStatus(error, QStringLiteral("error"));
    return;
  }

  currentTheme_ = importedName;
  loadThemes();
  Q_EMIT currentThemeChanged();
  if (enabled_)
    sendCommand(QStringLiteral("reload"));
  setStatus(QStringLiteral("Theme imported and applied"),
            QStringLiteral("success"));
}

void SettingsBackend::useTheme(const QString &name) {
  QString error;
  if (!UserConfig::instance().setTheme(name, &error)) {
    setStatus(error, QStringLiteral("error"));
    return;
  }
  currentTheme_ = name;
  Q_EMIT currentThemeChanged();
  if (enabled_)
    sendCommand(QStringLiteral("reload"));
  setStatus(QStringLiteral("Theme applied"), QStringLiteral("success"));
}

void SettingsBackend::removeTheme(const QString &name) {
  QString error;
  if (!UserConfig::instance().removeTheme(name, &error)) {
    setStatus(error, QStringLiteral("error"));
    return;
  }
  loadThemes();
  setStatus(QStringLiteral("Theme removed"), QStringLiteral("success"));
}

void SettingsBackend::openThemeFolder(const QString &name) {
  if (name.isEmpty() || name.contains(QLatin1Char('/')) ||
      name.contains(QLatin1Char('\\')))
    return;
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(QDir(AppPaths::userDataDir()).filePath(name)));
}

void SettingsBackend::openDataDirectory() {
  QDesktopServices::openUrl(QUrl::fromLocalFile(AppPaths::userDataDir()));
}

QVariantMap SettingsBackend::getThemeDetails(const QString &name) const {
  QVariantMap result{{QStringLiteral("displayName"), name},
                     {QStringLiteral("iconPath"), QString()},
                     {QStringLiteral("author"), QStringLiteral("Unknown")},
                     {QStringLiteral("description"), QString()},
                     {QStringLiteral("minWidth"), 128},
                     {QStringLiteral("minHeight"), 128},
                     {QStringLiteral("builtIn"), true},
                     {QStringLiteral("removable"), false}};

  if (name.isEmpty() || name.contains(QLatin1Char('/')) ||
      name.contains(QLatin1Char('\\')))
    return result;

  const QString themeDirectory = QDir(AppPaths::userDataDir()).filePath(name);
  ThemeMetadata metadata;
  if (!ThemeMetadataReader::load(themeDirectory, metadata))
    return result;

  QString iconUrl;
  if (!metadata.iconPath.isEmpty()) {
    const QFileInfo icon(metadata.iconPath);
    const QString iconFile =
        icon.isAbsolute() ? metadata.iconPath
                          : QDir(themeDirectory).filePath(metadata.iconPath);
    iconUrl = QUrl::fromLocalFile(iconFile).toString();
  }

  const bool builtIn = AppPaths::isBuiltInTheme(name);
  result[QStringLiteral("displayName")] =
      metadata.displayName.isEmpty() ? name : metadata.displayName;
  result[QStringLiteral("iconPath")] = iconUrl;
  result[QStringLiteral("author")] = metadata.author;
  result[QStringLiteral("description")] = metadata.description;
  result[QStringLiteral("minWidth")] = metadata.minWidth;
  result[QStringLiteral("minHeight")] = metadata.minHeight;
  result[QStringLiteral("builtIn")] = builtIn;
  result[QStringLiteral("removable")] = !builtIn && name != currentTheme_;
  return result;
}

void SettingsBackend::uninstall() {
#ifdef Q_OS_WIN
  const QString uninstaller = QDir(QCoreApplication::applicationDirPath())
                                  .filePath(QStringLiteral("../uninstall.exe"));
  if (!QFileInfo::exists(uninstaller) ||
      !QProcess::startDetached(uninstaller, {})) {
    setStatus(QStringLiteral("Could not start the uninstaller"),
              QStringLiteral("error"));
    return;
  }
  QCoreApplication::quit();
#else
  setStatus(QStringLiteral("Remove this package with: yay -Rns "
                           "ultralightwebcursor-git"),
            QStringLiteral("info"));
#endif
}
