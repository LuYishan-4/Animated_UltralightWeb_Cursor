#include <epoxy/gl.h>  
#include "../header/KwinCursorEffect.hpp"
#include "../header/KwinMouseProvider.hpp"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include <QDBusConnection>
#include <QImage>
#include <iostream>
#include <stdexcept>
#include <QOpenGLContext> 
#include <QOpenGLFunctions> 
#include <thread> 
#include "opengl/glutils.h"
namespace KWin {

extern EffectsHandler *effects;

KWIN_EFFECT_FACTORY_SUPPORTED(
    KWin::KwinCursorEffect,
    "metadata.json",
    return KWin::KwinCursorEffect::supported();
)

KwinCursorEffect::KwinCursorEffect() {
    if (!initializeCore<KwinMouseProvider>())return;
    connect(effects, &EffectsHandler::windowActivated, this, &KwinCursorEffect::slotWindowStateChanged);
    m_mouseProvider->setCallback([this](const UltralightWebCursorM::MousePoint& pt) {
        if (!m_html) return;
        QRect oldRect = getCursorRect(m_cursorPoint).toRect().adjusted(-20, -20, 20, 20); 
        m_cursorPoint = QPointF(pt.x, pt.y); 

        m_html->move(pt.x, pt.y, pt.pressed);

        QRect newRect = getCursorRect(m_cursorPoint).toRect().adjusted(-20, -20, 20, 20);
        effects->addRepaint(KWin::Rect(oldRect));
        effects->addRepaint(KWin::Rect(newRect));
    });
    QDBusConnection::sessionBus().registerObject(
        QStringLiteral("/UltralightCursor"),
        this,
        QDBusConnection::ExportAllSlots
    );
}

KwinCursorEffect::~KwinCursorEffect() {
    if (m_mouseProvider) {
        m_mouseProvider->setCallback(nullptr);
        m_mouseProvider.reset();
    }
    m_cursorTexture.reset();
}

bool KwinCursorEffect::supported() {
    return effects->isOpenGLCompositing();
}

void KwinCursorEffect::enable() {
    UltralightWebCursorM::MainCursorStaff::enable();
    effects->addRepaintFull();
}

void KwinCursorEffect::disable() {
    UltralightWebCursorM::MainCursorStaff::disable();
    m_cursorTexture.reset();
    effects->addRepaintFull();
}

void KwinCursorEffect::reloadHtml() {
    UltralightWebCursorM::MainCursorStaff::reloadHtml();
    effects->addRepaintFull();
}

bool KwinCursorEffect::isBlacklisted() const {
    auto window = effects->activeWindow();
    if (!window) return false;
    return isWindowBlacklisted(window->windowClass().toStdString());
}
GLTexture* KwinCursorEffect::ensureCursorTexture() {
    if (!m_html || !m_html->isEnabled() || m_isIdleHidden) return nullptr;
    static bool first_focus_done = false;
    if (!first_focus_done && m_html->view()) {
        m_html->view()->Focus();
        first_focus_done = true;
    }

    m_html->update();

    int w = m_html->width();
    int h = m_html->height();
    if (w <= 0 || h <= 0) return nullptr;
    
    unsigned int gpuTexId = m_html->textureId();
    if (gpuTexId != 0) {
        if (!m_cursorTexture || m_cursorTexture->width() != w || m_cursorTexture->height() != h) {
            m_cursorTexture.reset();
            m_cursorTexture = GLTexture::createNonOwningWrapper(gpuTexId, GL_RGBA8, QSize(w, h));
            if (!m_cursorTexture) return nullptr;
            m_cursorTexture->setWrapMode(GL_CLAMP_TO_EDGE);
            m_cursorTexture->setFilter(GL_LINEAR);
        }
        return m_cursorTexture.get();
    }
    
    if (m_cursorTexture && !m_html->hasNewFrame()) return m_cursorTexture.get();

    const uint8_t* pixels = m_html->pixels();
    if (!pixels) return nullptr;
    if (m_cursorTexture && (m_cursorTexture->width() != w || m_cursorTexture->height() != h)) m_cursorTexture.reset(); 
    if (!m_cursorTexture) {
        QImage wrapperImage(const_cast<uint8_t*>(pixels), w, h, m_html->stride(), QImage::Format_ARGB32_Premultiplied);
        m_cursorTexture = GLTexture::upload(wrapperImage);
        if (!m_cursorTexture) return nullptr;
        m_cursorTexture->setWrapMode(GL_CLAMP_TO_EDGE);
        m_html->clearNewFrame();
        return m_cursorTexture.get();
    }

    if (m_html->hasNewFrame()) {
        m_cursorTexture->bind();
        QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();
        if (funcs) {
            funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, 4); 
            funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        }
        m_cursorTexture->unbind();
        m_html->clearNewFrame();
    }
    return m_cursorTexture.get();
}

void KwinCursorEffect::paintScreen(const RenderTarget& renderTarget, const RenderViewport& viewport, int mask, const Region& region, LogicalOutput* screen) {
    effects->paintScreen(renderTarget, viewport, mask, region, screen);
    if (!m_html || !m_html->isEnabled() || m_isIdleHidden) return;
    
    GLTexture* texture = ensureCursorTexture();
    static int frameCounter = 0;
    frameCounter++;
    

    if (frameCounter % 60 == 0) {
        unsigned int gpuTexId = m_html->textureId();
        auto kwin_qt_context = QOpenGLContext::currentContext();
        
        qDebug() << "[UltralightKwinLinkDebug]  [KWin Pipeline Context Check]"
                 << " | Qt Context :" << kwin_qt_context
                 << " | Wrapped Texture ID:" << gpuTexId
                 << " | Wrapped Status:" << (texture != nullptr);
    }
    // ----------------------------------------------------------------------

    if (!texture) {
        effects->addRepaintFull();
        return;
    }
    const int w = m_html->width();
    const int h = m_html->height();

    QPointF hotspot(w / 2.0, h / 2.0);
    QPointF pos = effects->cursorPos() - screen->geometry().topLeft() - hotspot;
    
    auto scale = viewport.scale();
    QMatrix4x4 mvp = viewport.projectionMatrix();
    mvp.translate(pos.x() * scale, pos.y() * scale); 
    ShaderBinder binder(ShaderTrait::MapTexture);
    GLShader* shader = binder.shader();
    if (!shader) return;

    shader->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, mvp);

    glEnablei(GL_BLEND, 0);
    glBlendFunci(0, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    texture->render(QSizeF(w, h) * scale); 
    glDisablei(GL_BLEND, 0);

    QRect repaintRect = getCursorRect(effects->cursorPos()).toRect().adjusted(-20, -20, 20, 20);
    effects->addRepaint(KWin::Rect(repaintRect));
}




bool KwinCursorEffect::isActive() const {
    return m_html != nullptr;
}

bool KwinCursorEffect::checkFullScreen() const {
    if (EffectWindow *activeWin = effects->activeWindow()) {
        return activeWin->isFullScreen();
    }
    return false;
}

void KwinCursorEffect::slotWindowStateChanged(EffectWindow *w) {
    Q_UNUSED(w);
    if (!checkFullScreen()) {
        if (m_isIdleHidden) {
            m_isIdleHidden = false;
            if (m_html) m_html->setEnabled(true); 
            effects->addRepaintFull();
        }
    } else {
        m_isIdleHidden = true;
        if (m_html) m_html->setEnabled(false);
        effects->addRepaintFull();
    }
}

} // namespace KWin

#include "main.moc"
