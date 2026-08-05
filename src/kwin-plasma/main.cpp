#include "../header/KwinCursorEffect.hpp"
#include "../header/KwinMouseProvider.hpp"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glutils.h"
#include <QDBusConnection>
#include <QImage>
#include <iostream>
#include <stdexcept>
#include <QOpenGLContext> 
#include <QOpenGLFunctions> 
#include <GL/gl.h>

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
        QRect oldRect = getCursorRect(m_cursorPoint).toRect(); 
        m_cursorPoint = QPointF(pt.x, pt.y); 

        
        m_html->move(pt.x, pt.y, pt.pressed);
        
        m_html->update(); 
        
        QRect newRect = getCursorRect(m_cursorPoint).toRect();
        effects->addRepaint(KWin::Rect(oldRect.adjusted(-5, -5, 5, 5)));
        effects->addRepaint(KWin::Rect(newRect.adjusted(-5, -5, 5, 5)));
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
    if (m_html) {
        QRect lastCursorRect = getCursorRect(effects->cursorPos()).toRect().adjusted(-20, -20, 20, 20);
        effects->addRepaint(KWin::Rect(lastCursorRect));
        m_html->setEnabled(false);
    }
    m_cursorTexture.reset();
    effects->addRepaintFull();
    qDebug() << "[UltralightDebug] isnowdisable";
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
    
    int w = m_html->width();
    int h = m_html->height();
    if (w <= 0 || h <= 0) return nullptr;
    
    unsigned int gpuTexId = m_html->textureId();
    if (gpuTexId != 0) {
        if (!m_cursorTexture || m_cursorTexture->width() != w || m_cursorTexture->height() != h) {
            m_cursorTexture.reset();
            m_cursorTexture = GLTexture::createNonOwningWrapper(gpuTexId, GL_RGBA, QSize(w, h));
            if (!m_cursorTexture) return nullptr;
            m_cursorTexture->setWrapMode(GL_CLAMP_TO_EDGE);
            m_cursorTexture->setFilter(GL_LINEAR);
        }
        return m_cursorTexture.get();
    }
    if (m_cursorTexture && m_cursorTexture->width() == w && m_cursorTexture->height() == h && !m_html->hasNewFrame()) return m_cursorTexture.get();
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
    
    GLTexture* texture = ensureCursorTexture();
    
    static int frameCounter = 0;
    frameCounter++;
    
    if (frameCounter % 60 == 0) {
        if (!m_html) {
            qDebug() << "[UltralightDebug] html none！";
        } else {
            unsigned int gpuTexId = m_html->textureId();
            qDebug() << "[UltralightDebug] info:" << m_html->width() << "x" << m_html->height()
                     << "| isenable:" << m_html->isEnabled()
                     << "| (hasNewFrame):" << m_html->hasNewFrame()
                     << "| GPU  ID:" << gpuTexId
                     << "| KWin " << (texture != nullptr);
        }
    }
    // -------------------------------------------------------------------

    if (!texture || !m_html) return;
    
    const int w = m_html->width();
    const int h = m_html->height();
    
    // ----------------- [DEBUG ] -----------------
    if (m_html->hasNewFrame() && frameCounter % 30 == 0) {
        const uint8_t* rawPixels = m_html->pixels();
        if (rawPixels) {
            int centerX = w / 2;
            int centerY = h / 2;
            size_t pixelOffset = (centerY * m_html->stride()) + (centerX * 4);
            uint8_t b = rawPixels[pixelOffset];
            uint8_t g = rawPixels[pixelOffset + 1];
            uint8_t r = rawPixels[pixelOffset + 2];
            uint8_t a = rawPixels[pixelOffset + 3];
            
            qDebug() << "[UltralightDebug] htmlpixel(" << centerX << "," << centerY << ") "
                     << "Raw RGBA: (" << (int)r << "," << (int)g << "," << (int)b << "," << (int)a << ")"
                     << " -> " << (a == 0 ? "none" : "noneee");
        } else {
            if (m_html->textureId() == 0) {
                qDebug() << "[UltralightDebug] none gpu";
            }
        }
    }
    QPointF hotspot(m_html->hotspotX(), m_html->hotspotY());
    QPointF pos = effects->cursorPos() - screen->geometry().topLeft() - hotspot;
    
    QMatrix4x4 mvp = viewport.projectionMatrix();
    mvp.translate(pos.x(), pos.y()); 

    ShaderBinder binder(ShaderTrait::MapTexture | ShaderTrait::TransformColorspace);
    GLShader* shader = binder.shader();
    if (!shader) return;

    shader->setColorspaceUniforms(
        ColorDescription::sRGB,
        renderTarget.colorDescription(),
        RenderingIntent::Perceptual
    );
    shader->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, mvp);
    texture->render(QSize(w, h)); 
    if (m_html->hasNewFrame()) {
        QRect repaintRect = getCursorRect(effects->cursorPos()).toRect().adjusted(-5, -5, 5, 5);
        qDebug() << "[UltralightDebug] reloooooooooooooooooo" 
                 << repaintRect;
        effects->addRepaint(KWin::Rect(repaintRect));
        m_html->clearNewFrame();
    }
    // -------------------------------------------------------------------
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
