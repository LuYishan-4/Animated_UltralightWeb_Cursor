#pragma once

#include <memory>
#include <QTimer>
#include "../header/UltralightHtmlEffect.hpp"
#include "PlatformMouseProvider.hpp"

class StandaloneCursorEffect : public QObject
{
    Q_OBJECT
public:
    StandaloneCursorEffect(QObject* parent = nullptr);
    ~StandaloneCursorEffect();

    bool initialize();
    void start();

private slots:
    void onTick();

private:
    std::unique_ptr<UltralightWebCursorM::UltralightHtmlEffect> html_;
    std::unique_ptr<PlatformMouseProvider> mouseProvider_;
    QTimer timer_;
};
