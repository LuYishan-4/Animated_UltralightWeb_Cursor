#include "../header/KwinCursorEffect.hpp"
#include "../header/KwinMouseProvider.hpp"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glutils.h"

#include <QDBusConnection>
#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <QOpenGLExtraFunctions>
namespace KWin {

extern EffectsHandler *effects;

KWIN_EFFECT_FACTORY_SUPPORTED(
    KWin::KwinCursorEffect,
    "metadata.json",
    return KWin::KwinCursorEffect::supported();
)

KwinCursorEffect::KwinCursorEffect() {
    if (!initializeCore<KwinMouseProvider>()) return;
    connect(effects, &EffectsHandler::windowActivated, this, &KwinCursorEffect::slotWindowStateChanged);
    
    m_mouseProvider->setCallback([this](const UltralightWebCursorM::MousePoint& pt) {
        if (!m_html) return;
        QMetaObject::invokeMethod(effects, [this, pt]() {
            if (!m_html) return;
            QRect oldRect = getCursorRect(m_cursorPoint).toRect().adjusted(-20, -20, 20, 20); 
            m_cursorPoint = QPointF(pt.x, pt.y); 

            m_html->move(pt.x, pt.y, pt.pressed);

            QRect newRect = getCursorRect(m_cursorPoint).toRect().adjusted(-20, -20, 20, 20);
            effects->addRepaint(KWin::Rect(oldRect));
            effects->addRepaint(KWin::Rect(newRect));
        }, Qt::QueuedConnection);
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
    if (!m_html) return;
    m_html->setEnabled(true);
    effects->addRepaintFull();
}

void KwinCursorEffect::disable() {
    if (!m_html) return;
    m_html->setEnabled(false);
    m_cursorTexture.reset();
    effects->addRepaintFull();
}

void KwinCursorEffect::reloadHtml() {
    UltralightWebCursorM::UserConfig::instance()->load();
    UltralightWebCursorM::CursorJSON::instance()->load(UserConfigimp.html);
    if (!m_html) return;
    m_html->reload(UserConfigimp, CursorJSONImp);
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

    QOpenGLContext* qtContext = QOpenGLContext::currentContext();
    QOpenGLExtraFunctions* funcs = qtContext ? qtContext->extraFunctions() : nullptr;

    GLint native_kwin_fbo = 0;
    if (funcs) funcs->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &native_kwin_fbo);
    m_html->update();

    if (funcs) {
        funcs->glFlush();
        if (native_kwin_fbo != 0) {
            funcs->glBindFramebuffer(GL_FRAMEBUFFER, native_kwin_fbo);
        }
    }

    int w = m_html->width();
    int h = m_html->height();
    if (w <= 0 || h <= 0) return nullptr;

    unsigned int gpuTexId = m_html->textureId(); 
    if (gpuTexId != 0) {
        static unsigned int lastGpuTexId = 0;
        if (!m_cursorTexture || m_cursorTexture->width() != w || m_cursorTexture->height() != h || lastGpuTexId != gpuTexId) {
            m_cursorTexture.reset();
            m_cursorTexture = GLTexture::createNonOwningWrapper(gpuTexId, GL_RGBA8, QSize(w, h));
            if (!m_cursorTexture) return nullptr;
            
            m_cursorTexture->setWrapMode(GL_CLAMP_TO_EDGE);
            m_cursorTexture->setFilter(GL_LINEAR);
            lastGpuTexId = gpuTexId;
        }
        return m_cursorTexture.get();
    }
    if (m_cursorTexture && !m_html->hasNewFrame()) return m_cursorTexture.get();

    const uint8_t* pixels = m_html->pixels();
    if (!pixels) return nullptr;
    
    if (m_cursorTexture && (m_cursorTexture->width() != w || m_cursorTexture->height() != h)) {
        m_cursorTexture.reset(); 
    }
    
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
    if (++frameCounter % 60 == 0) {
        unsigned int gpuTexId = m_html->textureId();
        qDebug() << "[UltralightKwinLinkDebug] [KWin Pipeline Check]"
                 << " | Qt Context :" << QOpenGLContext::currentContext()
                 << " | Native GL Tex ID:" << gpuTexId
                 << " | Texture Valid:" << (texture != nullptr);
    }

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

    QOpenGLContext* qtContext = QOpenGLContext::currentContext();
    QOpenGLFunctions* funcs = qtContext ? qtContext->functions() : nullptr;

    if (funcs) {
        funcs->glEnable(GL_BLEND);
        funcs->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    texture->render(QSizeF(w, h) * scale); 

    if (funcs) {
        funcs->glDisable(GL_BLEND);
    }

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