#include "../header/QtCursorEffect.hpp"

#include "../header/QtMouseProvider.hpp"
#include "SharedCursorRender.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QScreen>
#include <QSurfaceFormat>
#include <QVariant>

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__) || defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>
#undef Status
#undef Bool
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef Cursor
#undef Screen
#undef Window
#undef Event
#endif

namespace UltralightWebCursorM {

namespace {

// Verbose tracing is opt-in: set ULTRALIGHTWEBCURSOR_DEBUG=1 in the
// environment to see per-frame mouse/render diagnostics.
bool debugEnabled() {
  static const bool enabled =
      qEnvironmentVariableIsSet("ULTRALIGHTWEBCURSOR_DEBUG");
  return enabled;
}

} // namespace

QtCursorEffect::QtCursorEffect(QObject *parent) : QObject(parent) {
  connect(&timer_, &QTimer::timeout, this, &QtCursorEffect::onTick);

  m_viewWindow = std::make_unique<QWindow>();
  m_viewWindow->setFlags(Qt::FramelessWindowHint |
                         Qt::WindowTransparentForInput |
                         Qt::WindowStaysOnTopHint | Qt::ToolTip);

  // Per-pixel alpha keeps the empty parts of the overlay transparent instead
  // of letting the windowing system paint them opaque.
  QSurfaceFormat format = m_viewWindow->format();
  format.setAlphaBufferSize(8);
  m_viewWindow->setFormat(format);

  if (QScreen *screen = QGuiApplication::primaryScreen()) {
    m_viewWindow->setGeometry(screen->geometry());
    qDebug() << "[QtCursorEffect] platform:" << QGuiApplication::platformName()
             << "screen:" << screen->geometry()
             << "dpr:" << screen->devicePixelRatio();
  }

  // QWindow::requestUpdate() delivers QEvent::UpdateRequest to the window
  // itself, not to this QObject, so watch the window instead.
  m_viewWindow->installEventFilter(this);

  m_viewWindow->show();
  qDebug() << "[QtCursorEffect] window shown | winId:" << m_viewWindow->winId()
           << "size:" << m_viewWindow->size()
           << "visible:" << m_viewWindow->isVisible()
           << "alphaBuffer:" << m_viewWindow->format().alphaBufferSize();

#if defined(_WIN32) || defined(_WIN64)
  HWND hwnd = reinterpret_cast<HWND>(m_viewWindow->winId());
  LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
  SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED);
  SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
#elif defined(__linux__) || defined(Q_OS_LINUX)
  Display *dpy = XOpenDisplay(nullptr);
  if (dpy) {
    Window winId = static_cast<Window>(m_viewWindow->winId());
    XserverRegion region = XFixesCreateRegion(dpy, nullptr, 0);
    XFixesSetWindowShapeRegion(dpy, winId, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(dpy, region);
    XFlush(dpy);
    XCloseDisplay(dpy);
  }
#endif

  m_backingStore = std::make_unique<QBackingStore>(m_viewWindow.get());
  m_backingStore->resize(m_viewWindow->size());
  qDebug() << "[QtCursorEffect] backing store ready, size:"
           << m_viewWindow->size();
}

QtCursorEffect::~QtCursorEffect() = default;

bool QtCursorEffect::initialize() {
  if (!initializeCore<QtMouseProvider>()) {
    qCritical() << "UltralightCore or QtMouseProvider initialize failed";
    return false;
  }

  m_mouseProvider->setCallback(
      [this](const UltralightWebCursorM::MousePoint &pt) {
        if (m_html) {
          m_cursorPoint = QPointF(pt.x, pt.y);
          m_html->move(pt.x, pt.y, pt.pressed);
          if (debugEnabled())
            qDebug() << "[QtCursorEffect] mouse" << m_cursorPoint
                     << "pressed:" << pt.pressed;
          m_viewWindow->requestUpdate();
        }
      });

  return true;
}

void QtCursorEffect::start() {
  qDebug() << "[QtCursorEffect] starting 16 ms render timer";
  timer_.start(16);
}

bool QtCursorEffect::event(QEvent *event) {
  if (event && event->type() == QEvent::UpdateRequest) {
    renderWindow();
    return true;
  }
  return QObject::event(event);
}

bool QtCursorEffect::eventFilter(QObject *watched, QEvent *event) {
  if (watched == m_viewWindow.get() && event &&
      event->type() == QEvent::UpdateRequest) {
    renderWindow();
    return true;
  }
  return QObject::eventFilter(watched, event);
}

void QtCursorEffect::onTick() {
  if (m_mouseProvider) {
    static_cast<QtMouseProvider *>(m_mouseProvider.get())->updateMouseState();
  }

  if (m_html) {
    m_html->update();
    if (m_html->hasNewFrame()) {
      if (debugEnabled())
        qDebug() << "[QtCursorEffect] new frame" << m_html->width() << "x"
                 << m_html->height() << "stride:" << m_html->stride();
      m_viewWindow->requestUpdate();
    }
  }
}

void QtCursorEffect::renderWindow() {
  if (!m_html || !m_backingStore)
    return;

  CursorRenderState state;
  state.pos = m_cursorPoint;
  state.hotspot = QPointF(m_html->hotspotX(), m_html->hotspotY());
  state.visible = !m_isIdleHidden && m_html->isEnabled();

  const QRect full(QPoint(0, 0), m_viewWindow->size());

  if (!shouldRenderCursor(state)) {
    m_backingStore->beginPaint(full);
    QPaintDevice *device = m_backingStore->paintDevice();
    QPainter painter(device);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(full, Qt::transparent);
    painter.end();
    m_backingStore->flush(full);
    return;
  }

  m_backingStore->beginPaint(full);
  QPaintDevice *device = m_backingStore->paintDevice();
  QPainter painter(device);

  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(full, Qt::transparent);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

  const uint8_t *pixels = m_html->pixels();
  if (debugEnabled()) {
    static quint64 renders = 0;
    ++renders;
    if (renders <= 5 || renders % 300 == 0)
      qDebug() << "[QtCursorEffect] render #" << renders << "pos:" << state.pos
               << "visible:" << state.visible << "frame:" << m_html->width()
               << "x" << m_html->height() << "stride:" << m_html->stride()
               << "pixels:" << (pixels != nullptr);
  }
  if (pixels) {
    QImage webFrame(pixels, m_html->width(), m_html->height(), m_html->stride(),
                    QImage::Format_ARGB32_Premultiplied);
    QPointF renderPos = state.pos - state.hotspot;
    painter.drawImage(renderPos, webFrame);
  }

  painter.end();
  m_backingStore->flush(full);
  m_html->clearNewFrame();
}

} // namespace UltralightWebCursorM
