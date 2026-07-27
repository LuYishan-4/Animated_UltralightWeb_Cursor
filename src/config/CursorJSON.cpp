#include "CursorJSON.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace UltralightWebCursorM {

CursorJSON* CursorJSON::instance() {
    static CursorJSON inst;
    return &inst;
}

CursorJSON::CursorJSON() {
    schema_ = {
        {"IconPath",  "",       [this](const std::string& v) { values.IconPath = v; }},
        {"WebType",   "html",   [this](const std::string& v) { values.WebType = v; }},
        {"Author",    "Unknown",[this](const std::string& v) { values.Author = v; }},
        {"minHeight", "128",    [this](const std::string& v) { values.minHeight = std::stoi(v); }},
        {"minWidth",  "128",    [this](const std::string& v) { values.minWidth = std::stoi(v); }},
        {"describe",  "",       [this](const std::string& v) { values.describe = v; }},
        {"localServer",  "false",  [this](const std::string& v){ values.localServer = (v == "false"); }},
        {"main",  "",       [this](const std::string& v) { values.main = v; }},
    };
    for (const auto& item : schema_) {
        item.updater(item.defaultValue);
    }
}


void CursorJSON::ensureInitialized(const std::string& projectPath) {
    static bool initialized = false;
    if (!initialized) {
        load(projectPath);
        initialized = true;
    }
}
bool CursorJSON::load(const std::string& projectPath) {
    std::filesystem::path configPath = "CursorData.json";
    
    if (!projectPath.empty()) {
        configPath = std::filesystem::path(projectPath) / "CursorData.json";
    }

    if (!std::filesystem::exists(configPath)) {
        return false;
    }

    std::ifstream file(configPath);
    if (!file.is_open()) return false;
    std::string line;
    while (std::getline(file, line)) {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;
        std::string key = line.substr(0, colonPos);
        std::string val = line.substr(colonPos + 1);
        auto cleanStr = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n\""));
            size_t end = s.find_last_not_of(" \t\r\n\",");
            if (end != std::string::npos) s.erase(end + 1);
        };
        cleanStr(key);
        cleanStr(val);
        if (!key.empty()) data_[key] = val;
    }

    for (const auto& item : schema_) {
        auto it = data_.find(item.key);
        if (it != data_.end()) {
            try {
                item.updater(it->second);
            } catch (...) {
                return false;
            }
        }
    }

    return true;
}

} 
