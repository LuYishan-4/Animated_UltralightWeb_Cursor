#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QVariant>
#include <QList>
#include <QPointF>
#include <QRectF>
#include "../header/QtCursorEffect.hpp"
#include "SharedCursorRender.hpp"
#include "../header/QtMouseProvider.hpp"
#if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
#elif defined(__linux__) || defined(Q_OS_LINUX)
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <X11/extensions/shape.h> 
#  include <X11/extensions/Xfixes.h>
#  undef Status
#  undef Bool
#  undef None
#  undef KeyPress
#  undef KeyRelease
#  undef FocusIn
#  undef FocusOut
#  undef FontChange
#  undef Cursor
#  undef Screen
#  undef Window
#  undef Event
#endif 


namespace UltralightWebCursorM {

QtCursorEffect::QtCursorEffect(QObject* parent)
    : MainCursorStaff(parent)
{
    connect(&timer_, &QTimer::timeout, this, &QtCursorEffect::onTick);

    m_viewWindow = std::make_unique<QWindow>();
    m_viewWindow->setFlags(Qt::FramelessWindowHint |  
                           Qt::WindowTransparentForInput | 
                           Qt::WindowStaysOnTopHint |  
                           Qt::ToolTip);
    
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        m_viewWindow->setGeometry(screen->geometry());
    }
    
    m_viewWindow->show();
#if defined(_WIN32) || defined(_WIN64)
    HWND hwnd = (HWND)m_viewWindow->winId();
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
#elif defined(__linux__) || defined(Q_OS_LINUX)
    Display* dpy = XOpenDisplay(nullptr);
    if (dpy) {
        Window winId = (Window)m_viewWindow->winId();
        XserverRegion region = XFixesCreateRegion(dpy, nullptr, 0);
        
        XFixesSetWindowShapeRegion(dpy, winId, ShapeInput, 0, 0, region);
        
        XFixesDestroyRegion(dpy, region);
        
        XFlush(dpy);
        XCloseDisplay(dpy);
        qDebug() << "[QtCursorEffect] X11 Input Shape Region bypassed successfully.";
    }
#endif

    m_backingStore = std::make_unique<QBackingStore>(m_viewWindow.get());
    m_backingStore->resize(m_viewWindow->size());
}

QtCursorEffect::~QtCursorEffect() 
{
}

bool QtCursorEffect::initialize() {
    if (!initializeCore<QtMouseProvider>()) {
        qCritical() << "UltralightCore or QtMouseProvider initialize failed";
        return false;
    }

    m_mouseProvider->setCallback([this](const UltralightWebCursorM::MousePoint& pt) {
        if (m_html) {
            m_cursorPoint = QPointF(pt.x, pt.y);
            m_html->move(pt.x, pt.y, pt.pressed);
            m_viewWindow->requestUpdate(); 
        }
    });

    return true;
}

void QtCursorEffect::start() {
    timer_.start(16);
}

void QtCursorEffect::onTick() {
    if (m_mouseProvider) {
        static_cast<QtMouseProvider*>(m_mouseProvider.get())->updateMouseState();
    }

    if (m_html) {
        m_html->update();
        if (m_html->hasNewFrame()) {
            m_viewWindow->requestUpdate();
        }
    }
}

void QtCursorEffect::renderWindow() {
    if (!m_html || !m_backingStore) return;
    
    CursorRenderState state;
    state.pos = m_cursorPoint;

    state.hotspot = QPointF(m_html->hotspotX(), m_html->hotspotY()); 
    
    state.visible = !m_isIdleHidden; 

    if (!shouldRenderCursor(state)) {
        m_backingStore->beginPaint(QRect(QPoint(0, 0), m_viewWindow->size()));
        QPaintDevice *device = m_backingStore->paintDevice();
        QPainter painter(device);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(QRect(QPoint(0, 0), m_viewWindow->size()), Qt::transparent);
        painter.end();
        m_backingStore->flush(QRect(QPoint(0, 0), m_viewWindow->size()));
        return;
    }
    m_backingStore->beginPaint(QRect(QPoint(0, 0), m_viewWindow->size()));
    QPaintDevice *device = m_backingStore->paintDevice();
    QPainter painter(device);
    
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(QRect(QPoint(0, 0), m_viewWindow->size()), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    const uint8_t* pixels = m_html->pixels();
    if (pixels) {
        QImage webFrame(
            pixels,
            m_html->width(),
            m_html->height(),
            m_html->stride(),
            QImage::Format_ARGB32_Premultiplied
        );
        QPointF renderPos = state.pos - state.hotspot;
        painter.drawImage(renderPos, webFrame);
    }

    painter.end();
    m_backingStore->flush(QRect(QPoint(0, 0), m_viewWindow->size()));
    m_html->clearNewFrame();
}

} // namespace UltralightWebCursorM
