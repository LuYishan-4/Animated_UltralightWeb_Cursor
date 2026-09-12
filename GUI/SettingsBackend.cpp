#include "SettingsBackend.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QUrl>

#include <filesystem>

#if defined(BUILD_TYPE_KWIN)
#include <QDBusConnection>
#include <QDBusInterface>
#endif

using namespace UltralightWebCursorM;

namespace {

constexpr auto kIpcSocketName = "ultralightwebcursor_ipc";
constexpr int kIpcConnectTimeoutMs = 500;

namespace fs = std::filesystem;

bool hasThemeLayout(const fs::path &dir) {
  return fs::exists(dir / "CursorData.json") && fs::exists(dir / "index.html");
}

} // namespace

SettingsBackend::SettingsBackend(QObject *parent) : QObject(parent) {
  connect(&ipcSocket_, &QLocalSocket::connected, this,
          [this]() { Q_EMIT mainProcessConnectedChanged(); });
  connect(&ipcSocket_, &QLocalSocket::disconnected, this,
          [this]() { Q_EMIT mainProcessConnectedChanged(); });

#if !defined(BUILD_TYPE_KWIN)
  ensureConnected();
#endif

  reload();
}

bool SettingsBackend::enabled() const { return enabled_; }
QString SettingsBackend::statusMessage() const { return statusMessage_; }
QStringList SettingsBackend::blacklist() const { return blacklist_; }
QString SettingsBackend::currentTheme() const { return currentTheme_; }
int SettingsBackend::cursorWidth() const { return cursorWidth_; }
int SettingsBackend::cursorHeight() const { return cursorHeight_; }
bool SettingsBackend::gpuRender() const { return gpuRender_; }
bool SettingsBackend::autostart() const { return autostart_; }

QStringList SettingsBackend::themeList() const {
  const_cast<SettingsBackend *>(this)->loadThemes();
  return themeList_;
}

bool SettingsBackend::mainProcessConnected() const {
#if defined(BUILD_TYPE_KWIN)
  return QDBusConnection::sessionBus().isConnected();
#else
  return ipcSocket_.state() == QLocalSocket::ConnectedState;
#endif
}

void SettingsBackend::setEnabled(bool value) {
  if (enabled_ == value)
    return;
  enabled_ = value;
  Q_EMIT enabledChanged();
}

void SettingsBackend::setCursorWidth(int value) {
  if (cursorWidth_ == value)
    return;
  cursorWidth_ = value;
  Q_EMIT cursorWidthChanged();
  save();
  reconfigureSystem();
}

void SettingsBackend::setCursorHeight(int value) {
  if (cursorHeight_ == value)
    return;
  cursorHeight_ = value;
  Q_EMIT cursorHeightChanged();
  save();
  reconfigureSystem();
}

void SettingsBackend::setGpuRender(bool value) {
  if (gpuRender_ == value)
    return;
  gpuRender_ = value;
  Q_EMIT gpuRenderChanged();
  save();
  reconfigureSystem();
}

void SettingsBackend::setStatusMessage(const QString &message) {
  if (statusMessage_ == message)
    return;
  statusMessage_ = message;
  Q_EMIT statusMessageChanged();
}

void SettingsBackend::reload() {
  UltralightWebCursorM::UserConfig::instance()->load();

  enabled_ = UserConfigValues.enabled;
  cursorWidth_ = UserConfigValues.width;
  cursorHeight_ = UserConfigValues.height;
  gpuRender_ = UserConfigValues.enableGpu;
  currentTheme_ =
      QString::fromStdString(UserConfig::instance()->currentTheme());

  blacklist_.clear();
  for (const auto &item : UserConfigValues.blacklist)
    blacklist_ << QString::fromStdString(item);

  loadThemes();

  Q_EMIT enabledChanged();
  Q_EMIT blacklistChanged();
  Q_EMIT currentThemeChanged();
  Q_EMIT cursorWidthChanged();
  Q_EMIT cursorHeightChanged();
  Q_EMIT gpuRenderChanged();

  setStatusMessage(QStringLiteral("Loaded"));
}

