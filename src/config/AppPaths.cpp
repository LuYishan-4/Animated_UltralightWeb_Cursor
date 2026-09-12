#include "AppPaths.hpp"

#include "BuildConfig.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

namespace UltralightWebCursor {
namespace {

constexpr auto kBuiltInMarker = ".uwc-built-in";

void setError(QString *destination, const QString &message) {
  if (destination)
    *destination = message;
}

bool copyFile(const QString &source, const QString &destination,
              QString *errorMessage) {
  QDir().mkpath(QFileInfo(destination).absolutePath());
  QFile::remove(destination);
  if (QFile::copy(source, destination))
    return true;

  setError(errorMessage,
           QStringLiteral("Could not copy %1 to %2").arg(source, destination));
  return false;
}

bool copyDirectory(const QString &source, const QString &destination,
                   QString *errorMessage) {
  const QDir sourceDir(source);
  if (!sourceDir.exists()) {
    setError(errorMessage,
             QStringLiteral("Data directory does not exist: %1").arg(source));
    return false;
  }

  if (!QDir().mkpath(destination)) {
    setError(errorMessage,
             QStringLiteral("Could not create directory: %1").arg(destination));
    return false;
  }

  QDirIterator iterator(source, QDir::NoDotAndDotDot | QDir::AllEntries,
                        QDirIterator::Subdirectories);
  while (iterator.hasNext()) {
    const QString sourcePath = iterator.next();
    const QFileInfo info(sourcePath);
    const QString relativePath = sourceDir.relativeFilePath(sourcePath);
    const QString destinationPath = QDir(destination).filePath(relativePath);

    if (info.isDir()) {
      if (!QDir().mkpath(destinationPath)) {
        setError(errorMessage, QStringLiteral("Could not create directory: %1")
                                   .arg(destinationPath));
        return false;
      }
    } else if (info.isFile() &&
               !copyFile(sourcePath, destinationPath, errorMessage)) {
      return false;
    }
  }
  return true;
}

bool hasRequiredResources(const QString &root) {
  const QDir resources(QDir(root).filePath(QStringLiteral("resources")));
  return resources.exists(QStringLiteral("cacert.pem")) &&
         resources.exists(QStringLiteral("icudt67l.dat"));
}

QString firstValidDataRoot(const QStringList &candidates) {
  for (const QString &candidate : candidates) {
    if (!candidate.isEmpty() && hasRequiredResources(candidate))
      return QDir::cleanPath(candidate);
  }
  return {};
}

} // namespace

QString AppPaths::configFile() {
  return QDir(QStandardPaths::writableLocation(
                  QStandardPaths::GenericConfigLocation))
      .filePath(QStringLiteral("ultralightwebcursor/config.ini"));
}

QString AppPaths::userDataDir() {
  return QDir(QStandardPaths::writableLocation(
                  QStandardPaths::GenericDataLocation))
      .filePath(QStringLiteral("ultralightwebcursor"));
}

QString AppPaths::bundledDataDir() {
  const QString executableDir = QCoreApplication::applicationDirPath();
  const QString configured =
      QString::fromLocal8Bit(qgetenv("ULTRALIGHTWEBCURSOR_DATA_DIR"));
  return firstValidDataRoot(
      {configured, executableDir,
       QDir(executableDir).filePath(QStringLiteral("data")),
       QString::fromUtf8(BuildConfig::installDataDir),
       QStringLiteral("/usr/share/ultralightwebcursor"),
       QStringLiteral("/usr/local/share/ultralightwebcursor")});
}

QString AppPaths::engineExecutablePath() {
  QString executable = QString::fromUtf8(BuildConfig::engineExecutable);
#ifdef Q_OS_WIN
  executable += QStringLiteral(".exe");
#endif
  return QDir(QCoreApplication::applicationDirPath()).filePath(executable);
}

bool AppPaths::provisionUserData(QString *errorMessage) {
  const QString destinationRoot = userDataDir();
  const QString sourceRoot = bundledDataDir();

  if (!QDir().mkpath(destinationRoot)) {
    setError(errorMessage,
             QStringLiteral("Could not create user data directory: %1")
                 .arg(destinationRoot));
    return false;
  }

  // Remove per-user files created by the retired variant selector. The current
  // packaged launcher lives in the system application directory.
  QFile::remove(QDir(QFileInfo(configFile()).absolutePath())
                    .filePath(QStringLiteral("variant")));
  const QString applications =
      QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
  if (!applications.isEmpty()) {
    QFile::remove(
        QDir(applications)
            .filePath(QStringLiteral("org.ultralightwebcursor.desktop")));
  }

  const QString versionMarker =
      QDir(destinationRoot).filePath(QStringLiteral(".provisioned-version"));
  QFile markerReader(versionMarker);
  if (hasRequiredResources(destinationRoot) &&
      markerReader.open(QIODevice::ReadOnly | QIODevice::Text) &&
      QString::fromUtf8(markerReader.readAll()).trimmed() ==
          QString::fromUtf8(BuildConfig::dataRevision)) {
    return true;
  }

  if (sourceRoot.isEmpty()) {
    if (hasRequiredResources(destinationRoot))
      return true;
    setError(errorMessage,
             QStringLiteral("Bundled Ultralight resources were not found"));
    return false;
  }

  if (!copyDirectory(
          QDir(sourceRoot).filePath(QStringLiteral("resources")),
          QDir(destinationRoot).filePath(QStringLiteral("resources")),
          errorMessage)) {
    return false;
  }

  const QDir source(sourceRoot);
  const QStringList entries =
      source.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QString &name : entries) {
    const QString sourceTheme = source.filePath(name);
    if (!isThemeDirectory(sourceTheme))
      continue;

    const QString destinationTheme = QDir(destinationRoot).filePath(name);
    const QString marker =
        QDir(destinationTheme).filePath(QString::fromLatin1(kBuiltInMarker));

    // Bundled theme names are managed by the application. Synchronizing them
    // keeps metadata and assets current after package upgrades.
    if (!copyDirectory(sourceTheme, destinationTheme, errorMessage))
      return false;

    QSaveFile markerFile(marker);
    if (markerFile.open(QIODevice::WriteOnly)) {
      markerFile.write("managed by Ultralight Web Cursor\n");
      markerFile.commit();
    }
  }

