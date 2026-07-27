#pragma once


#include <memory>

#include <QPointF>


#include <core/output.h>
#include <kwin/effect/effect.h>


#include "UltralightHtmlEffect.hpp"
#include "MouseProvider.hpp"
#include "../config/UserConfig.hpp"
#include "../config/CursorJSON.hpp"
#include "../lib/BlackList/BlacklistManager.hpp"


namespace KWin
{


class GLTexture;



class UltralightCursorEffect :
    public Effect
{

    Q_OBJECT

     Q_DISABLE_COPY(UltralightCursorEffect)
public:


    UltralightCursorEffect();


    ~UltralightCursorEffect() override;




    bool isBlacklisted() const;
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




public Q_SLOTS:


    void enable();


    void disable();


    void reloadHtml();


private:


    void hideCursor();

    void showCursor();

    GLTexture* ensureCursorTexture();

private:


    std::unique_ptr<UltralightWebCursorM::UltralightHtmlEffect> m_html;


    std::unique_ptr<UltralightWebCursorM::IMouseProvider> m_mouseProvider;

    UltralightWebCursorM::BlacklistManager m_blacklist;

    std::unique_ptr<GLTexture> m_cursorTexture;



    QPointF m_cursorPoint;


};




}