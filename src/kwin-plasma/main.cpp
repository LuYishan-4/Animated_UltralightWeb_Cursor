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

KWIN_EFFECT_FACTORY_SUPPORTED(KWin::KwinCursorEffect, "metadata.json",
                              return KWin::KwinCursorEffect::supported();)

KwinCursorEffect::KwinCursorEffect() {
  if (!initializeCore<KwinMouseProvider>())
    return;
  connect(effects, &EffectsHandler::windowActivated, this,
          &KwinCursorEffect::slotWindowStateChanged);
  m_mouseProvider->setCallback(
      [this](const UltralightWebCursorM::MousePoint &pt) {
        if (!m_html)
          return;
        QRect oldRect =
            getCursorRect(m_cursorPoint).toRect().adjusted(-20, -20, 20, 20);
        m_cursorPoint = QPointF(pt.x, pt.y);
        m_html->move(pt.x, pt.y, pt.pressed);
        QRect newRect =
            getCursorRect(m_cursorPoint).toRect().adjusted(-20, -20, 20, 20);
        effects->addRepaint(KWin::Rect(oldRect));
        effects->addRepaint(KWin::Rect(newRect));
      });
  QDBusConnection::sessionBus().registerObject(
      QStringLiteral("/UltralightCursor"), this,
      QDBusConnection::ExportAllSlots);
  // m_html->update();
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
  m_html->setEnabled(true);
  effects->addRepaintFull();
}

void KwinCursorEffect::disable() {
  if (!m_html)
    return;
  m_html->setEnabled(false);
  m_cursorTexture.reset();
  effects->addRepaintFull();
}
void KwinCursorEffect::reloadHtml() {
  UltralightWebCursorM::UserConfig::instance()->load();
  UltralightWebCursorM::CursorJSON::instance()->load(UserConfigimp.html);
  if (!m_html)
    return;
  m_html->reload(UserConfigimp, CursorJSONImp);
  effects->addRepaintFull();
}

