#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <functional>
namespace UltralightWebCursorM{

struct JSONConf{
    std::string IconPath;
    std::string WebType;
    std::string Author;
    int minHeight;
    int minWidth;
    std::string describe;
};

class CursorJSON{
public:
    static CursorJSON* instance();
    void ensureInitialized();
    JSONConf values;
    bool load();
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
