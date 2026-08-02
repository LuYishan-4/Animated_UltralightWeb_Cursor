#pragma once

#include <memory>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include "UltralightHtmlEffect.hpp"
#include "MouseProvider.hpp"
#include "../config/UserConfig.hpp"
#include "../config/CursorJSON.hpp"
#include "../lib/BlackList/BlacklistManager.hpp"

namespace UltralightWebCursorM {

class GLTexture;
class EffectWindow; 

class MainCursorStaff : public QObject
{
    Q_OBJECT

public:
    MainCursorStaff(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~MainCursorStaff() {}
    virtual bool isBlacklisted() const { return false; }

public Q_SLOTS:
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual void reloadHtml() = 0;

protected:
    virtual void hideCursor() {}
    virtual void showCursor() {}
    virtual GLTexture* ensureCursorTexture();
    virtual bool checkFullScreen() const { return false; }
    virtual void slotWindowStateChanged(EffectWindow *w) {}
    QTimer *m_idleTimer = nullptr;
    bool m_isIdleHidden = false;
    std::unique_ptr<UltralightWebCursorM::UltralightHtmlEffect> m_html;
    std::unique_ptr<UltralightWebCursorM::IMouseProvider> m_mouseProvider;
    UltralightWebCursorM::BlacklistManager m_blacklist;
    std::unique_ptr<GLTexture> m_cursorTexture;

    QPointF m_cursorPoint;
};

} // namespace UltralightWebCursorM
