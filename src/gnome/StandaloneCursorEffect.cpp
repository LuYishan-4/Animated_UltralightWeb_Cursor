#include "StandaloneCursorEffect.hpp"
#include <QCoreApplication>
#include <QDebug>

StandaloneCursorEffect::StandaloneCursorEffect(QObject* parent)
    : QObject(parent)
{
    connect(&timer_, &QTimer::timeout, this, &StandaloneCursorEffect::onTick);
}

StandaloneCursorEffect::~StandaloneCursorEffect(){ }

bool StandaloneCursorEffect::initialize(){
    html_ = std::make_unique<UltralightWebCursorM::UltralightHtmlEffect>();
    mouseProvider_ = std::make_unique<PlatformMouseProvider>();
    if(!html_) return false;
    if(!mouseProvider_) return false;
    // load config
    UltralightWebCursorM::UserConfig::instance()->load();
    std::filesystem::path htmlPath(UltralightWebCursorM::UserConfigimp.html);
    UltralightWebCursorM::CursorJSON::instance()->load(htmlPath.parent_path().string());

    if(!html_->initialize(UltralightWebCursorM::UserConfigimp, UltralightWebCursorM::CursorJSONImp)){
        qCritical() << "UltralightHtmlEffect initialize failed";
        return false;
    }

    mouseProvider_->setCallback([this](const UltralightWebCursorM::MousePoint& pt){
        html_->move(pt.x, pt.y, pt.pressed);
    });
    mouseProvider_->initialize();
    return true;
}

void StandaloneCursorEffect::start(){
    timer_.start(16);
}

void StandaloneCursorEffect::onTick(){
    if(html_) html_->update();
}
