#include "PlatformMouseProvider.hpp"
#include <QDebug>

PlatformMouseProvider::PlatformMouseProvider(QObject* parent) : QObject(parent) {}

bool PlatformMouseProvider::initialize(){
    // For wlroots-based compositors integration is platform specific; placeholder
    return true;
}

void PlatformMouseProvider::setCallback(Callback callback){
    callback_ = std::move(callback);
}
