#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <functional>
namespace UltralightWebCursorM{

struct JSONConf{
    std::string IconPath;
    std::string Author;
    int minHeight;
    int minWidth;
    std::string describe;
    int hotspotX;
    int hotspotY;
};

class CursorJSON{
public:
    static CursorJSON* instance();
    JSONConf values;
   void ensureInitialized(const std::string& projectPath = "");
    bool load(const std::string& projectPath = "");
private:
    CursorJSON();
    CursorJSON(const CursorJSON&) = delete;
    CursorJSON& operator=(const  CursorJSON&) = delete;
   struct BindItem {
        std::string key;
        std::string defaultValue;
        std::function<void(const std::string&)> updater;
    };
    std::vector<BindItem> schema_;
    std::unordered_map<std::string, std::string> data_;
};
#define CursorJSONImp (::UltralightWebCursorM::CursorJSON::instance()->values)
} 
