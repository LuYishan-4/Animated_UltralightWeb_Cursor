#pragma once 

#include <string>
#include <unordered_map>
#include <algorithm> 
#include <cctype>


enum class WebType {
    TypeScript,
    Html,
    Json,
    Xml,
    Unknown
};
inline WebType stringToWebType(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    static const std::unordered_map<std::string, WebType> typeMap = {
        {"typescript", WebType::TypeScript},
        {"html",       WebType::Html},
        {"json",       WebType::Json},
        {"xml",        WebType::Xml}
    };
    
    auto it = typeMap.find(str);
    return (it != typeMap.end()) ? it->second : WebType::Unknown;
}
