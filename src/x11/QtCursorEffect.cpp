#include "../header/QtCursorEffect.hpp"
#include <QCoreApplication>
#include <QDebug>
#include "../lib/SharedCursorRender.hpp"
#include "../header/QtMouseProvider.hpp"

namespace UltralightWebCursorM {

QtCursorEffect::QtCursorEffect(QObject* parent)
    : MainCursorStaff(parent){
    connect(&timer_, &QTimer::timeout, this, &QtCursorEffect::onTick);
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
        }
    });

    m_mouseProvider->initialize();
    return true;
}

void QtCursorEffect::start() {
    timer_.start(16);
}

void QtCursorEffect::onTick() {
    if (m_html) {
        m_html->update();
    }
}

} // namespace UltralightWebCursorM
