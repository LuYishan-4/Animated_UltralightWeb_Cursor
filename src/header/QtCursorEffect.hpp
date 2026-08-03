#pragma once

#include "MainCursorStaff.hpp" 

namespace UltralightWebCursorM{
class QtCursorEffect : public MainCursorStaff
{
    Q_OBJECT
public:
    QtCursorEffect(QObject* parent = nullptr);
    ~QtCursorEffect() override;

    bool initialize();
    void start();

private slots:
    void onTick();


private:
    QTimer timer_;
};
}