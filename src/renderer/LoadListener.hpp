#pragma once

#include <Ultralight/Ultralight.h>

#include <QDebug>

namespace UltralightWebCursor {

class LoadListener final : public ultralight::LoadListener {
public:
  explicit LoadListener(bool *loaded) : loaded_(loaded) {}

  void OnFinishLoading(ultralight::View *, uint64_t, bool mainFrame,
                       const ultralight::String &url) override {
    if (!mainFrame)
      return;
    if (loaded_)
      *loaded_ = true;
    qInfo() << "[Renderer] Theme loaded:" << url.utf8().data();
  }

  void OnFailLoading(ultralight::View *, uint64_t, bool mainFrame,
                     const ultralight::String &url,
                     const ultralight::String &description,
                     const ultralight::String &, int errorCode) override {
    if (!mainFrame)
      return;
    if (loaded_)
      *loaded_ = false;
    qCritical() << "[Renderer] Theme load failed:" << url.utf8().data()
                << description.utf8().data() << "(" << errorCode << ")";
  }

private:
  bool *loaded_ = nullptr;
};

} // namespace UltralightWebCursor
