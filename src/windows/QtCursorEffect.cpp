#include "../header/QtCursorEffect.hpp"
#include <QCoreApplication>
#include <QDebug>
#include "../lib/SharedCursorRender.hpp"

QtCursorEffect::QtCursorEffect(QObject* parent)
    : QObject(parent)
{
    connect(&timer_, &QTimer::timeout, this, &QtCursorEffect::onTick);
}

QtCursorEffect::~QtCursorEffect(){ }

bool QtCursorEffect::initialize(){
    html_ = std::make_unique<UltralightWebCursorM::UltralightHtmlEffect>();
    mouseProvider_ = std::make_unique<QtMouseProvider>();
    if(!html_) return false;
    if(!mouseProvider_) return false;
    UltralightWebCursorM::UserConfig::instance()->load();
    std::filesystem::path htmlPath(UserConfigimp.html);
    UltralightWebCursorM::CursorJSON::instance()->load(htmlPath.parent_path().string());

    if(!html_->initialize(UserConfigimp,CursorJSONImp)){
        qCritical() << "UltralightHtmlEffect initialize failed";
        return false;
    }

    mouseProvider_->setCallback([this](const UltralightWebCursorM::MousePoint& pt){
        html_->move(pt.x, pt.y, pt.pressed);
    });
    mouseProvider_->initialize();
    return true;
}

void QtCursorEffect::start(){
    timer_.start(16);
}

void QtCursorEffect::onTick(){
    if(html_) html_->update();
}
