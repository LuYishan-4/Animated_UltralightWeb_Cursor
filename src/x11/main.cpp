#include <QGuiApplication>
#include <QTimer>
#include <QDebug>
#include "../header/QtCursorEffect.hpp"

int main(int argc, char** argv){
    QGuiApplication app(argc, argv);

    QtCursorEffect effect;
    if(!effect.initialize()){
        qCritical() << "Failed to initialize Niri standalone effect";
        return 1;
    }
    effect.start();
    qDebug() << "Niri standalone cursor effect running";
    return app.exec();
}
