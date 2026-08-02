#pragma once

#include <memory>
#include <QTimer>
#include "UltralightHtmlEffect.hpp"
#include "QtMouseProvider.hpp"

class QtCursorEffect : public QObject
{
    Q_OBJECT
public:
    QtCursorEffect(QObject* parent = nullptr);
    ~QtCursorEffect();

    bool initialize();
    void start();

private slots:
    void onTick();

private:
    std::unique_ptr<UltralightWebCursorM::UltralightHtmlEffect> html_;
    std::unique_ptr<QtMouseProvider> mouseProvider_;
    QTimer timer_;
};
