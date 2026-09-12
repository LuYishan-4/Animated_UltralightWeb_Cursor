#pragma once

#include <QString>

namespace UltralightWebCursor {

struct ThemeMetadata {
  QString displayName;
  QString iconPath;
  QString author = QStringLiteral("Unknown");
  QString description;
  int minWidth = 128;
  int minHeight = 128;
  int hotspotX = 64;
  int hotspotY = 64;
};

class ThemeMetadataReader final {
public:
  static bool load(const QString &themeDirectory, ThemeMetadata &metadata,
                   QString *errorMessage = nullptr);

private:
  ThemeMetadataReader() = delete;
};

} // namespace UltralightWebCursor
