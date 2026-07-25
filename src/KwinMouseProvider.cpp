#include "header/KwinMouseProvider.hpp"

#include "effect/effecthandler.h"
#include <QDBusConnection>
#include <iostream>


namespace KWin
{


KwinMouseProvider::KwinMouseProvider(QObject* parent) : QObject(parent){}

bool KwinMouseProvider::initialize(){
    connect(
        effects,
        &EffectsHandler::mouseChanged,
        this,
        &KwinMouseProvider::slotMouseChanged
    );
    //std::cout << "[KwinMouseProvider][debug] effects->mouseChanged\n";
    return true;
}
void KwinMouseProvider::setCallback(Callback callback){
    callback_ = std::move(callback);
}

void KwinMouseProvider::slotMouseChanged(
    const QPointF& pos,
    const QPointF&,
    Qt::MouseButtons buttons,
    Qt::MouseButtons,
    Qt::KeyboardModifiers,
    Qt::KeyboardModifiers
)
{
    if(!callback_) return;

    UltralightWebCursorM::MousePoint pt;

    pt.x = static_cast<int>(pos.x());
    pt.y = static_cast<int>(pos.y());
    /*
     qDebug()
    << "[KwinMouseProvider][debug] mouseChanged"
    << "x =" << pos.x()
    << "y =" << pos.y()
    << "left =" << buttons.testFlag(Qt::LeftButton);
    */
    pt.pressed = buttons.testFlag(Qt::LeftButton);

    callback_(pt);
}


}
