#pragma once

#include <QtGlobal>

#include <functional>
#include <utility>

namespace UltralightWebCursor {

struct MouseState {
  qreal x = 0;
  qreal y = 0;
  bool pressed = false;
};

class MouseProvider final {
public:
  using Callback = std::function<void(const MouseState &)>;

  MouseProvider() = default;
  ~MouseProvider();

  bool initialize();
  void setCallback(Callback callback) { callback_ = std::move(callback); }
  void update();

private:
  Callback callback_;
  void *nativeDisplay_ = nullptr;
};

} // namespace UltralightWebCursor
