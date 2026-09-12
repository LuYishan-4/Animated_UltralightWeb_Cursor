#pragma once

#include <QMap>
#include <QString>

namespace UltralightWebCursor {

struct ConfigValues {
  QString version;
  QString htmlPath;
  QString dataRoot;
  int width = 128;
  int height = 128;
  bool enabled = true;
};

class UserConfig final {
public:
  static UserConfig &instance();

  const ConfigValues &values() const { return values_; }
  bool load(QString *errorMessage = nullptr);
  bool save(QString *errorMessage = nullptr) const;

  bool setEnabled(bool enabled, QString *errorMessage = nullptr);
  bool setSize(int width, int height, QString *errorMessage = nullptr);
  bool setTheme(const QString &themeName, QString *errorMessage = nullptr);

  bool importTheme(const QString &sourceDirectory,
                   QString *importedThemeName = nullptr,
                   QString *errorMessage = nullptr);
  bool removeTheme(const QString &themeName, QString *errorMessage = nullptr);

  QString currentTheme() const;

private:
  UserConfig() = default;

  void applyDefaults();
  bool selectUsableTheme(const QString &preferredTheme);

  ConfigValues values_;
};

} // namespace UltralightWebCursor
