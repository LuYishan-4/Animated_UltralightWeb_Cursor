#pragma once

#include <core/output.h>
#include <kwin/effect/effect.h>
#include "MainCursorStaff.hpp"

namespace KWin {

class LogicalOutput;
class GLTexture;
class EffectWindow; 

class KwinCursorEffect : 
    public Effect, 
    public UltralightWebCursorM::MainCursorStaff
{
    Q_OBJECT
    Q_DISABLE_COPY(KwinCursorEffect)

public:
    KwinCursorEffect();
    ~KwinCursorEffect() override;

    void paintScreen(
        const RenderTarget& renderTarget,
        const RenderViewport& viewport,
        int mask,
        const Region& region,
        LogicalOutput* screen
    ) override;

    bool isActive() const override;

    int requestedEffectChainPosition() const override { return 99; }

    static bool supported();
public Q_SLOTS:
    void enable() override;
    void disable() override;
    void reloadHtml() override;

private:
    bool checkFullScreen() const override;
    bool isBlacklisted() const;
    GLTexture* ensureCursorTexture();
    void slotWindowStateChanged(EffectWindow *w);

    std::unique_ptr<GLTexture> m_cursorTexture;
};

}
