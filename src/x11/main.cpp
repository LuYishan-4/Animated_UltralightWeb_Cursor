#include <QGuiApplication>
#include <QTimer>
#include <QDebug>
#include "StandaloneCursorEffect.hpp"

int main(int argc, char** argv){
    QGuiApplication app(argc, argv);

    StandaloneCursorEffect effect;
    if(!effect.initialize()){
        qCritical() << "Failed to initialize Niri standalone effect";
        return 1;
    }
    effect.start();
    qDebug() << "Niri standalone cursor effect running";
    return app.exec();
}
