#include "PluginPath.hpp"
#include "GlobalConstants.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

namespace UltralightWebCursorM {

std::filesystem::path PluginPath::dataDir() {
  QString path;
  switch (GlobalConstants::buildType) {
  case BuildType::Windows: {
    // Portable first: resources + themes next to the executable.
    const QString exeDir = QCoreApplication::applicationDirPath();
    if (QDir(exeDir + QStringLiteral("/resources")).exists()) {
      path = exeDir;
    } else {
      path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
      if (path.isEmpty()) {
        path = exeDir + QStringLiteral("/data");
      }
    }
    break;
  }
  case BuildType::Kwin: {
    // KWin keeps its original system data path.
    path = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        QStringLiteral("kwin/effects/ultralightwebcursor"),
        QStandardPaths::LocateDirectory);
    break;
  }
  case BuildType::X11:
  default: {
    // Portable first: resources + themes next to the executable.
    const QString exeDir = QCoreApplication::applicationDirPath();
    if (QDir(exeDir + QStringLiteral("/resources")).exists()) {
      path = exeDir;
    } else {
      path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                    QStringLiteral("ultralightwebcursor"),
                                    QStandardPaths::LocateDirectory);
      if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(
                   QStandardPaths::GenericDataLocation) +
               QStringLiteral("/ultralightwebcursor");
      }
    }
    break;
  }
  }

  return std::filesystem::path(path.toStdString());
}

} // namespace UltralightWebCursorM
