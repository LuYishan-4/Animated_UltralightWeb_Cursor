#pragma once

#include "MouseProvider.hpp"
#include <QObject>

class QtMouseProvider : public QObject, public UltralightWebCursorM::IMouseProvider
{
    Q_OBJECT
public:
    explicit QtMouseProvider(QObject* parent = nullptr);
    
    bool initialize() override;
    void setCallback(Callback callback) override;
    void updateMouseState();

private Q_SLOTS:
    void onTimer();

private:
    Callback callback_;
};
