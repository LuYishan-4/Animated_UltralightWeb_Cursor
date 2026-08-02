#include "PlatformMouseProvider.hpp"
#include <QDebug>

PlatformMouseProvider::PlatformMouseProvider(QObject* parent) : QObject(parent) {}

bool PlatformMouseProvider::initialize(){
    // Placeholder for Niri-specific input integration
    return true;
}

void PlatformMouseProvider::setCallback(Callback callback){
    callback_ = std::move(callback);
}
