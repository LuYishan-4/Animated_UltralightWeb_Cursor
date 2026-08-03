#pragma once

#include <core/output.h>
#include <kwin/effect/effect.h>
#include "MainCursorStaff.hpp"

namespace KWin::UltralightWebCursorM{

class LogicalOutput;
class GLTexture;
class EffectWindow; 
class UltralightCursorEffect : 
    public Effect, 
    public MainCursorStaff
{
    Q_OBJECT
    Q_DISABLE_COPY(UltralightCursorEffect)

public:
    UltralightCursorEffect();
    ~UltralightCursorEffect() override;

    void paintScreen(
        const RenderTarget& renderTarget,
        const RenderViewport& viewport,
        int mask,
        const Region& region,
        LogicalOutput* screen
    ) override;

    bool isActive() const override;

    int requestedEffectChainPosition() const override
    {
        return 99;
    }

    static bool supported();
    private:
        GLTexture* ensureCursorTexture();
        void slotWindowStateChanged(EffectWindow *w);
        std::unique_ptr<GLTexture> m_cursorTexture;
};


} // namespace KWin
