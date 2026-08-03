#include "../header/UltralightCursorEffect.hpp"
#include "../header/KwinMouseProvider.hpp"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glutils.h"
#include <QDBusConnection>
#include <QImage>
#include <iostream>
#include <stdexcept>

#include <QOpenGLFunctions>

namespace KWin {

extern EffectsHandler *effects;

KWIN_EFFECT_FACTORY_SUPPORTED(
    KWin::UltralightCursorEffect,
    "metadata.json",
    return KWin::UltralightCursorEffect::supported();
)

UltralightCursorEffect::UltralightCursorEffect() {
    if (!initializeCore<KwinMouseProvider>()) {
        qCritical() << "[UltralightCursorEffect] Core initialization failed!";
        return;
    }
    qDebug() << "[UltralightCursorEffect] init";
    connect(effects, &EffectsHandler::windowActivated, this, &UltralightCursorEffect::slotWindowStateChanged);
    connect(effects, &EffectsHandler::windowGeometryShapeChanged, this, [this](EffectWindow *w) {
        if (w == effects->activeWindow()) {
            slotWindowStateChanged(w);
        }
    });
    m_mouseProvider->setCallback([this](const UltralightWebCursorM::MousePoint& pt) {
        if (!m_html) return;
        QRect oldRect = getCursorRect(m_cursorPoint).toRect();
        m_cursorPoint = QPointF(pt.x, pt.y);
        m_html->move(pt.x, pt.y, pt.pressed);
        QRect newRect = getCursorRect(m_cursorPoint).toRect();
        effects->addRepaint(oldRect);
        effects->addRepaint(newRect);
    });

    m_mouseProvider->initialize();

    QDBusConnection::sessionBus().registerObject(
        QStringLiteral("/UltralightCursor"),
        this,
        QDBusConnection::ExportAllSlots
    );
}

UltralightCursorEffect::~UltralightCursorEffect()m_cursorTexture.reset();


bool UltralightCursorEffect::supported() {
    return effects->isOpenGLCompositing();
}

void UltralightCursorEffect::enable() {
    UltralightWebCursorM::MainCursorStaff::enable();
    effects->addRepaintFull();
}

void UltralightCursorEffect::disable() {
    UltralightWebCursorM::MainCursorStaff::disable();
    m_cursorTexture.reset();
    effects->addRepaintFull();
}

void UltralightCursorEffect::reloadHtml() {
    UltralightWebCursorM::MainCursorStaff::reloadHtml();
    effects->addRepaintFull();
}

bool UltralightCursorEffect::isBlacklisted() const {
    auto window = effects->activeWindow();
    if (!window) return false;
    // 呼叫基底類別通用的黑名單篩選工具
    return isWindowBlacklisted(window->windowClass().toStdString());
}
GLTexture* UltralightCursorEffect::ensureCursorTexture() {
    // 註：m_isIdleHidden 變數已由繼承 MainCursorStaff 獲得
    if (!m_html || !m_html->isEnabled() || m_isIdleHidden) return nullptr;
    
    m_html->update();
    
    if (m_cursorTexture && !m_html->hasNewFrame()) return m_cursorTexture.get();

    const uint8_t* pixels = m_html->pixels();
    if (!pixels) return nullptr;

    int w = m_html->width();
    int h = m_html->height();

    if (!m_cursorTexture) {
        QImage emptyImage(w, h, QImage::Format_ARGB32_Premultiplied);
        emptyImage.fill(Qt::transparent);
        m_cursorTexture = GLTexture::upload(emptyImage);
        if (!m_cursorTexture) return nullptr;
        m_cursorTexture->setWrapMode(GL_CLAMP_TO_EDGE);
    }

    if (m_html->hasNewFrame()) {
        m_cursorTexture->bind();
        QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();
        funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        m_cursorTexture->unbind();
    }

    m_html->clearNewFrame();
    return m_cursorTexture.get();
}

void UltralightCursorEffect::paintScreen(
    const RenderTarget& renderTarget,
    const RenderViewport& viewport,
    int mask,
    const Region& region,
    LogicalOutput* screen
) {
    effects->paintScreen(renderTarget, viewport, mask, region, screen);

    GLTexture* texture = ensureCursorTexture();
    if (!texture) return;

    const int w = m_html->width();
    const int h = m_html->height();
    QPointF hotspot(w / 2.0, h / 2.0);
    QPointF pos = effects->cursorPos() - screen->geometry().topLeft() - hotspot;
    auto scale = viewport.scale();

    ShaderBinder binder(ShaderTrait::MapTexture | ShaderTrait::TransformColorspace);
    GLShader* shader = binder.shader();

    shader->setColorspaceUniforms(
        ColorDescription::sRGB,
        renderTarget.colorDescription(),
        RenderingIntent::Perceptual
    );

    QMatrix4x4 mvp = viewport.projectionMatrix();
    mvp.translate(pos.x() * scale, pos.y() * scale);
    shader->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, mvp);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    texture->render(QSizeF(w, h) * scale);
    
    glDisable(GL_BLEND);

    if (m_html->hasNewFrame()) {
        effects->addRepaint(getCursorRect(effects->cursorPos()).toRect());
    }
}

bool UltralightCursorEffect::isActive() const {
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

#include "UltralightCursorEffect.moc"
