#include "PlatformMouseProvider.hpp"
#include <QGuiApplication>
#include <QCursor>

PlatformMouseProvider::PlatformMouseProvider(QObject* parent) : QObject(parent) {}

bool PlatformMouseProvider::initialize(){
    // Placeholder: no compositor integration here; apps can poll QCursor
    return true;
}

void PlatformMouseProvider::setCallback(Callback callback){
    callback_ = std::move(callback);
    // Simple polling timer could be added by the platform app if needed
}
