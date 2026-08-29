#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

#include "SettingsBackend.hpp"

int main(int argc, char **argv) {
  QGuiApplication app(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("UltralightWebCursor"));
  QCoreApplication::setApplicationName(QStringLiteral("ultralightwebcursor"));

  QQmlApplicationEngine engine;
  SettingsBackend backend;
  engine.rootContext()->setContextProperty(QStringLiteral("appBackend"),
                                           &backend);

  engine.loadFromModule(QStringLiteral("WebCursorGui"), QStringLiteral("Main"));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
