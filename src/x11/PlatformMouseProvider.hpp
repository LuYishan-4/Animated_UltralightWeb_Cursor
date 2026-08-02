#pragma once

#include "../header/MouseProvider.hpp"
#include <QObject>
#include <QTimer>

class PlatformMouseProvider : public QObject, public UltralightWebCursorM::IMouseProvider
{
    Q_OBJECT
public:
    explicit PlatformMouseProvider(QObject* parent = nullptr);
    bool initialize() override;
    void setCallback(Callback callback) override;

private slots:
    void onTimer();

private:
    Callback callback_;
    QTimer timer_;
};
