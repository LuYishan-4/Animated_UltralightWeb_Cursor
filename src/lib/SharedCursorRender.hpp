#pragma once

#include <QPointF>

namespace UltralightWebCursorM {

// Minimal render-state bundle shared by the standalone Qt-based effect.
//
// The KWin plugin performs its own compositor-level painting, so this is only
// consumed by QtCursorEffect on X11/Windows. `visible` already folds in the
// idle-hidden and enabled flags; additional filtering (e.g. fullscreen or
// blacklist suppression) can be added here without touching the renderer.
struct CursorRenderState {
  QPointF pos;
  QPointF hotspot;
  bool visible = true;
};

inline bool shouldRenderCursor(const CursorRenderState &state) {
  return state.visible;
}

} // namespace UltralightWebCursorM