void SettingsBackend::save() {
  auto *uc = UltralightWebCursorM::UserConfig::instance();
  uc->setKeyValue("enabled", enabled_ ? "true" : "false");
  uc->setKeyValue("width", std::to_string(cursorWidth_));
  uc->setKeyValue("height", std::to_string(cursorHeight_));
  uc->setKeyValue("EnableGPU", gpuRender_ ? "true" : "false");

  if (uc->save())
    setStatusMessage(QStringLiteral("Saved"));
  else
    setStatusMessage(QStringLiteral("Save failed"));
}

void SettingsBackend::addBlacklist(const QString &app) {
  UltralightWebCursorM::UserConfig::instance()->appendBlacklist(
      app.toStdString());
  reload();
  reconfigureSystem();
}

void SettingsBackend::removeBlacklist(const QString &app) {
  UltralightWebCursorM::UserConfig::instance()->removeBlacklist(
      app.toStdString());
  reload();
  reconfigureSystem();
}

void SettingsBackend::loadThemes() {
  QStringList newThemes;
  const fs::path dir = g_sdkInitialPath;
  std::error_code ec;
  if (fs::exists(dir)) {
    fs::directory_iterator it(dir, ec);
    const fs::directory_iterator end;
    while (!ec && it != end) {
      const fs::directory_entry entry = *it;
      if (entry.is_directory() && hasThemeLayout(entry.path()))
        newThemes << QString::fromStdString(entry.path().filename().string());
      it.increment(ec);
    }
  }

  if (themeList_ != newThemes) {
    themeList_ = newThemes;
    Q_EMIT themeListChanged();
  }
}

bool SettingsBackend::uploadTheme(const QString &path) {
  const QDir srcDir(QDir::cleanPath(path));
  if (!srcDir.exists()) {
    setStatusMessage(QStringLiteral("Folder not found"));
    return false;
  }

  const fs::path src = fs::path(path.toStdString());
  if (!hasThemeLayout(src)) {
    setStatusMessage(
        QStringLiteral("Selected folder is not a valid cursor theme"));
    return false;
  }

  const QString name = srcDir.dirName();
  const bool ok = UltralightWebCursorM::UserConfig::instance()->uploadTheme(
      path.toStdString(), name.toStdString());

  if (!ok) {
    setStatusMessage(QStringLiteral("Upload failed"));
    return false;
  }

  loadThemes();
  setStatusMessage(QStringLiteral("Theme uploaded successfully"));
  return true;
}

void SettingsBackend::useTheme(const QString &name) {
  UltralightWebCursorM::UserConfig::instance()->setTheme(name.toStdString());
  reload();
  reconfigureSystem();
}

bool SettingsBackend::removeTheme(const QString &name) {
  if (name.isEmpty())
    return false;

  const bool ok = UltralightWebCursorM::UserConfig::instance()->removeTheme(
      name.toStdString());

  if (!ok) {
    setStatusMessage(QStringLiteral("Remove failed"));
    return false;
  }

  loadThemes();
  setStatusMessage(QStringLiteral("Theme removed successfully"));
  return true;
}

void SettingsBackend::openThemeFolder(const QString &name) {
  const fs::path p = g_sdkInitialPath / name.toStdString();
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(QString::fromStdString(p.string())));
}

QVariantMap SettingsBackend::getThemeDetails(const QString &name) {
  QVariantMap details;
  details[QStringLiteral("iconPath")] = QString();
  details[QStringLiteral("author")] = QStringLiteral("Unknown");
  details[QStringLiteral("describe")] = QString();
  details[QStringLiteral("minWidth")] = 128;
  details[QStringLiteral("minHeight")] = 128;

  if (name.isEmpty())
    return details;

  const fs::path themePath = g_sdkInitialPath / name.toStdString();
  if (!UltralightWebCursorM::CursorJSON::instance()->load(themePath.string()))
    return details;

  const auto values = UltralightWebCursorM::CursorJSON::instance()->values;
  const QString rawIconPath = QString::fromStdString(values.IconPath);
  QString fullIconUrl;

  if (!rawIconPath.isEmpty()) {
    const QFileInfo info(rawIconPath);
    const QString absolute = info.isAbsolute()
                                 ? rawIconPath
                                 : QString::fromStdString(themePath.string()) +
                                       QLatin1Char('/') + rawIconPath;
    fullIconUrl = QUrl::fromLocalFile(absolute).toString();
  }

  details[QStringLiteral("iconPath")] = fullIconUrl;
  details[QStringLiteral("author")] = QString::fromStdString(values.Author);
  details[QStringLiteral("describe")] = QString::fromStdString(values.describe);
  details[QStringLiteral("minWidth")] = values.minWidth;
  details[QStringLiteral("minHeight")] = values.minHeight;
  return details;
}