bool KwinCursorEffect::isBlacklisted() const {
  auto window = effects->activeWindow();
  if (!window)
    return false;
  return isWindowBlacklisted(window->windowClass().toStdString());
}
GLTexture *KwinCursorEffect::ensureCursorTexture() {
  static bool logged = false;
  if (!logged)
    logged = true;

  if (!m_html || !m_html->isEnabled() || m_isIdleHidden)
    return nullptr;

  static bool first_focus_done = false;
  if (!first_focus_done && m_html->view()) {
    m_html->view()->Focus();
    first_focus_done = true;
  }
  GLint native_draw_fbo = 0;
  GLint native_read_fbo = 0;
  GLint native_active_program = 0;
  GLint native_vertex_array = 0;
  GLint native_array_buffer = 0;
  GLint native_active_texture = 0;
  GLint native_texture_bindings_2d[3] = {0, 0, 0};
  GLint native_viewport[4] = {0, 0, 0, 0};
  GLint native_scissor_box[4] = {0, 0, 0, 0};
  GLboolean native_blend = glIsEnabled(GL_BLEND);
  GLboolean native_scissor = glIsEnabled(GL_SCISSOR_TEST);
  GLboolean native_depth_test = glIsEnabled(GL_DEPTH_TEST);
  GLboolean native_cull_face = glIsEnabled(GL_CULL_FACE);
  GLboolean native_stencil_test = glIsEnabled(GL_STENCIL_TEST);
  GLboolean native_color_mask[4] = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
  GLint native_depth_func = GL_LESS;
  GLint native_blend_src_rgb = GL_ONE;
  GLint native_blend_dst_rgb = GL_ZERO;
  GLint native_blend_src_alpha = GL_ONE;
  GLint native_blend_dst_alpha = GL_ZERO;
  GLint native_blend_equation_rgb = GL_FUNC_ADD;
  GLint native_blend_equation_alpha = GL_FUNC_ADD;
  GLint native_unpack_alignment = 4;
  GLint native_unpack_row_length = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &native_draw_fbo);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &native_read_fbo);
  glGetIntegerv(GL_CURRENT_PROGRAM, &native_active_program);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &native_vertex_array);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &native_array_buffer);
  glGetIntegerv(GL_ACTIVE_TEXTURE, &native_active_texture);
  glGetIntegerv(GL_VIEWPORT, native_viewport);
  glGetIntegerv(GL_SCISSOR_BOX, native_scissor_box);
  glGetBooleanv(GL_COLOR_WRITEMASK, native_color_mask);
  for (int unit = 0; unit < 3; ++unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &native_texture_bindings_2d[unit]);
  }
  glActiveTexture(native_active_texture);
  glGetIntegerv(GL_DEPTH_FUNC, &native_depth_func);
  glGetIntegerv(GL_BLEND_SRC_RGB, &native_blend_src_rgb);
  glGetIntegerv(GL_BLEND_DST_RGB, &native_blend_dst_rgb);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &native_blend_src_alpha);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &native_blend_dst_alpha);
  glGetIntegerv(GL_BLEND_EQUATION_RGB, &native_blend_equation_rgb);
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &native_blend_equation_alpha);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &native_unpack_alignment);
  glGetIntegerv(GL_UNPACK_ROW_LENGTH, &native_unpack_row_length);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  m_html->update();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, native_draw_fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, native_read_fbo);

  glUseProgram(native_active_program);
  glBindVertexArray(native_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, native_array_buffer);

  for (int unit = 0; unit < 3; ++unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, native_texture_bindings_2d[unit]);
  }
  glActiveTexture(native_active_texture);

  glViewport(native_viewport[0], native_viewport[1], native_viewport[2],
             native_viewport[3]);
  glScissor(native_scissor_box[0], native_scissor_box[1], native_scissor_box[2],
            native_scissor_box[3]);
  if (native_blend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  if (native_scissor)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
  if (native_depth_test)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  if (native_cull_face)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  if (native_stencil_test)
    glEnable(GL_STENCIL_TEST);
  else
    glDisable(GL_STENCIL_TEST);
  glDepthFunc(native_depth_func);
  glBlendFuncSeparate(native_blend_src_rgb, native_blend_dst_rgb,
                      native_blend_src_alpha, native_blend_dst_alpha);
  glBlendEquationSeparate(native_blend_equation_rgb,
                          native_blend_equation_alpha);
  glColorMask(native_color_mask[0], native_color_mask[1], native_color_mask[2],
              native_color_mask[3]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, native_unpack_alignment);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, native_unpack_row_length);

  int w = m_html->width();
  int h = m_html->height();
  if (w <= 0 || h <= 0)
    return nullptr;

  if (m_html->view() && m_html->view()->needs_paint()) {
    QRect repaintRect =
        getCursorRect(effects->cursorPos()).toRect().adjusted(-20, -20, 20, 20);
    effects->addRepaint(KWin::Rect(repaintRect));
  }

  unsigned int gpuTexId = m_html->textureId();
  if (gpuTexId != 0) {
    m_lastGpuTexId = gpuTexId;
    return nullptr;
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
    m_cursorTexture->bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE,
                    pixels);
    m_cursorTexture->unbind();
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
  const unsigned int gpuTexId = m_html->textureId();
  if (gpuTexId == 0 && !texture) {
    effects->addRepaintFull();
    return;
  }
  const int w = m_html->width();
  const int h = m_html->height();

  QPointF hotspot(m_html->hotspotX(), m_html->hotspotY());
  QPointF pos = effects->cursorPos() - screen->geometry().topLeft() - hotspot;

  auto scale = viewport.scale();
  QMatrix4x4 mvp = viewport.projectionMatrix();
  mvp.translate(pos.x() * scale, pos.y() * scale);

  if (texture) {
    ShaderBinder binder(ShaderTrait::MapTexture);
    GLShader *shader = binder.shader();
    if (!shader)
      return;
    shader->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, mvp);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    texture->render(QSizeF(w, h) * scale);
    glDisable(GL_BLEND);
  }
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
  if (!checkFullScreen()) {
    if (m_isIdleHidden) {
      m_isIdleHidden = false;
      if (m_html)
        enable();
      effects->addRepaintFull();
    }
  } else {
    m_isIdleHidden = true;
    if (m_html)
      disable();
    effects->addRepaintFull();
  }
}

} // namespace KWin

#include "main.moc"
