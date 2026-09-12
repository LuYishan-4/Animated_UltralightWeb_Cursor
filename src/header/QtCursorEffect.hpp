#pragma once

#include "CursorEffectCore.hpp"

#include <QBackingStore>
#include <QEvent>
#include <QObject>
#include <QTimer>
#include <QWindow>
#include <memory>

#if defined(__linux__) || defined(Q_OS_LINUX)
#undef Event
#undef Cursor
#undef Status
#undef Bool
#undef None
#undef Window
#undef Screen
#endif

namespace UltralightWebCursorM {

// Standalone (non-KWin) cursor effect body for X11 and Windows.
//
// KWin hosts the same rendering through KwinCursorEffect (a KWin::Effect
// plugin). Here we drive a transparent, click-through, always-on-top QWindow
// and repaint the HTML cursor into it each frame.
class QtCursorEffect : public QObject, public CursorEffectCore {
  Q_OBJECT

public:
  explicit QtCursorEffect(QObject *parent = nullptr);
  ~QtCursorEffect() override;

  bool initialize();
  void start();
  void renderWindow();

protected:
  bool event(QEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
  void onTick();

private:
  QTimer timer_;

  std::unique_ptr<QWindow> m_viewWindow;
  std::unique_ptr<QBackingStore> m_backingStore;
};

} // namespace UltralightWebCursorM
