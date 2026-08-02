#include "PluginPath.hpp"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include "GlobalConstas.hpp"

namespace UltralightWebCursorM{

std::filesystem::path PluginPath::dataDir() {
    QString path;
    switch (GloablContast::buildType) {
        case BuildType::Windows: {
            path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            if (path.isEmpty()) {
                path = QCoreApplication::applicationDirPath() + "/data";
            }
            break;
        }
        case BuildType::Kwin: {
            path = QStandardPaths::locate(
                QStandardPaths::GenericDataLocation,
                QStringLiteral("kwin/effects/ultralightwebcursor"),
                QStandardPaths::LocateDirectory
            );
            break;
        }
        case BuildType::X11:
        default: {
            path = QStandardPaths::locate(
                QStandardPaths::GenericDataLocation,
                QStringLiteral("ultralightwebcursor"),
                QStandardPaths::LocateDirectory
            );
            if (path.isEmpty()) {
                path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) 
                       + "/ultralightwebcursor";
            }
            break;
        }
    }

    return std::filesystem::path(path.toStdString());
}

} // namespace UltralightWebCursorM
