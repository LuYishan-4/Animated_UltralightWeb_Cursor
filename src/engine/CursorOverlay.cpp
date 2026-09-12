#include "CursorOverlay.hpp"

#include "../config/ThemeMetadata.hpp"
#include "../config/UserConfig.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QScreen>
#include <QSurfaceFormat>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>
#undef Bool
#undef Cursor
#undef None
#undef Status
#undef Window
#endif

namespace UltralightWebCursor {
namespace {

bool debugEnabled() {
  static const bool enabled =
      qEnvironmentVariableIsSet("ULTRALIGHTWEBCURSOR_DEBUG");
  return enabled;
}

QRect virtualDesktopGeometry() {
  QRect geometry;
  for (QScreen *screen : QGuiApplication::screens())
    geometry = geometry.united(screen->geometry());
  return geometry;
}

} // namespace

CursorOverlay::CursorOverlay(QObject *parent) : QObject(parent) {
  connect(&timer_, &QTimer::timeout, this, &CursorOverlay::onTick);

  window_ = std::make_unique<QWindow>();
  window_->setFlags(Qt::FramelessWindowHint | Qt::WindowTransparentForInput |
                    Qt::WindowStaysOnTopHint | Qt::ToolTip);

  QSurfaceFormat format = window_->format();
  format.setAlphaBufferSize(8);
  window_->setFormat(format);
  updateOverlayGeometry();
  window_->installEventFilter(this);
  window_->show();

  backingStore_ = std::make_unique<QBackingStore>(window_.get());
  backingStore_->resize(window_->size());

  connect(qGuiApp, &QGuiApplication::screenAdded, this,
          [this](QScreen *screen) {
            connect(screen, &QScreen::geometryChanged, this,
                    &CursorOverlay::updateOverlayGeometry);
            updateOverlayGeometry();
          });
  connect(qGuiApp, &QGuiApplication::screenRemoved, this,
          &CursorOverlay::updateOverlayGeometry);
  for (QScreen *screen : QGuiApplication::screens()) {
    connect(screen, &QScreen::geometryChanged, this,
            &CursorOverlay::updateOverlayGeometry);
  }
}

CursorOverlay::~CursorOverlay() = default;

bool CursorOverlay::configureNativeWindow() {
#ifdef Q_OS_WIN
  const HWND handle = reinterpret_cast<HWND>(window_->winId());
  const LONG_PTR style = GetWindowLongPtr(handle, GWL_EXSTYLE);
  SetLastError(ERROR_SUCCESS);
  const LONG_PTR result = SetWindowLongPtr(
      handle, GWL_EXSTYLE,
      style | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE);
  return result != 0 || GetLastError() == ERROR_SUCCESS;
#elif defined(Q_OS_LINUX)
  if (QGuiApplication::platformName() != QStringLiteral("xcb")) {
    qCritical() << "[Overlay] Linux engine requires the xcb Qt platform "
                   "plugin (Xorg or XWayland).";
    return false;
  }

  Display *display = XOpenDisplay(nullptr);
  if (!display) {
    qCritical() << "[Overlay] Could not open X11 display";
    return false;
  }

  const ::Window id = static_cast<::Window>(window_->winId());
  XserverRegion region = XFixesCreateRegion(display, nullptr, 0);
  XFixesSetWindowShapeRegion(display, id, ShapeInput, 0, 0, region);
  XFixesDestroyRegion(display, region);
  XFlush(display);
  XCloseDisplay(display);
  return true;
#else
  return false;
#endif
}

bool CursorOverlay::initialize() {
  if (!configureNativeWindow())
    return false;

  QString error;
  UserConfig &config = UserConfig::instance();
  if (!config.load(&error)) {
    qCritical() << "[Overlay] Configuration failed:" << error;
    return false;
  }

  ThemeMetadata metadata;
  const QString themeDirectory = QFileInfo(config.values().htmlPath).path();
  if (!ThemeMetadataReader::load(themeDirectory, metadata, &error)) {
    qCritical() << "[Overlay] Invalid theme:" << error;
    return false;
  }

  renderer_ = std::make_unique<HtmlRenderer>();
  if (!renderer_->initialize(config.values(), metadata))
    return false;

  mouseProvider_ = std::make_unique<MouseProvider>();
  if (!mouseProvider_->initialize())
    return false;

  mouseProvider_->setCallback([this](const MouseState &state) {
    cursorPosition_ = QPointF(state.x, state.y);
    if (renderer_) {
      renderer_->move(qRound(state.x), qRound(state.y), state.pressed);
      window_->requestUpdate();
    }
  });

  qInfo() << "[Overlay] Ready | platform:" << QGuiApplication::platformName()
          << "desktop:" << window_->geometry()
          << "theme:" << config.currentTheme();
  return true;
}

void CursorOverlay::start() {
  timer_.start(16);
  qInfo() << "[Overlay] Render loop started";
}

void CursorOverlay::reloadConfig() {
  QString error;
  UserConfig &config = UserConfig::instance();
  if (!config.load(&error)) {
    qWarning() << "[Overlay] Reload failed:" << error;
    return;
  }

  ThemeMetadata metadata;
  if (!ThemeMetadataReader::load(QFileInfo(config.values().htmlPath).path(),
                                 metadata, &error)) {
    qWarning() << "[Overlay] Theme reload failed:" << error;
    return;
  }

  if (renderer_)
    renderer_->reload(config.values(), metadata);
}

void CursorOverlay::switchTheme(const QString &themeName) {
  QString error;
  if (!UserConfig::instance().setTheme(themeName, &error)) {
    qWarning() << "[Overlay] Could not select theme:" << error;
    return;
  }
  reloadConfig();
}

void CursorOverlay::setEnabled(bool enabled) {
  QString error;
  if (!UserConfig::instance().setEnabled(enabled, &error)) {
    qWarning() << "[Overlay] Could not save enabled state:" << error;
    return;
  }
  if (renderer_)
    renderer_->setEnabled(enabled);
  window_->requestUpdate();
}

bool CursorOverlay::eventFilter(QObject *watched, QEvent *event) {
  if (watched == window_.get() && event &&
      event->type() == QEvent::UpdateRequest) {
    render();
    return true;
  }
  return QObject::eventFilter(watched, event);
}

void CursorOverlay::onTick() {
  if (mouseProvider_)
    mouseProvider_->update();

  if (renderer_) {
    renderer_->update();
    if (renderer_->hasNewFrame())
      window_->requestUpdate();
  }
}

void CursorOverlay::updateOverlayGeometry() {
  if (!window_)
    return;
  const QRect geometry = virtualDesktopGeometry();
  if (geometry.isValid() && window_->geometry() != geometry)
    window_->setGeometry(geometry);
  if (backingStore_)
    backingStore_->resize(window_->size());
}

void CursorOverlay::render() {
  if (!renderer_ || !backingStore_ || !window_)
    return;

  const QRect area(QPoint(0, 0), window_->size());
  backingStore_->beginPaint(area);
  QPainter painter(backingStore_->paintDevice());
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(area, Qt::transparent);

  const uint8_t *pixels = renderer_->pixels();
  if (renderer_->isEnabled() && pixels) {
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    const QImage frame(pixels, renderer_->width(), renderer_->height(),
                       renderer_->stride(),
                       QImage::Format_ARGB32_Premultiplied);
    const QPointF localPosition =
        cursorPosition_ - window_->geometry().topLeft() -
        QPointF(renderer_->hotspotX(), renderer_->hotspotY());
    painter.drawImage(localPosition, frame);
  }
  painter.end();
  backingStore_->flush(area);
  renderer_->clearNewFrame();

  if (debugEnabled()) {
    static quint64 frameNumber = 0;
    ++frameNumber;
    if (frameNumber <= 5 || frameNumber % 300 == 0) {
      qDebug() << "[Overlay] frame" << frameNumber
               << "cursor:" << cursorPosition_
               << "pixels:" << (pixels != nullptr)
               << "enabled:" << renderer_->isEnabled();
    }
  }
}

} // namespace UltralightWebCursor
