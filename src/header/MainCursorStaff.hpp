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

namespace UltralightWebCursorM {

class MainCursorStaff{


public:
    MainCursorStaff() = default; 
    virtual ~MainCursorStaff() {
        m_mouseProvider.reset();
        m_html.reset();
    }

    bool isWindowBlacklisted(const std::string& windowClass) const {
        return m_blacklist.contains(windowClass);
    }

    QRectF getCursorRect(const QPointF& basePos) const {
        if (!m_html) return QRectF();
        qreal w = m_html->width();
        qreal h = m_html->height();
        return QRectF(basePos.x() - w / 2.0, basePos.y() - h / 2.0, w, h);
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
        std::filesystem::path p(UserConfigimp.html);
        std::string path = p.parent_path().string();
        UltralightWebCursorM::CursorJSON::instance()->load(path);
        if (m_html) m_html->reload(UserConfigimp, CursorJSONImp);
    }

protected:
    template <typename MouseProviderType>
    bool initializeCore() {
        try {
            UltralightWebCursorM::UserConfig::instance()->load();
            std::filesystem::path p(UserConfigimp.html);
            std::string path = p.parent_path().string();
            UltralightWebCursorM::CursorJSON::instance()->load(path);
            
            m_html = std::make_unique<UltralightWebCursorM::UltralightHtmlEffect>();
            m_mouseProvider = std::make_unique<MouseProviderType>();
            
            if (!m_html || !m_mouseProvider) return false;

            if (!m_html->initialize(UserConfigimp, CursorJSONImp)) {
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
