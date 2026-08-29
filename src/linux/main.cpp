#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <filesystem>

namespace fs = std::filesystem;

namespace {

QString envValue(const char *name) {
  const QByteArray raw = qgetenv(name);
  return QString::fromLocal8Bit(raw);
}

QString sessionType() { return envValue("XDG_SESSION_TYPE"); }

QString currentDesktop() {
  const QString desktop = envValue("XDG_CURRENT_DESKTOP");
  if (!desktop.isEmpty())
    return desktop;
  return envValue("XDG_SESSION_DESKTOP");
}

QString variantFilePath() {
  return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
         QStringLiteral("/ultralightwebcursor/variant");
}

void saveVariant(const QString &variant) {
  const QFileInfo info(variantFilePath());
  QDir().mkpath(info.absolutePath());
  QFile file(info.absoluteFilePath());
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    file.write(variant.toUtf8());
    file.write("\n");
  }
}

QString engineCommand() {
  return QCoreApplication::applicationDirPath() + QDir::separator() +
         QStringLiteral("ultralightwebcursor_x11");
}

// Per-user data directory for the standalone (X11) engine.
QString dataDir() {
  return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
         QStringLiteral("/ultralightwebcursor");
}

bool isTheme(const fs::path &dir) {
  return fs::exists(dir / "CursorData.json") && fs::exists(dir / "index.html");
}

bool copyDir(const fs::path &src, const fs::path &dst) {
  std::error_code ec;
  fs::create_directories(dst, ec);
  if (ec)
    return false;

  fs::directory_iterator it(src, ec);
  const fs::directory_iterator end;
  while (!ec && it != end) {
    fs::copy(it->path(), dst / it->path().filename(),
             fs::copy_options::recursive | fs::copy_options::overwrite_existing,
             ec);
    it.increment(ec);
  }
  return !ec;
}

// Locate the source directory containing "resources" and built-in themes.
fs::path findSourceDataDir() {
  const QStringList candidates = {
      QCoreApplication::applicationDirPath(),
      QStringLiteral("/usr/share/ultralightwebcursor"),
      QStringLiteral("/usr/share/kwin/effects/ultralightwebcursor"),
  };
  for (const QString &candidate : candidates) {
    if (QDir(candidate + QStringLiteral("/resources")).exists())
      return fs::path(candidate.toStdString());
  }
  return fs::path();
}

// Populate ~/.local/share/ultralightwebcursor with resources + themes so the
// standalone engine can locate the Ultralight runtime data without root.
bool populateUserData() {
  const fs::path src = findSourceDataDir();
  if (src.empty())
    return false;

  const fs::path dst(dataDir().toStdString());
  std::error_code ec;
  fs::create_directories(dst, ec);

  if (fs::exists(src / "resources"))
    copyDir(src / "resources", dst / "resources");

  fs::directory_iterator it(src, ec);
  const fs::directory_iterator end;
  while (!ec && it != end) {
    const fs::path entry = it->path();
    if (entry.filename() != "resources" && fs::is_directory(entry) &&
        isTheme(entry)) {
      copyDir(entry, dst / entry.filename());
    }
    it.increment(ec);
  }
  return true;
}

// Write the settings GUI into the application menu (user level, no root).
bool createGuiDesktopEntry() {
  const QString appsDir =
      QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
  QDir().mkpath(appsDir);

  const QString path =
      appsDir + QStringLiteral("/org.ultralightwebcursor.desktop");
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    return false;

  file.write("[Desktop Entry]\n");
  file.write("Type=Application\n");
  file.write("Name=Ultralight Web Cursor\n");
  file.write("Comment=Configure the animated HTML/CSS cursor\n");
  file.write("Exec=ultralightwebcursor-gui\n");
  file.write("Terminal=false\n");
  file.write("Categories=Settings;Qt;Graphics;\n");
  file.close();
  return true;
}

// X11 variant: populate the user data dir, write an XDG autostart entry, and
// register the GUI in the application menu.
bool setupX11() {
  populateUserData();

  const QString dir =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/autostart");
  QDir().mkpath(dir);

  const QString path = dir + QStringLiteral("/ultralightwebcursor.desktop");
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    return false;

  file.write("[Desktop Entry]\n");
  file.write("Type=Application\n");
  file.write("Name=Ultralight Web Cursor\n");
  file.write("Comment=Animated HTML/CSS cursor\n");
  file.write("Exec=\"");
  file.write(engineCommand().toUtf8());
  file.write("\" --silent\n");
  file.write("Terminal=false\n");
  file.write("X-GNOME-Autostart-enabled=true\n");
  file.close();

  createGuiDesktopEntry();
  saveVariant(QStringLiteral("x11"));
  return true;
}

// KWin variant: ask KWin to load the effect and register the GUI in the
// application menu. The effect reads themes/resources from its own system dir.
bool setupKwin() {
  QProcess proc;
  proc.start(QStringLiteral("busctl"),
             {QStringLiteral("--user"), QStringLiteral("call"),
              QStringLiteral("org.kde.KWin"), QStringLiteral("/Effects"),
              QStringLiteral("org.kde.kwin.Effects"),
              QStringLiteral("loadEffect"), QStringLiteral("s"),
              QStringLiteral("ultralightwebcursor")});
  proc.waitForFinished(5000);

  if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0)
    return false;

  createGuiDesktopEntry();
  saveVariant(QStringLiteral("kwin"));
  return true;
}

} // namespace

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName(QStringLiteral("ultralightwebcursor"));
  QCoreApplication::setOrganizationName(QStringLiteral("UltralightWebCursor"));

  QTextStream out(stdout);
  QTextStream in(stdin);

  const QString session = sessionType();
  const QString desktop = currentDesktop();

  const bool wayland =
      session.compare(QStringLiteral("wayland"), Qt::CaseInsensitive) == 0;
  const bool kde = desktop.contains(QStringLiteral("KDE"), Qt::CaseInsensitive);
  const QString recommend =
      (wayland || kde) ? QStringLiteral("kwin") : QStringLiteral("x11");

  out << "Ultralight Web Cursor - Linux setup\n";
  out << "-----------------------------------\n";
  out << "Session: "
      << (session.isEmpty() ? QStringLiteral("unknown") : session)
      << " | Desktop: "
      << (desktop.isEmpty() ? QStringLiteral("unknown") : desktop) << "\n\n";
  out << "Choose the version to use:\n";
  out << "  [1] KWin  (Wayland / KDE Plasma)\n";
  out << "  [2] X11   (standalone overlay)\n";
  out << "Selection (default "
      << (recommend == QStringLiteral("kwin") ? QStringLiteral("1")
                                              : QStringLiteral("2"))
      << "): ";
  out.flush();

  QString line = in.readLine().trimmed();
  if (line.isEmpty())
    line = (recommend == QStringLiteral("kwin") ? QStringLiteral("1")
                                                : QStringLiteral("2"));

  if (line == QStringLiteral("1")) {
    if (setupKwin()) {
      out << "KWin effect loaded. A re-login may be required.\n";
    } else {
      out << "Could not load the KWin effect via D-Bus.\n";
      out << "Make sure the plugin is installed and KWin is running.\n";
    }
  } else {
    if (setupX11()) {
      out << "X11 standalone version ready and registered to start on login.\n";
    } else {
      out << "Failed to register X11 autostart.\n";
    }
  }

  return 0;
}
