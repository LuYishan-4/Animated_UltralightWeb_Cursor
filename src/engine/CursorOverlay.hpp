#pragma once

#include "../renderer/HtmlRenderer.hpp"
#include "MouseProvider.hpp"

#include <QBackingStore>
#include <QEvent>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QWindow>

#include <memory>

namespace UltralightWebCursor {

class CursorOverlay final : public QObject {
  Q_OBJECT

public:
  explicit CursorOverlay(QObject *parent = nullptr);
  ~CursorOverlay() override;

  bool initialize();
  void start();

  void reloadConfig();
  void switchTheme(const QString &themeName);
  void setEnabled(bool enabled);

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
  void onTick();
  void updateOverlayGeometry();

private:
  bool configureNativeWindow();
  void render();

  QTimer timer_;
  std::unique_ptr<QWindow> window_;
  std::unique_ptr<QBackingStore> backingStore_;
  std::unique_ptr<HtmlRenderer> renderer_;
  std::unique_ptr<MouseProvider> mouseProvider_;
  QPointF cursorPosition_;
};

} // namespace UltralightWebCursor
