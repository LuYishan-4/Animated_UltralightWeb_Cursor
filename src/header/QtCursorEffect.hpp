#pragma once

#include "MainCursorStaff.hpp" 
#include <QTimer>
#include <QWindow>
#include <QBackingStore>
#include <QEvent>

namespace UltralightWebCursorM {

class QtCursorEffect : public MainCursorStaff
{
    Q_OBJECT
public:
    QtCursorEffect(QObject* parent = nullptr);
    ~QtCursorEffect() override;

    bool initialize();
    void start();

    void renderWindow();

protected:
    bool event(QEvent *event) override {
        if (event->type() == QEvent::UpdateRequest) {
            renderWindow();
            return true;
        }
        return MainCursorStaff::event(event);
    }

private slots:
    void onTick();

private:
    QTimer timer_;
    
    std::unique_ptr<QWindow> m_viewWindow;
    std::unique_ptr<QBackingStore> m_backingStore;
};

} // namespace UltralightWebCursorM
