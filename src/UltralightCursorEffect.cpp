#include "header/UltralightCursorEffect.hpp"
#include "header/KwinMouseProvider.hpp"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glutils.h"
#include <QDBusConnection>
#include <QImage>
#include <iostream>
#include <stdexcept>
#include <QTimer>

namespace KWin{


KWIN_EFFECT_FACTORY_SUPPORTED(
       KWin::UltralightCursorEffect,
        "metadata.json",
        return KWin::UltralightCursorEffect::supported();
)



UltralightCursorEffect::UltralightCursorEffect(){

    try{
    UltralightWebCursorM::UserConfig::instance()->load();
    UltralightWebCursorM::CursorJSON::instance()->load(UserConfigimp.html);
    m_html = std::make_unique<UltralightWebCursorM::UltralightHtmlEffect >();
    if(!m_html->initialize(UserConfigimp,CursorJSONImp)){
        m_html.reset();
        return;
    }
    qDebug() << "[UltralightCursorEffect] init";
    m_blacklist.setBlacklist(UltralightWebCursorM::UserConfig::instance()->getBlacklist());

    m_idleTimer = new QTimer(this);
    m_idleTimer->setSingleShot(true);
    m_idleTimer->setInterval(2500); 
    connect(m_idleTimer, &QTimer::timeout, this, [this]() {
        if (checkFullScreen()) {
            m_isIdleHidden = true;
            effects->addRepaintFull();
        }
    });
    connect(effects, &EffectsHandler::windowActivated, this, &UltralightCursorEffect::slotWindowStateChanged);
    m_mouseProvider =std::make_unique<KwinMouseProvider>();
    m_mouseProvider->setCallback([this](const UltralightWebCursorM::MousePoint& pt){
        if(!m_html)return;
        m_cursorPoint =
            QPointF(
                pt.x,
                pt.y
             );
        m_html->move(
                pt.x,
                pt.y,
                pt.pressed
            );

        effects->addRepaintFull();
        }
);

m_mouseProvider->initialize();
m_idleTimer->start();

QDBusConnection::sessionBus().registerObject(
    QStringLiteral("/UltralightCursor"),
        this,
        QDBusConnection::ExportAllSlots
    );
    }catch (const char* msg) { 
            std::cerr << "error: " << msg << "\n";
    } 
}

    UltralightCursorEffect::~UltralightCursorEffect(){
        m_cursorTexture.reset();
        m_mouseProvider.reset();
        m_html.reset();
    }

    //GPU-Supported
    bool UltralightCursorEffect::supported(){
        return effects->isOpenGLCompositing();
    }

    void UltralightCursorEffect::enable(){
        if(!m_html)return;
        m_html->setEnabled(true);
        effects->addRepaintFull();
    }

    void UltralightCursorEffect::disable(){
        if(!m_html)return;
        m_html->setEnabled(false);
        m_cursorTexture.reset();
        effects->addRepaintFull();
    }
    void UltralightCursorEffect::reloadHtml(){
        UltralightWebCursorM::UserConfig::instance()->load();
        UltralightWebCursorM::CursorJSON::instance()->load(UserConfigimp.html);
        if(!m_html)return;
        m_html->reload(UserConfigimp,CursorJSONImp);
        effects->addRepaintFull();
    }
    bool UltralightCursorEffect::isBlacklisted() const {
         auto window = effects->activeWindow();
          if(!window) return false;
           QString app = window->windowClass();
            return m_blacklist.contains( app.toStdString() );
     }

    GLTexture* UltralightCursorEffect::ensureCursorTexture(){
        
        if(!m_html)return nullptr;
        if(!m_html->isEnabled())return nullptr;
        if (m_isIdleHidden)return nullptr;
            



        m_html->update();
        if(m_cursorTexture &&!m_html->hasNewFrame())return m_cursorTexture.get();

        const uint8_t* pixels =m_html->pixels();

        if(!pixels)return nullptr;

        QImage image(
            pixels,
            m_html->width(),
            m_html->height(),
            m_html->stride(),
            QImage::Format_ARGB32_Premultiplied
        );
        m_cursorTexture =
            GLTexture::upload(
            image.copy()
        );
        if(!m_cursorTexture)return nullptr;
        m_cursorTexture->setWrapMode(GL_CLAMP_TO_EDGE);
        m_html->clearNewFrame();
        return m_cursorTexture.get();
    }

    void UltralightCursorEffect::paintScreen(
        const RenderTarget& renderTarget,
        const RenderViewport& viewport,
        int mask,
        const Region& region,
        LogicalOutput* screen
    ){
        effects->paintScreen(
            renderTarget,
            viewport,
            mask,
            region,
            screen
        );
      


        GLTexture* texture =ensureCursorTexture();
        if(!texture){
            effects->addRepaintFull();
            return;
         }
       const int w=m_html->width();
       const int h=m_html->height();

        QPointF hotspot(
            w/2.0,
            h/2.0
        );

        QPointF pos =effects->cursorPos() - screen->geometry().topLeft() - hotspot;
         auto scale =  viewport.scale();

        ShaderBinder binder(
            ShaderTrait::MapTexture |
            ShaderTrait::TransformColorspace
        );

        GLShader* shader =binder.shader();

        shader->setColorspaceUniforms(
            ColorDescription::sRGB,
            renderTarget.colorDescription(),
            RenderingIntent::Perceptual
        );

        QMatrix4x4 mvp =viewport.projectionMatrix();

        mvp.translate(
            pos.x()*scale,
            pos.y()*scale
         );

        shader->setUniform(
            GLShader::Mat4Uniform::ModelViewProjectionMatrix,
            mvp
        );

        glEnable(GL_BLEND);

        glBlendFunc(
            GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA
        );
        texture->render(QSizeF(w,h)*scale);
        glDisable(GL_BLEND);
        effects->addRepaintFull();
    }

    bool UltralightCursorEffect::isActive() const{
        return m_html != nullptr;
    }
       bool UltralightCursorEffect::checkFullScreen() const {
        if (EffectWindow *activeWin = effects->activeWindow()) {
            return activeWin->isFullScreen();
        }
        return false;
    }

   void UltralightCursorEffect::slotWindowStateChanged(EffectWindow *w) {
        Q_UNUSED(w);
        if (!checkFullScreen()) {
            if (m_isIdleHidden) {
                m_isIdleHidden = false;
                effects->addRepaintFull();
            }
            m_idleTimer->stop();
        } else {
            m_idleTimer->start();
    }



}

#include "UltralightCursorEffect.moc"