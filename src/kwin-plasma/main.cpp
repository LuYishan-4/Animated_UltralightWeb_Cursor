#include "../header/KwinCursorEffect.hpp"
#include "../header/KwinMouseProvider.hpp"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glutils.h"
#include <QDBusConnection>
#include <QImage>
#include <qlogging.h>

namespace KWin {

extern EffectsHandler *effects;

KWIN_EFFECT_FACTORY_SUPPORTED(KWin::KwinCursorEffect,
                              "ultralightwebcursor.json",
                              return KWin::KwinCursorEffect::supported();)

KwinCursorEffect::KwinCursorEffect() {
  if (!initializeCore<KwinMouseProvider>())
    return;

  connect(effects, &EffectsHandler::windowActivated, this,
          &KwinCursorEffect::slotWindowStateChanged);

  m_idleTimer.setSingleShot(true);
  m_idleTimer.setInterval(3000);
  connect(&m_idleTimer, &QTimer::timeout, this,
          &KwinCursorEffect::slotIdleTimeout);

  m_mouseProvider->setCallback(
      [this](const UltralightWebCursorM::MousePoint &pt) {
        if (!m_html)
          return;
        if (m_autoHidden) {
          m_autoHidden = false;
          m_html->setEnabled(true);
          effects->addRepaintFull();
        }

        QRect oldRect =
            getCursorRect(m_cursorPoint).toRect().adjusted(-20, -20, 20, 20);
        m_cursorPoint = QPointF(pt.x, pt.y);

        // While disabled we only track the position so the cursor reappears
        // where the pointer is; skip the JS evaluation and repaints entirely.
        if (!m_html->isEnabled())
          return;

        m_html->move(pt.x, pt.y, pt.pressed);

        QRect newRect =
            getCursorRect(m_cursorPoint).toRect().adjusted(-20, -20, 20, 20);
        effects->addRepaint(KWin::Rect(oldRect));
        effects->addRepaint(KWin::Rect(newRect));

        if (!m_isIdleHidden)
          startIdleTimer();
      });
  QDBusConnection::sessionBus().registerObject(
      QStringLiteral("/UltralightCursor"), this,
      QDBusConnection::ExportAllSlots);
}

KwinCursorEffect::~KwinCursorEffect() {
  if (m_mouseProvider) {
    m_mouseProvider->setCallback(nullptr);
    m_mouseProvider.reset();
  }
  m_cursorTexture.reset();
}

bool KwinCursorEffect::supported() { return effects->isOpenGLCompositing(); }

void KwinCursorEffect::enable() {
  if (!m_html)
    return;
  m_autoHidden = false;
  m_html->setEnabled(true);
  if (!m_isIdleHidden)
    startIdleTimer();
  effects->addRepaintFull();
}

void KwinCursorEffect::disable() {
  if (!m_html)
    return;
  m_idleTimer.stop();
  m_autoHidden = false;
  m_html->setEnabled(false);
  m_cursorTexture.reset();
  effects->addRepaintFull();
}

void KwinCursorEffect::reloadHtml() {
  UltralightWebCursorM::UserConfig::instance()->load();
  UltralightWebCursorM::CursorJSON::instance()->load(UserConfigValues.html);
  if (!m_html)
    return;
  m_html->reload(UserConfigValues, CursorJSONImp);
  effects->addRepaintFull();
}

void KwinCursorEffect::reconfigure(ReconfigureFlags flags) {
  Q_UNUSED(flags);
  m_cursorTexture.reset();
}

bool KwinCursorEffect::isBlacklisted() const {
  auto window = effects->activeWindow();
  if (!window)
    return false;
  return isWindowBlacklisted(window->windowClass().toStdString());
}

GLTexture *KwinCursorEffect::ensureCursorTexture() {
  if (!m_html || !m_html->isEnabled() || m_isIdleHidden)
    return nullptr;

  if (!m_firstFocusDone && m_html->view()) {
    m_html->view()->Focus();
    m_firstFocusDone = true;
  }
  m_html->update();

  int w = m_html->width();
  int h = m_html->height();
  if (w <= 0 || h <= 0)
    return nullptr;

  if (m_cursorTexture && !glIsTexture(m_cursorTexture->texture()))
    m_cursorTexture.reset();

  if (m_html->view() && m_html->view()->needs_paint()) {
    QRect repaintRect =
        getCursorRect(effects->cursorPos()).toRect().adjusted(-20, -20, 20, 20);
    effects->addRepaint(KWin::Rect(repaintRect));
  }

  if (m_cursorTexture && !m_html->hasNewFrame())
    return m_cursorTexture.get();

  const uint8_t *pixels = m_html->pixels();
  if (!pixels)
    return nullptr;
  if (m_cursorTexture &&
      (m_cursorTexture->width() != w || m_cursorTexture->height() != h))
    m_cursorTexture.reset();
  if (!m_cursorTexture) {
    QImage wrapperImage(const_cast<uint8_t *>(pixels), w, h, m_html->stride(),
                        QImage::Format_ARGB32_Premultiplied);
    m_cursorTexture = GLTexture::upload(wrapperImage);
    if (!m_cursorTexture)
      return nullptr;
    m_cursorTexture->setWrapMode(GL_CLAMP_TO_EDGE);
    m_html->clearNewFrame();
    return m_cursorTexture.get();
  }

  if (m_html->hasNewFrame()) {
    GLint savedUnpackAlignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &savedUnpackAlignment);
    m_cursorTexture->bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE,
                    pixels);
    m_cursorTexture->unbind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, savedUnpackAlignment);
    m_html->clearNewFrame();
  }
  return m_cursorTexture.get();
}

