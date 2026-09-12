#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QVariant>

#include "BuildConfig.hpp"
#include "SettingsBackend.hpp"

int main(int argc, char **argv) {
  QGuiApplication app(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("UltralightWebCursor"));
  QCoreApplication::setApplicationName(
      QString::fromUtf8(UltralightWebCursor::BuildConfig::applicationName));
  QCoreApplication::setApplicationVersion(
      QString::fromUtf8(UltralightWebCursor::BuildConfig::version));
  QGuiApplication::setDesktopFileName(
      QString::fromUtf8(UltralightWebCursor::BuildConfig::applicationId));
  QGuiApplication::setWindowIcon(
      QIcon(QStringLiteral(":/qt/qml/WebCursorGui/icons/"
                           "io.github.luyishan4.ultralightwebcursor.svg")));

  QQmlApplicationEngine engine;
  SettingsBackend backend;
  engine.setInitialProperties(
      {{QStringLiteral("backend"), QVariant::fromValue(&backend)}});
  engine.loadFromModule(QStringLiteral("WebCursorGui"), QStringLiteral("Main"));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