bool SettingsBackend::pathExists(const QString &path) const {
  return QFileInfo::exists(path);
}

void SettingsBackend::enable() {
  setEnabled(true);
  save();
  // Bring the engine up if it is not running, then enable it.
  ensureMainProcessRunning();
  notifyMainProcess(QStringLiteral("enable"));
  setStatusMessage(QStringLiteral("Enabled"));
}

void SettingsBackend::disable() {
  setEnabled(false);
  save();
  notifyMainProcess(QStringLiteral("disable"));
  setStatusMessage(QStringLiteral("Disabled"));
}

void SettingsBackend::reconfigureSystem() {
  notifyMainProcess(QStringLiteral("reload"));
}

void SettingsBackend::quit() { notifyMainProcess(QStringLiteral("quit")); }

QString SettingsBackend::engineExecutablePath() const {
#if defined(BUILD_TYPE_WINDOWS)
  return QCoreApplication::applicationDirPath() +
         QStringLiteral("/ultralightwebcursor_windows.exe");
#elif defined(BUILD_TYPE_X11)
  return QCoreApplication::applicationDirPath() +
         QStringLiteral("/ultralightwebcursor_x11");
#else
  return QString();
#endif
}

void SettingsBackend::ensureMainProcessRunning() {
#if defined(BUILD_TYPE_KWIN)
  // The KWin variant runs inside the compositor; there is nothing to spawn.
#else
  ensureConnected();
  if (ipcSocket_.state() == QLocalSocket::ConnectedState)
    return;

  const QString executable = engineExecutablePath();
  if (executable.isEmpty() || !QFileInfo::exists(executable)) {
    setStatusMessage(QStringLiteral("Cursor engine executable not found"));
    return;
  }

  if (!QProcess::startDetached(executable, {QStringLiteral("--silent")})) {
    setStatusMessage(QStringLiteral("Could not start the cursor engine"));
    return;
  }

  // Wait for the engine to create its IPC server, then reconnect.
  for (int attempt = 0;
       attempt < 10 && ipcSocket_.state() != QLocalSocket::ConnectedState;
       ++attempt) {
    ensureConnected();
  }
#endif
}