void KwinCursorEffect::paintScreen(const RenderTarget &renderTarget,
                                   const RenderViewport &viewport, int mask,
                                   const Region &region,
                                   LogicalOutput *screen) {
  effects->paintScreen(renderTarget, viewport, mask, region, screen);
  if (!m_html || !m_html->isEnabled() || m_isIdleHidden)
    return;

  GLTexture *texture = ensureCursorTexture();
  if (!texture)
    return;
  const int w = m_html->width();
  const int h = m_html->height();
  if (w <= 0 || h <= 0)
    return;

  QPointF hotspot(m_html->hotspotX(), m_html->hotspotY());
  QPointF pos = effects->cursorPos() - screen->geometry().topLeft() - hotspot;

  auto scale = viewport.scale();
  QMatrix4x4 mvp = viewport.projectionMatrix();
  mvp.translate(pos.x() * scale, pos.y() * scale);

  GLint savedBlendSrc = GL_ONE;
  GLint savedBlendDst = GL_ZERO;
  GLboolean blendEnabled = glIsEnabled(GL_BLEND);
  glGetIntegerv(GL_BLEND_SRC_RGB, &savedBlendSrc);
  glGetIntegerv(GL_BLEND_DST_RGB, &savedBlendDst);
  GLint savedTexture = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &savedTexture);
  GLint savedActiveTexture = GL_TEXTURE0;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &savedActiveTexture);
  GLint savedProgram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &savedProgram);
  GLint savedVao = 0;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &savedVao);
  GLint savedArrayBuffer = 0;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &savedArrayBuffer);
  GLint savedViewport[4] = {0, 0, 0, 0};
  glGetIntegerv(GL_VIEWPORT, savedViewport);
  GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
  GLint savedScissor[4] = {0, 0, 0, 0};
  glGetIntegerv(GL_SCISSOR_BOX, savedScissor);

  ShaderBinder binder(ShaderTrait::MapTexture);
  GLShader *shader = binder.shader();
  if (shader) {
    shader->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, mvp);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    texture->render(QSizeF(w, h) * scale);
    glDisable(GL_BLEND);
  }

  if (blendEnabled)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  glBlendFunc(savedBlendSrc, savedBlendDst);
  glActiveTexture(savedActiveTexture);
  glBindTexture(GL_TEXTURE_2D, savedTexture);
  if (savedProgram)
    glUseProgram(savedProgram);
  glBindVertexArray(savedVao);
  glBindBuffer(GL_ARRAY_BUFFER, savedArrayBuffer);
  glViewport(savedViewport[0], savedViewport[1], savedViewport[2],
             savedViewport[3]);
  if (scissorEnabled)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
  glScissor(savedScissor[0], savedScissor[1], savedScissor[2], savedScissor[3]);

  if (m_html->view() && m_html->view()->needs_paint()) {
    QRect repaintRect =
        getCursorRect(effects->cursorPos()).toRect().adjusted(-20, -20, 20, 20);
    effects->addRepaint(KWin::Rect(repaintRect));
  }
  effects->addRepaintFull();
}

bool KwinCursorEffect::isActive() const { return m_html != nullptr; }

bool KwinCursorEffect::checkFullScreen() const {
  if (EffectWindow *activeWin = effects->activeWindow()) {
    return activeWin->isFullScreen();
  }
  return false;
}

void KwinCursorEffect::slotWindowStateChanged(EffectWindow *w) {
  Q_UNUSED(w);
  const bool isFullScreen = checkFullScreen();
  if (isFullScreen == m_isIdleHidden)
    return;
  m_isIdleHidden = isFullScreen;
  if (m_html) {
    if (isFullScreen) {
      m_idleTimer.stop();
      m_html->setEnabled(false);
    } else {
      m_html->setEnabled(!m_autoHidden);
      if (!m_autoHidden)
        startIdleTimer();
    }
  }
  effects->addRepaintFull();
}

void KwinCursorEffect::startIdleTimer() { m_idleTimer.start(); }

void KwinCursorEffect::slotIdleTimeout() {
  if (m_isIdleHidden || !m_html)
    return;
  m_autoHidden = true;
  m_html->setEnabled(false);
  effects->addRepaintFull();
}

} // namespace KWin

#include "main.moc"
