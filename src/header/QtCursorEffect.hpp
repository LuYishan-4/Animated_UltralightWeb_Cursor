#pragma once

#include <memory>
#include <QTimer>
#include <QWindow>
#include <QBackingStore>
#include <QEvent>
#include "MainCursorStaff.hpp" 
#if defined(__linux__) || defined(Q_OS_LINUX)
#  undef Event
#  undef Cursor
#  undef Status
#  undef Bool
#endif
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
        if (event && event->type() == QEvent::UpdateRequest) {
            renderWindow();
            return true;
        }
        return MainCursorStaff::event(event);
    }

private Q_SLOTS:
    void onTick();

private:
    QTimer timer_;
    

    std::unique_ptr<QWindow> m_viewWindow;
    std::unique_ptr<QBackingStore> m_backingStore;
};

} // namespace UltralightWebCursorM