void SettingsBackend::uninstall() {
#if defined(BUILD_TYPE_WINDOWS)
  const QString uninstaller =
      QDir(QCoreApplication::applicationDirPath())
          .absoluteFilePath(QStringLiteral("../uninstall.exe"));
  if (!QFileInfo::exists(uninstaller)) {
    setStatusMessage(QStringLiteral("Uninstaller not found"));
    return;
  }
  QProcess::startDetached(uninstaller, {});
  setStatusMessage(QStringLiteral("Uninstalling..."));
  QCoreApplication::quit();
#elif defined(BUILD_TYPE_KWIN) || defined(BUILD_TYPE_X11)
  const QDir executableDir(QCoreApplication::applicationDirPath());
  const QString prefix =
      QDir::cleanPath(executableDir.absoluteFilePath(QStringLiteral("..")));
  const bool supported = prefix == QStringLiteral("/usr") ||
                         prefix == QStringLiteral("/usr/local") ||
                         prefix == QDir::homePath() + QStringLiteral("/.local");
  if (!supported) {
    setStatusMessage(
        QStringLiteral("Uninstall is only available for installed builds"));
    return;
  }

  // User-level files first (no privileges required).
  QFile::remove(
      QDir::homePath() +
      QStringLiteral("/.config/autostart/ultralightwebcursor.desktop"));
  QFile::remove(QDir::homePath() +
                QStringLiteral("/.local/share/applications/"
                               "org.ultralightwebcursor.desktop"));
  QDir(QDir::homePath() + QStringLiteral("/.local/share/ultralightwebcursor"))
      .removeRecursively();
  QDir(QDir::homePath() + QStringLiteral("/.config/ultralightwebcursor"))
      .removeRecursively();

  const QString script = QStringLiteral(
      "prefix=\"$1\"; "
      "pkill -f ultralightwebcursor_x11 >/dev/null 2>&1; "
      "rm -rf \"$prefix/bin/ultralightwebcursor-gui\" "
      "\"$prefix/bin/ultralightwebcursor_x11\" "
      "\"$prefix/bin/ultralightwebcursor-install\" "
      "\"$prefix/lib/ultralightwebcursor\" "
      "\"$prefix/lib64/ultralightwebcursor\" "
      "\"$prefix/share/ultralightwebcursor\" "
      "\"$prefix/share/kwin/effects/ultralightwebcursor\" "
      "\"$prefix/share/applications/org.ultralightwebcursor.desktop\" "
      "\"$prefix/share/icons/hicolor/scalable/apps/"
      "org.ultralightwebcursor.svg\"; "
      "rm -f "
      "\"$prefix\"/lib*/plugins/kwin/effects/plugins/ultralightwebcursor.so "
      "\"$prefix\"/lib*/kwin/effects/plugins/ultralightwebcursor.so");
  if (!QProcess::startDetached(QStringLiteral("pkexec"),
                               {QStringLiteral("sh"), QStringLiteral("-c"),
                                script, QStringLiteral("--"), prefix})) {
    setStatusMessage(QStringLiteral("Could not launch the uninstaller"));
    return;
  }
  setStatusMessage(
      QStringLiteral("Uninstall started (authorization required)"));
  QCoreApplication::quit();
#endif
}

void SettingsBackend::setAutostart(bool value) {
  autostart_ = value;
  Q_EMIT autostartChanged();
  notifyMainProcess(QStringLiteral("setAutostart"),
                    {{QStringLiteral("enabled"), value}});
}

void SettingsBackend::ensureConnected() {
  if (ipcSocket_.state() == QLocalSocket::ConnectedState)
    return;
  if (ipcSocket_.state() != QLocalSocket::UnconnectedState)
    ipcSocket_.abort();
  ipcSocket_.connectToServer(QLatin1String(kIpcSocketName));
  ipcSocket_.waitForConnected(kIpcConnectTimeoutMs);
}

void SettingsBackend::notifyMainProcess(const QString &command,
                                        const QVariantMap &payload) {
#if defined(BUILD_TYPE_KWIN)
  if (command == QStringLiteral("setAutostart") ||
      command == QStringLiteral("quit"))
    return;

  QString method;
  if (command == QStringLiteral("enable"))
    method = QStringLiteral("enable");
  else if (command == QStringLiteral("disable"))
    method = QStringLiteral("disable");
  else
    method = QStringLiteral("reloadHtml");

  QDBusInterface iface(QStringLiteral("org.kde.KWin"),
                       QStringLiteral("/UltralightCursor"),
                       QStringLiteral("org.kde.kwin.KWin.KwinCursorEffect"),
                       QDBusConnection::sessionBus());
  if (iface.isValid()) {
    iface.call(method);
  } else {
    setStatusMessage(QStringLiteral("KWin effect not reachable"));
  }
#else
  ensureConnected();
  if (ipcSocket_.state() != QLocalSocket::ConnectedState) {
    setStatusMessage(QStringLiteral("Main process not reachable"));
    return;
  }

  QJsonObject obj{
      {QStringLiteral("command"), command},
      {QStringLiteral("payload"), QJsonObject::fromVariantMap(payload)}};
  ipcSocket_.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  ipcSocket_.flush();
#endif
}
