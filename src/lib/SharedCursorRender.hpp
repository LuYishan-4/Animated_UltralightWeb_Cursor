#pragma once

#include <QPointF>
#include <QImage>
#include <QString>
#include <memory>

namespace UltralightWebCursorM
{

struct CursorRenderState
{
    QPointF hotspot{0.0, 0.0};
    QPointF pos{0.0, 0.0};
    bool visible = true;
};

inline QPointF computeCursorHotspot(int width, int height)
{
    return QPointF(width / 2.0, height / 2.0);
}

inline bool shouldRenderCursor(const CursorRenderState& state)
{
    return state.visible;
}

}
