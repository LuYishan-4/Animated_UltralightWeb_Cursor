#pragma once

#include <core/output.h>
#include <kwin/effect/effect.h>
#include "MainCursorStaff.hpp"
#include <QTimer> 
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
    void enable();
    void disable();
    void reloadHtml();

private:
    bool checkFullScreen() const override;
    bool isBlacklisted() const;
    GLTexture* ensureCursorTexture();
    void slotWindowStateChanged(EffectWindow *w);
     //QTimer* m_renderTimer = nullptr; 
    std::unique_ptr<GLTexture> m_cursorTexture;
};

}
