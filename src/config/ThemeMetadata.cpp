#include "ThemeMetadata.hpp"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <algorithm>

namespace UltralightWebCursor {
namespace {

int boundedInteger(const QJsonValue &value, int fallback, int minimum,
                   int maximum) {
  bool ok = false;
  int parsed = fallback;
  if (value.isDouble()) {
    parsed = value.toInt(fallback);
    ok = true;
  } else if (value.isString()) {
    parsed = value.toString().toInt(&ok);
  }
  return ok ? std::clamp(parsed, minimum, maximum) : fallback;
}

} // namespace

bool ThemeMetadataReader::load(const QString &themeDirectory,
                               ThemeMetadata &metadata, QString *errorMessage) {
  metadata = ThemeMetadata{};

  QFile file(QDir(themeDirectory).filePath(QStringLiteral("CursorData.json")));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    if (errorMessage)
      *errorMessage = QStringLiteral("CursorData.json could not be opened");
    return false;
  }

  QJsonParseError parseError;
  const QJsonDocument document =
      QJsonDocument::fromJson(file.readAll(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
    if (errorMessage)
      *errorMessage = QStringLiteral("Invalid CursorData.json: %1")
                          .arg(parseError.errorString());
    return false;
  }

  const QJsonObject object = document.object();
  metadata.displayName = object.value(QStringLiteral("Name")).toString();
  metadata.iconPath = object.value(QStringLiteral("IconPath")).toString();
  metadata.author =
      object.value(QStringLiteral("Author")).toString(metadata.author);
  metadata.description = object.value(QStringLiteral("describe")).toString();
  metadata.minWidth = boundedInteger(object.value(QStringLiteral("minWidth")),
                                     metadata.minWidth, 16, 4096);
  metadata.minHeight = boundedInteger(object.value(QStringLiteral("minHeight")),
                                      metadata.minHeight, 16, 4096);
  metadata.hotspotX = boundedInteger(object.value(QStringLiteral("hotspotX")),
                                     metadata.hotspotX, 0, 4096);
  metadata.hotspotY = boundedInteger(object.value(QStringLiteral("hotspotY")),
                                     metadata.hotspotY, 0, 4096);
  return true;
}

} // namespace UltralightWebCursor