  // Remove bundled themes that disappeared from the package. Marker-managed
  // directories are always safe to delete. variant9-gun is a pre-marker
  // bundled theme removed in 1.1 because it was incompatible with the overlay.
  const QDir destination(destinationRoot);
  for (const QString &name :
       destination.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
    const QString destinationTheme = destination.filePath(name);
    const bool managed = QFileInfo::exists(
        QDir(destinationTheme).filePath(QString::fromLatin1(kBuiltInMarker)));
    const bool obsoleteGunTheme =
        name == QStringLiteral("variant9-gun") &&
        QFileInfo::exists(
            QDir(destinationTheme).filePath(QStringLiteral("assets/gun.gif")));
    if ((managed && !isThemeDirectory(source.filePath(name))) ||
        obsoleteGunTheme) {
      QDir(destinationTheme).removeRecursively();
    }
  }

  if (!hasRequiredResources(destinationRoot))
    return false;

  QSaveFile versionFile(versionMarker);
  if (versionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    versionFile.write(QByteArray(BuildConfig::dataRevision) + '\n');
    versionFile.commit();
  }
  return true;
}

bool AppPaths::isThemeDirectory(const QString &path) {
  const QDir directory(path);
  return directory.exists(QStringLiteral("CursorData.json")) &&
         directory.exists(QStringLiteral("index.html"));
}

bool AppPaths::isBuiltInTheme(const QString &name) {
  if (name.isEmpty() || name.contains(QLatin1Char('/')) ||
      name.contains(QLatin1Char('\\')))
    return false;
  return QFileInfo::exists(QDir(userDataDir())
                               .filePath(name + QLatin1Char('/') +
                                         QString::fromLatin1(kBuiltInMarker)));
}

QStringList AppPaths::builtInThemes() {
  QStringList result;
  const QDir data(userDataDir());
  const QStringList entries =
      data.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QString &name : entries) {
    if (isBuiltInTheme(name))
      result.append(name);
  }
  return result;
}

} // namespace UltralightWebCursor
