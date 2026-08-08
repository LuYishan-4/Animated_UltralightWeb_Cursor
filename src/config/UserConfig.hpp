#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
namespace UltralightWebCursorM {

extern std::filesystem::path g_sdkInitialPath;
extern std::filesystem::path g_htmlInitialPath;

struct ConfigValues {
  std::string configver;
  std::string html;
  std::string sdk;
  std::vector<std::string> blacklist;
  int width;
  int height;
  bool enabled;
  bool EnableGPU;
};

class UserConfig {
public:
  static UserConfig *instance();
  void ensureInitialized();

  ConfigValues values;
  bool load();

  bool save();
  void setKeyValue(const std::string &key, const std::string &path);
  std::string readKeyValue(const std::string &key) const;
  std::vector<std::string> getBlacklist() const;
  void appendBlacklist(const std::string &app);
  void removeBlacklist(const std::string &app);
  bool uploadTheme(const std::string &srcPath, const std::string &themeName);
  bool removeTheme(const std::string &themeName);
  void setTheme(const std::string &themeName);
  std::string currentTheme() const;

private:
  UserConfig();
  UserConfig(const UserConfig &) = delete;
  UserConfig &operator=(const UserConfig &) = delete;
  struct BindItem {
    std::string key;
    std::string defaultValue;
    std::function<void(const std::string &)> updater;
  };

  std::vector<BindItem> schema_;
  std::string configPath_;
  std::unordered_map<std::string, std::string> data_;
};
#define UserConfigimp (::UltralightWebCursorM::UserConfig::instance()->values)
} // namespace UltralightWebCursorM
