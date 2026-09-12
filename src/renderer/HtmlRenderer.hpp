#pragma once

#include "../config/ThemeMetadata.hpp"
#include "../config/UserConfig.hpp"
#include "WebBridge.hpp"

#include <AppCore/AppCore.h>
#include <Ultralight/Ultralight.h>

#include <QString>

#include <cstdint>
#include <memory>
#include <vector>

namespace UltralightWebCursor {

class HtmlRenderer final {
public:
  HtmlRenderer() = default;
  ~HtmlRenderer();

  bool initialize(const ConfigValues &config, const ThemeMetadata &metadata);
  void reload(const ConfigValues &config, const ThemeMetadata &metadata);
  void update();
  void move(int x, int y, bool pressed);

  const uint8_t *pixels() const;
  int width() const { return width_; }
  int height() const { return height_; }
  int stride() const { return stride_; }
  int hotspotX() const { return hotspotX_; }
  int hotspotY() const { return hotspotY_; }

  void setEnabled(bool enabled) { enabled_ = enabled; }
  bool isEnabled() const { return enabled_; }
  bool hasNewFrame() const { return newFrame_; }
  void clearNewFrame() { newFrame_ = false; }

private:
  bool ensureView();
  bool load(const QString &path);
  static ultralight::RefPtr<ultralight::Renderer> sharedRenderer();

  QString dataRoot_;
  QString htmlPath_;
  int width_ = 128;
  int height_ = 128;
  int stride_ = 0;
  int hotspotX_ = 64;
  int hotspotY_ = 64;

  ultralight::RefPtr<ultralight::Renderer> renderer_;
  ultralight::RefPtr<ultralight::View> view_;
  std::unique_ptr<ultralight::LoadListener> loadListener_;
  WebBridge webBridge_;
  std::vector<uint8_t> pixelBuffer_;

  bool loaded_ = false;
  bool enabled_ = true;
  bool newFrame_ = false;
  bool viewInitialized_ = false;

  inline static bool platformInitialized_ = false;
};

} // namespace UltralightWebCursor
