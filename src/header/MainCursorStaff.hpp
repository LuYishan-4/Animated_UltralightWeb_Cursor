#pragma once

#include <memory>
#include <filesystem>
#include <string>
#include <QPointF>
#include <QRectF>       
#include <QObject>   
#include <memory>
#include <filesystem>
#include "UltralightHtmlEffect.hpp"
#include "MouseProvider.hpp"
#include "../config/UserConfig.hpp"
#include "../config/CursorJSON.hpp"
#include "../lib/BlackList/BlacklistManager.hpp"
#include "../lib/CrashReport/CrashReport.hpp"


namespace UltralightWebCursorM {

class MainCursorStaff{


public:
    MainCursorStaff() = default; 
    virtual ~MainCursorStaff() = default;

    bool isWindowBlacklisted(const std::string& windowClass) const {
        return m_blacklist.contains(windowClass);
    }

    QRectF getCursorRect(const QPointF& basePos) const {
        if (!m_html) return QRectF();
        qreal w = m_html->width();
        qreal h = m_html->height();
         return QRectF(basePos.x() - m_html->hotspotX(), basePos.y() - m_html->hotspotY(), w, h);
    }

public:
    virtual void enable() {
        if (m_html) m_html->setEnabled(true);
    }

    virtual void disable() {
        if (m_html) m_html->setEnabled(false);
    }

    virtual void reloadHtml() {
        UltralightWebCursorM::UserConfig::instance()->load();
        UltralightWebCursorM::CursorJSON::instance()->load(UserConfigimp.html);
        if (m_html) m_html->reload(UserConfigimp, CursorJSONImp);
    }

protected:
    template <typename MouseProviderType>
    bool initializeCore() {
        UltralightWebCursorM::CrashHandler::registerHandler();
        try {
            UltralightWebCursorM::UserConfig::instance()->load();
            UltralightWebCursorM::CursorJSON::instance()->load(UserConfigimp.html);
            
            m_html = std::make_unique<UltralightWebCursorM::UltralightHtmlEffect>();
            m_mouseProvider = std::make_unique<MouseProviderType>();
            m_mouseProvider->initialize();

            if (!m_html || !m_mouseProvider) {
                 qDebug() << "[UltralightCursorEffect] deeeeeeeeee";
                return false;
            }
           

            if (!m_html->initialize(UserConfigimp, CursorJSONImp)) {
                 qDebug() << "[UltralightCursorEffect] dccce";
                m_html.reset();
                m_mouseProvider.reset();
                return false;
            }
            m_blacklist.setBlacklist(UltralightWebCursorM::UserConfig::instance()->getBlacklist());
            return true;
        } catch (...) {
            return false;
        }
    }


    virtual bool checkFullScreen() const { return false; }
    bool m_isIdleHidden = false;
    std::unique_ptr<UltralightWebCursorM::UltralightHtmlEffect> m_html;
    std::unique_ptr<UltralightWebCursorM::IMouseProvider> m_mouseProvider;
    UltralightWebCursorM::BlacklistManager m_blacklist;

    QPointF m_cursorPoint;
};

} // namespace UltralightWebCursorM
