#pragma once

#include <Ultralight/Ultralight.h>

#include <string>

namespace UltralightWebCursor {

class WebBridge final {
public:
  void setView(ultralight::RefPtr<ultralight::View> view) { view_ = view; }

  void move(int x, int y, bool pressed) {
    if (!view_)
      return;

    std::string script = "if(window.moveCursor){window.moveCursor(";
    script += std::to_string(x);
    script += ',';
    script += std::to_string(y);
    script += pressed ? ",true);}" : ",false);}";
    view_->EvaluateScript(ultralight::String(script.c_str()));
    view_->set_needs_paint(true);
  }

private:
  ultralight::RefPtr<ultralight::View> view_;
};

} // namespace UltralightWebCursor
