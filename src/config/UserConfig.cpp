#include "UserConfig.hpp"
#include <QDBusConnection>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <QDebug>
#include "../lib/Quick/PluginPath/PluginPath.hpp"
#include "GlobalConstas.hpp"
namespace fs = std::filesystem;
namespace UltralightWebCursorM{
fs::path g_sdkInitialPath;
fs::path g_htmlInitialPath;


static std::vector<std::string> parseCsv(const std::string& str) {
    std::vector<std::string> res;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            res.push_back(item);
        }
    }
    return res;
}

UserConfig* UserConfig::instance() {
    static UserConfig inst;
    return &inst;
}

UserConfig::UserConfig(){
    const char* home = std::getenv("HOME");
    if(home) configPath_ = std::string(home) + "/.config/ultralightwebcursor/config.ini";
    auto base = KWin::PluginPath::dataDir();
    g_sdkInitialPath = base / "sdk" / "ultralight-free-sdk-1.4.0-linux-x64";
    g_htmlInitialPath = g_sdkInitialPath / "resources";
    schema_ = {
        {"configver",  "1.0.0", [this](const std::string& v){ values.configver = v; }},
        {"html",       (g_htmlInitialPath / "default" / "index.html").string(), [this](const std::string& v){ values.html = v; }},
        {"sdk",        g_sdkInitialPath.string(), [this](const std::string& v){ values.sdk = v; }},
        {"blacklist",  "",      [this](const std::string& v){ values.blacklist = parseCsv(v); }},
        {"width",      "128",   [this](const std::string& v){ values.width = v.empty() ? 128 : std::stoi(v); }},
        {"height",     "128",   [this](const std::string& v){ values.height = v.empty() ? 128 : std::stoi(v); }},
        {"enabled",    "true",  [this](const std::string& v){ values.enabled = (v == "true"); }},
        {"isautohide", "true",  [this](const std::string& v){ values.isautohide = (v == "true"); }}
    };
    load();
}

bool UserConfig::load(){
    data_.clear();
    if(configPath_.empty()) return false;
    std::ifstream file(configPath_);
    if(!file.is_open()) {
        for (const auto& item : schema_) {
            data_[item.key] = item.defaultValue;
            item.updater(item.defaultValue);
        }
        return save();
    }
    std::string line;
    while(std::getline(file, line)){
        auto pos = line.find('=');
        if(pos == std::string::npos) continue;
        data_[line.substr(0, pos)] = line.substr(pos + 1);
    }
    bool needReSave = false;
    if (data_["configver"] != GloablContast::Version) {
        data_["configver"] = GloablContast::Version;
        needReSave = true;
    }

    for (const auto& item : schema_) {
        if (data_.find(item.key) == data_.end()) {
            data_[item.key] = item.defaultValue;
            needReSave = true;
        }
        item.updater(data_[item.key]);
    }
    
    if (needReSave)save();
    return true;
}

bool UserConfig::save(){
    if(configPath_.empty()) return false;
    fs::create_directories(fs::path(configPath_).parent_path());
    std::ofstream file(configPath_);
    if(!file.is_open()) return false;
    for(const auto& [k, v] : data_){
        file << k << "=" << v << "\n";
    }
    return true;
}

void UserConfig::setKeyValue(const std::string& key, const std::string& path){
    data_[key] = path;
    for (const auto& item : schema_) {
        if (item.key == key) {
            item.updater(path);
            break;
        }
    }
}

std::string UserConfig::readKeyValue(const std::string& key) const{
    auto it = data_.find(key);
    if(it == data_.end()){
        std::cerr << "[UserConfig] key not found: " << key << "\n";
        return "";
    }
    return it->second;
}

std::vector<std::string> UserConfig::getBlacklist() const{
    std::vector<std::string> result;
    auto it = data_.find("blacklist");
    if(it == data_.end() || it->second.empty()) return result;
    std::stringstream ss(it->second);
    std::string item;
    while(std::getline(ss, item, ',')){
        if(!item.empty())
            result.push_back(item);
    }
    return result;
}

void UserConfig::appendBlacklist(const std::string& app) {
    if (app.empty()) return;
    std::string current_list = data_["blacklist"]; 
    if (current_list.empty()) {
        setKeyValue("blacklist", app); 
        save();
        return;
    }
    size_t pos = current_list.find(app);
    while (pos != std::string::npos) {
        bool match_start = (pos == 0 || current_list[pos - 1] == ',');
        bool match_end = (pos + app.length() == current_list.length() || current_list[pos + app.length()] == ',');
        if (match_start && match_end) return; 
        
        pos = current_list.find(app, pos + 1);
    }
    current_list += "," + app;
    setKeyValue("blacklist", current_list);
    save();
}

void UserConfig::removeBlacklist(const std::string& app){
    std::string current_list = data_["blacklist"];
    if (current_list.empty()) return;
    std::string new_value;
    std::string token;
    std::stringstream ss(current_list);
    while (std::getline(ss, token, ',')) {
        if (token == app) continue; 
        if (!new_value.empty()) new_value += ",";
        new_value += token;
    }
    setKeyValue("blacklist", new_value);
    save();
}

bool UserConfig::uploadTheme(const std::string& path, const std::string& themeName){
    std::error_code ec;
    fs::path src(path);
    if (!fs::exists(src, ec) || ec){
        qDebug() << "src not exists:" << ec.message().c_str();
        return false;
    }
    if (!fs::is_directory(src, ec) || ec){
        qDebug() << "src is not directory:" << ec.message().c_str();
        return false;
    }
    fs::path dst = g_sdkInitialPath / "resources" / themeName;
    qDebug() << "uploadTheme dst =" << dst.string().c_str();
    fs::remove_all(dst, ec);
    if (ec){
        qDebug() << "remove old theme failed:" << ec.message().c_str();
        return false;
    }
    fs::create_directories(dst, ec);
    if (ec){
        qDebug() << "create directory failed:" << ec.message().c_str();
        return false;
    }
    qDebug() << "dst exists =" << fs::exists(dst);
    fs::directory_iterator it(src, ec);
    if (ec){
        qDebug() << "iterator failed:" << ec.message().c_str();
        return false;
    }
    for (; it != fs::directory_iterator(); it.increment(ec)){
        if (ec){
            qDebug() << "iterator increment failed:" << ec.message().c_str();
            return false;
        }
        fs::path sourceFile = it->path();
        fs::path targetFile = dst / sourceFile.filename();
        qDebug() << "copy:" << sourceFile.string().c_str() << "->" << targetFile.string().c_str();
        fs::copy(
            sourceFile,
            targetFile,
            fs::copy_options::recursive | fs::copy_options::overwrite_existing,
            ec
        );
        if (ec){
            qDebug() << "copy failed:" << ec.message().c_str();
            return false;
        }
    }
    qDebug() << "uploadTheme success";
    return true;
}

void UserConfig::setTheme(const std::string& themeName){
    std::string htmlPath = (g_htmlInitialPath / themeName / "index.html").string();
    setKeyValue("html", htmlPath);
    save();
}

std::string UserConfig::currentTheme() const{
    auto it = data_.find("html");
    if(it == data_.end() || it->second.empty()){
        return "default";
    }
    std::filesystem::path htmlPath = it->second;
    auto themePath = htmlPath.parent_path();
    if(themePath.filename().empty()){
        return "default";
    }
    return themePath.filename().string();
}

}
