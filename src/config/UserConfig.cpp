#include "UserConfig.hpp"

#include "AppPaths.hpp"
#include "BuildConfig.hpp"
#include "ThemeMetadata.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>
#include <QUuid>

#include <algorithm>

namespace UltralightWebCursor {
namespace {

void setError(QString *destination, const QString &message) {
  if (destination)
    *destination = message;
}

int boundedInteger(const QString &value, int fallback, int minimum,
                   int maximum) {
  bool ok = false;
  const int parsed = value.toInt(&ok);
  return ok ? std::clamp(parsed, minimum, maximum) : fallback;
}

bool copyDirectory(const QString &source, const QString &destination,
                   QString *errorMessage) {
  const QDir sourceDir(source);
  if (!sourceDir.exists() || !QDir().mkpath(destination)) {
    setError(errorMessage, QStringLiteral("Could not create theme directory"));
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
        setError(errorMessage,
                 QStringLiteral("Could not create %1").arg(destinationPath));
        return false;
      }
      continue;
    }

    if (!info.isFile())
      continue;

    QDir().mkpath(QFileInfo(destinationPath).absolutePath());
    if (!QFile::copy(sourcePath, destinationPath)) {
      setError(errorMessage,
               QStringLiteral("Could not copy %1").arg(info.fileName()));
      return false;
    }
  }
  return true;
}

QMap<QString, QString> readConfigFile(const QString &fileName) {
  QMap<QString, QString> values;
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return values;

  while (!file.atEnd()) {
    const QString line = QString::fromUtf8(file.readLine()).trimmed();
    if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
      continue;
    const qsizetype separator = line.indexOf(QLatin1Char('='));
    if (separator <= 0)
      continue;
    values.insert(line.left(separator).trimmed(),
                  line.mid(separator + 1).trimmed());
  }
  return values;
}

} // namespace

UserConfig &UserConfig::instance() {
  static UserConfig config;
  return config;
}

void UserConfig::applyDefaults() {
  values_.version = QString::fromUtf8(BuildConfig::version);
  values_.dataRoot = AppPaths::userDataDir();
  values_.width = 128;
  values_.height = 128;
  values_.enabled = true;
  values_.htmlPath = QDir(values_.dataRoot)
                         .filePath(QStringLiteral("variant1-neon/index.html"));
}

bool UserConfig::selectUsableTheme(const QString &preferredTheme) {
  const QDir data(values_.dataRoot);
  QString theme = preferredTheme;
  if (theme.isEmpty() || !AppPaths::isThemeDirectory(data.filePath(theme))) {
    const QStringList candidates =
        data.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    theme.clear();
    for (const QString &candidate : candidates) {
      if (AppPaths::isThemeDirectory(data.filePath(candidate))) {
        theme = candidate;
        break;
      }
    }
  }

  if (theme.isEmpty())
    return false;

  values_.htmlPath = data.filePath(theme + QStringLiteral("/index.html"));
  return true;
}

bool UserConfig::load(QString *errorMessage) {
  applyDefaults();

  QString provisioningError;
  const bool provisioned = AppPaths::provisionUserData(&provisioningError);
  QMap<QString, QString> stored = readConfigFile(AppPaths::configFile());
#ifdef Q_OS_WIN
  if (stored.isEmpty()) {
    QStringList legacyFiles;
    const QString roamingData = QString::fromLocal8Bit(qgetenv("APPDATA"));
    if (!roamingData.isEmpty()) {
      legacyFiles.append(
          QDir(roamingData)
              .filePath(QStringLiteral("UltralightWebCursor/config.ini")));
    }
    legacyFiles.append(
        QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
            .filePath(QStringLiteral("ultralightwebcursor/config.ini")));
    for (const QString &legacyFile : legacyFiles) {
      if (QDir::cleanPath(legacyFile) ==
          QDir::cleanPath(AppPaths::configFile()))
        continue;
      stored = readConfigFile(legacyFile);
      if (!stored.isEmpty())
        break;
    }
  }
#endif

  values_.width =
      boundedInteger(stored.value(QStringLiteral("width")), 128, 16, 4096);
  values_.height =
      boundedInteger(stored.value(QStringLiteral("height")), 128, 16, 4096);
  if (stored.contains(QStringLiteral("enabled")))
    values_.enabled =
        stored.value(QStringLiteral("enabled")) == QStringLiteral("true");

  // Migrate legacy absolute paths by preserving only the selected theme name
  // and rebasing it into the writable user data directory.
  const QString oldHtml = stored.value(QStringLiteral("html"));
  const QString preferredTheme = oldHtml.isEmpty()
                                     ? QStringLiteral("variant1-neon")
                                     : QFileInfo(oldHtml).dir().dirName();
  const bool hasTheme = selectUsableTheme(preferredTheme);

  if (!provisioned || !hasTheme) {
    setError(errorMessage,
             !provisioningError.isEmpty()
                 ? provisioningError
                 : QStringLiteral("No valid cursor theme is available"));
    return false;
  }

  return save(errorMessage);
}

