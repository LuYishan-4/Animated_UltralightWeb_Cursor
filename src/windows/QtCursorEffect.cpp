#include "../header/QtCursorEffect.hpp"
#include <QCoreApplication>
#include <QDebug>
#include "../lib/SharedCursorRender.hpp"
#include "../header/QtMouseProvider.hpp"
namespace UltralightWebCursorM{
QtCursorEffect::QtCursorEffect(QObject* parent)
    : MainCursorStaff(parent)
{
    connect(&timer_, &QTimer::timeout, this, &QtCursorEffect::onTick);
}

QtCursorEffect::~QtCursorEffect() 
{ 
}

bool QtCursorEffect::initialize(){
    m_html = std::make_unique<UltralightWebCursorM::UltralightHtmlEffect>();
    m_mouseProvider = std::make_unique<QtMouseProvider>();
    if(!m_html) return false;
    if(!m_mouseProvider) return false;
    UltralightWebCursorM::UserConfig::instance()->load();
    std::filesystem::path htmlPath(UserConfigimp.html);
    UltralightWebCursorM::CursorJSON::instance()->load(htmlPath.parent_path().string());
    if(!m_html->initialize(UserConfigimp,CursorJSONImp)){
        qCritical() << "UltralightHtmlEffect initialize failed";
        return false;
    }
    m_mouseProvider->setCallback([this](const UltralightWebCursorM::MousePoint& pt){
        m_html->move(pt.x, pt.y, pt.pressed);
    });
    m_mouseProvider->initialize();
    return true;
}

void QtCursorEffect::start(){
    timer_.start(16);
}

void QtCursorEffect::onTick(){
    if(m_html) m_html->update();
}
}