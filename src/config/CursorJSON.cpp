#include "CursorJSON.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QIODevice>
#include <QString>
#include <QDebug>

namespace UltralightWebCursorM {

CursorJSON* CursorJSON::instance() {
    static CursorJSON inst;
    return &inst;
}

CursorJSON::CursorJSON() {
    schema_ = {
        {"IconPath",  "",       [this](const std::string& v) { values.IconPath = v; }},
        {"Author",    "Unknown",[this](const std::string& v) { values.Author = v; }},
        {"minHeight", "128",    [this](const std::string& v) { values.minHeight = std::stoi(v); }},
        {"minWidth",  "128",    [this](const std::string& v) { values.minWidth = std::stoi(v); }},
        {"describe",  "",       [this](const std::string& v) { values.describe = v; }},
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
    qDebug() << "[UltralightCursorEffect] Loading cursor config...";
    QString configPath = QStringLiteral("CursorData.json");
    if (!projectPath.empty()) {
        configPath = QString::fromStdString(projectPath) + "/CursorData.json";
    }

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qDebug() << "[UltralightCursorEffect] JSON Parse Error:" << error.errorString();
        return false;
    }

    QJsonObject obj = doc.object();
    
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        std::string key = it.key().toStdString();
        std::string val;
        
        if (it.value().isString()) {
            val = it.value().toString().toStdString();
        } else if (it.value().isDouble()) {
            val = std::to_string(static_cast<int>(it.value().toDouble()));
        } else if (it.value().isBool()) {
            val = it.value().toBool() ? "true" : "false";
        }
        
        if (!key.empty()) {
            data_[key] = val;
        }
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
