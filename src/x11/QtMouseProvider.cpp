#include "../header/QtMouseProvider.hpp"
#include "../lib/X11MouseProvider.hpp"

QtMouseProvider::QtMouseProvider(QObject *parent) : QObject(parent) {}

bool QtMouseProvider::initialize() { return true; }

void QtMouseProvider::setCallback(Callback callback) {
  callback_ = std::move(callback);
}
void QtMouseProvider::updateMouseState() {
  if (!callback_)
    return;

  UltralightWebCursorM::MousePoint point;

  if (UltralightWebCursorM::readX11CursorPosition(point)) {
    callback_(point);
  }
}

void QtMouseProvider::onTimer() { updateMouseState(); }
