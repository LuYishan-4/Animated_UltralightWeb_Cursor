#pragma once

#include "MouseProvider.hpp"
#include <kwin/effect/effect.h>
#include <QObject>
#include <QPointF>


namespace KWin
{


class KwinMouseProvider :
    public QObject,
    public UltralightWebCursorM::IMouseProvider
{
    Q_OBJECT

public:

    explicit KwinMouseProvider(QObject* parent = nullptr);


    bool initialize() override;


    void setCallback(Callback callback) override;


private Q_SLOTS:

    void slotMouseChanged(
        const QPointF& pos,
        const QPointF& old,
        Qt::MouseButtons buttons,
        Qt::MouseButtons oldButtons,
        Qt::KeyboardModifiers modifiers,
        Qt::KeyboardModifiers oldModifiers
    );


private:

    Callback callback_;

};


}
