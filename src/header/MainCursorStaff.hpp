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




class MainCursorStaff : public QObject
{
    Q_OBJECT

public:
    MainCursorStaff(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~MainCursorStaff() {}

virtual bool isBlacklisted() const {return false;}

public Q_SLOTS:
    virtual void enable() {}
    virtual void disable() {}
    virtual void reloadHtml() {}

protected:
    virtual void hideCursor() {}
    virtual void showCursor() {}
    
    virtual bool checkFullScreen() const { return false; }


    QTimer *m_idleTimer = nullptr;
    bool m_isIdleHidden = false;
    std::unique_ptr<UltralightWebCursorM::UltralightHtmlEffect> m_html;
    std::unique_ptr<UltralightWebCursorM::IMouseProvider> m_mouseProvider;
    UltralightWebCursorM::BlacklistManager m_blacklist;


    QPointF m_cursorPoint;
};

} // namespace UltralightWebCursorM