bool UserConfig::save(QString *errorMessage) const {
  const QString fileName = AppPaths::configFile();
  if (!QDir().mkpath(QFileInfo(fileName).absolutePath())) {
    setError(errorMessage, QStringLiteral("Could not create config directory"));
    return false;
  }

  QSaveFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    setError(errorMessage, QStringLiteral("Could not open config for writing"));
    return false;
  }

  const QByteArray content =
      QByteArray("configver=") + values_.version.toUtf8() + '\n' +
      "html=" + values_.htmlPath.toUtf8() + '\n' +
      "sdk=" + values_.dataRoot.toUtf8() + '\n' +
      "width=" + QByteArray::number(values_.width) + '\n' +
      "height=" + QByteArray::number(values_.height) + '\n' + "enabled=" +
      (values_.enabled ? QByteArray("true") : QByteArray("false")) + '\n';
  if (file.write(content) != content.size() || !file.commit()) {
    setError(errorMessage, QStringLiteral("Could not save configuration"));
    return false;
  }
  return true;
}

bool UserConfig::setEnabled(bool enabled, QString *errorMessage) {
  values_.enabled = enabled;
  return save(errorMessage);
}

bool UserConfig::setSize(int width, int height, QString *errorMessage) {
  values_.width = std::clamp(width, 16, 4096);
  values_.height = std::clamp(height, 16, 4096);
  return save(errorMessage);
}

bool UserConfig::setTheme(const QString &themeName, QString *errorMessage) {
  if (themeName.isEmpty() || themeName == QStringLiteral(".") ||
      themeName == QStringLiteral("..") ||
      themeName.contains(QLatin1Char('/')) ||
      themeName.contains(QLatin1Char('\\'))) {
    setError(errorMessage, QStringLiteral("Invalid theme name"));
    return false;
  }

  const QString themeDirectory = QDir(values_.dataRoot).filePath(themeName);
  if (!AppPaths::isThemeDirectory(themeDirectory)) {
    setError(errorMessage, QStringLiteral("Theme is incomplete or missing"));
    return false;
  }

  values_.htmlPath =
      QDir(themeDirectory).filePath(QStringLiteral("index.html"));
  return save(errorMessage);
}

bool UserConfig::importTheme(const QString &sourceDirectory,
                             QString *importedThemeName,
                             QString *errorMessage) {
  const QFileInfo sourceInfo(sourceDirectory);
  const QString canonicalSource = sourceInfo.canonicalFilePath();
  const QString themeName = sourceInfo.fileName().trimmed();
  if (canonicalSource.isEmpty() || !sourceInfo.isDir() ||
      !AppPaths::isThemeDirectory(canonicalSource)) {
    setError(errorMessage,
             QStringLiteral("Select a folder containing index.html and "
                            "CursorData.json"));
    return false;
  }
  if (themeName.isEmpty() || themeName == QStringLiteral(".") ||
      themeName == QStringLiteral("..") ||
      themeName.contains(QLatin1Char('/')) ||
      themeName.contains(QLatin1Char('\\'))) {
    setError(errorMessage, QStringLiteral("Invalid theme folder name"));
    return false;
  }
  if (AppPaths::isBuiltInTheme(themeName)) {
    setError(errorMessage,
             QStringLiteral("A built-in theme already uses that name"));
    return false;
  }

  QDir data(values_.dataRoot);
  const QString destination = data.filePath(themeName);
  if (QFileInfo(destination).canonicalFilePath() == canonicalSource) {
    setError(errorMessage, QStringLiteral("Theme is already installed"));
    return false;
  }

  const QString transactionId =
      QUuid::createUuid().toString(QUuid::WithoutBraces);
  const QString temporaryName = QStringLiteral(".import-%1").arg(transactionId);
  const QString backupName = QStringLiteral(".backup-%1").arg(transactionId);
  const QString temporary = data.filePath(temporaryName);
  const QString backup = data.filePath(backupName);

  QDir(temporary).removeRecursively();
  if (!copyDirectory(canonicalSource, temporary, errorMessage) ||
      !AppPaths::isThemeDirectory(temporary)) {
    QDir(temporary).removeRecursively();
    return false;
  }

  const bool replacing = QFileInfo::exists(destination);
  if (replacing && !data.rename(themeName, backupName)) {
    QDir(temporary).removeRecursively();
    setError(errorMessage, QStringLiteral("Could not replace existing theme"));
    return false;
  }

  if (!data.rename(temporaryName, themeName)) {
    if (replacing)
      data.rename(backupName, themeName);
    QDir(temporary).removeRecursively();
    setError(errorMessage, QStringLiteral("Could not finish theme import"));
    return false;
  }

  if (replacing)
    QDir(backup).removeRecursively();
  if (importedThemeName)
    *importedThemeName = themeName;
  return true;
}

bool UserConfig::removeTheme(const QString &themeName, QString *errorMessage) {
  if (themeName.isEmpty() || themeName == QStringLiteral(".") ||
      themeName == QStringLiteral("..") ||
      themeName.contains(QLatin1Char('/')) ||
      themeName.contains(QLatin1Char('\\'))) {
    setError(errorMessage, QStringLiteral("Invalid theme name"));
    return false;
  }
  if (AppPaths::isBuiltInTheme(themeName)) {
    setError(errorMessage, QStringLiteral("Built-in themes cannot be removed"));
    return false;
  }
  if (themeName == currentTheme()) {
    setError(errorMessage,
             QStringLiteral("Select another theme before removing it"));
    return false;
  }

  const QString path = QDir(values_.dataRoot).filePath(themeName);
  if (!AppPaths::isThemeDirectory(path) || !QDir(path).removeRecursively()) {
    setError(errorMessage, QStringLiteral("Could not remove the theme"));
    return false;
  }
  return true;
}

QString UserConfig::currentTheme() const {
  return QFileInfo(values_.htmlPath).dir().dirName();
}

} // namespace UltralightWebCursor
