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
protected:
    template <typename MouseProviderType>
    bool initializeCore() {
        static bool isPermanentlyDisabled = false;
        if (isPermanentlyDisabled)return false; 
        UltralightWebCursorM::CrashHandler::registerHandler();
        try {
            UltralightWebCursorM::UserConfig::instance()->load();
            UltralightWebCursorM::CursorJSON::instance()->load(UserConfigimp.html);
            
            m_html = std::make_unique<UltralightWebCursorM::UltralightHtmlEffect>();
            m_mouseProvider = std::make_unique<MouseProviderType>();
            m_mouseProvider->initialize();

            if (!m_html || !m_mouseProvider) {
                 qDebug() << "[UltralightCursorEffect] deeeeeeeeee";
                  isPermanentlyDisabled = true;
                return false;
            }
           

            if (!m_html->initialize(UserConfigimp, CursorJSONImp)) {
                 qDebug() << "[UltralightCursorEffect] dccce";
                m_html.reset();
                m_mouseProvider.reset();
                 isPermanentlyDisabled = true;
                return false;
            }
            m_blacklist.setBlacklist(UltralightWebCursorM::UserConfig::instance()->getBlacklist());
            return true;
        } catch (const std::exception& e) {
            isPermanentlyDisabled = true;
            return false;
        } catch (...) {
            isPermanentlyDisabled = true;
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
