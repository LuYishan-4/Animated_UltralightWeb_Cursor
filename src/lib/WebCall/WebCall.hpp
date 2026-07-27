#pragma once

#include <string>
#include <Ultralight/Ultralight.h>

class WebCall {
public:
    ultralight::RefPtr<ultralight::View> view_; 
    inline void move(int x, int y, bool pressed) {
        if (!view_) return;
        
        std::string x_str = std::to_string(x);
        std::string y_str = std::to_string(y);
        const char* pressed_str = pressed ? "true" : "false";
        
        std::string js;
        js.reserve(60 + x_str.length() + y_str.length()); 
        
        js.append("if(window.moveCursor){window.moveCursor(")
          .append(x_str)
          .append(",")
          .append(y_str)
          .append(",")
          .append(pressed_str)
          .append(");}");
        view_->EvaluateScript(ultralight::String(js.c_str()));
        view_->set_needs_paint(true);
    }
};
