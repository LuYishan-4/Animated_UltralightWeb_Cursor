#pragma once

#include "../config/CursorJSON.hpp"
#include "../config/UserConfig.hpp"
#include "../lib/WebCall/WebCall.hpp"
#include <AppCore/AppCore.h>
#include <Ultralight/Ultralight.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace UltralightWebCursorM {

class UltralightHtmlEffect {

public:
  UltralightHtmlEffect();

  ~UltralightHtmlEffect();

  bool initialize(const ConfigValues &uconfig, const JSONConf &data);

  bool load(const std::string &path);

  void update();
  bool ensureInitialized();

  void move(int x, int y, bool pressed);

  ultralight::View *view() const;
  void reload(const ConfigValues &uconfig, const JSONConf &data);

  bool resize(const int &width, const int &height);

  static ultralight::RefPtr<ultralight::Renderer> sharedRenderer();

  const uint8_t *pixels() const;
  unsigned int textureId() const;

  int width() const { return html_value_.width_; }

  int height() const { return html_value_.height_; }

  int stride() const { return html_value_.stride_; }

  int hotspotX() const {
    // return html_value_.hotspot_x_;
    return html_value_.width_ / 2;
  }

  int hotspotY() const {
    // return html_value_.hotspot_y_;
    return html_value_.height_ / 2;
  }

  void setEnabled(bool enabled);

  bool isEnabled() const;

  bool hasNewFrame() const;

  void clearNewFrame();

private:
  struct Html_Value {
    int width_ = 128;
    int height_ = 128;
    int stride_ = 0;
    int minwidth = 128;
    int minheight = 128;
    int hotspot_x_ = 64;
    int hotspot_y_ = 64;
    std::string m_permanentSdkPath;
    std::filesystem::path html_path_;
  };

  Html_Value html_value_;

  ultralight::RefPtr<ultralight::Renderer> renderer_;
  std::shared_ptr<WebCall> webcall;

  ultralight::RefPtr<ultralight::View> view_;

  std::unique_ptr<ultralight::LoadListener> listener_;

  bool is_loaded_ = false;

  bool enabled_ = true;

  bool new_frame_ = false;

  bool renderer_initialized_ = false;

  inline static bool platform_initialized_ = false;

  std::vector<uint8_t> pixel_buffer_;

  std::filesystem::file_time_type html_time_;
};

} // namespace UltralightWebCursorM
