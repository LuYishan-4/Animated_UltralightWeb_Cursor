#include "PlatformMouseProvider.hpp"
#include <QDebug>

PlatformMouseProvider::PlatformMouseProvider(QObject* parent) : QObject(parent) {}

bool PlatformMouseProvider::initialize(){
    // Windows-specific input integration placeholder
    return true;
}

void PlatformMouseProvider::setCallback(Callback callback){
    callback_ = std::move(callback);
}
