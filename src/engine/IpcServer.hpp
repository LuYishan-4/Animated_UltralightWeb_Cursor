#pragma once

#include <QHash>
#include <QJsonObject>
#include <QLocalServer>
#include <QObject>

namespace UltralightWebCursor {

class IpcServer final : public QObject {
  Q_OBJECT

public:
  explicit IpcServer(QObject *parent = nullptr);
  bool isListening() const;
  QString errorString() const;

Q_SIGNALS:
  void commandReceived(const QString &command, const QJsonObject &payload);

private Q_SLOTS:
  void onNewConnection();
  void onReadyRead();

private:
  QLocalServer server_;
  QHash<QObject *, QByteArray> receiveBuffers_;
};

} // namespace UltralightWebCursor
