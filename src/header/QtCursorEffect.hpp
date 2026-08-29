#pragma once
#include "MainCursorStaff.hpp"
#include <QBackingStore>
#include <QEvent>
#include <QTimer>
#include <QWindow>
#include <memory>
#if defined(__linux__) || defined(Q_OS_LINUX)
#undef Event
#undef Cursor
#undef Status
#undef Bool
#endif
namespace UltralightWebCursorM {
class QtCursorEffect : public MainCursorStaff {
  Q_OBJECT
public:
  QtCursorEffect(QObject *parent = nullptr);
  ~QtCursorEffect() override;
  bool initialize();
  void start();
  void renderWindow();

protected:
private Q_SLOTS:
  void onTick();

private:
  QTimer timer_;
  std::unique_ptr<QWindow> m_viewWindow;
  std::unique_ptr<QBackingStore> m_backingStore;
  bool m_focusApplied = false;
};
} // namespace UltralightWebCursorM
