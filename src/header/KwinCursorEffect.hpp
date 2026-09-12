#pragma once

#include "CursorEffectCore.hpp"
#include <QTimer>
#include <array>
#include <core/output.h>
#include <kwin/effect/effect.h>

namespace KWin {

class LogicalOutput;
class GLTexture;
class EffectWindow;

class KwinCursorEffect : public Effect,
                         public UltralightWebCursorM::CursorEffectCore {
  Q_OBJECT
  Q_DISABLE_COPY(KwinCursorEffect)

public:
  KwinCursorEffect();
  ~KwinCursorEffect() override;

  void paintScreen(const RenderTarget &renderTarget,
                   const RenderViewport &viewport, int mask,
                   const Region &region, LogicalOutput *screen) override;

  bool isActive() const override;
  void reconfigure(ReconfigureFlags flags) override;

  int requestedEffectChainPosition() const override { return 99; }

  static bool supported();
public Q_SLOTS:
  void enable();
  void disable();
  void reloadHtml();

private:
  unsigned int m_lastGpuTexId = 0;
  bool checkFullScreen() const override;
  bool isBlacklisted() const;
  GLTexture *ensureCursorTexture();
  void slotWindowStateChanged(EffectWindow *w);
  void startIdleTimer();
  void slotIdleTimeout();
  std::unique_ptr<GLTexture> m_cursorTexture;
  QTimer m_idleTimer;
  bool m_autoHidden = false;
  bool m_firstFocusDone = false;
};

} // namespace KWin
