#pragma once

#include "MouseProvider.hpp"
#include <QObject>
#include <QTimer>

class QtMouseProvider : public QObject, public UltralightWebCursorM::IMouseProvider
{
    Q_OBJECT
public:
    explicit QtMouseProvider(QObject* parent = nullptr);
    bool initialize() override;
    void setCallback(Callback callback) override;

private slots:
    void onTimer();

private:
    Callback callback_;
    QTimer timer_;
};
