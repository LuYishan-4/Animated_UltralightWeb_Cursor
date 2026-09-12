#include "IpcServer.hpp"

#include "BuildConfig.hpp"

#include <QJsonDocument>
#include <QLocalSocket>

namespace UltralightWebCursor {

IpcServer::IpcServer(QObject *parent) : QObject(parent) {
  connect(&server_, &QLocalServer::newConnection, this,
          &IpcServer::onNewConnection);

  const QString socketName = QString::fromUtf8(BuildConfig::ipcSocketName);
  if (!server_.listen(socketName)) {
    QLocalServer::removeServer(socketName);
    server_.listen(socketName);
  }
}

bool IpcServer::isListening() const { return server_.isListening(); }

QString IpcServer::errorString() const { return server_.errorString(); }

void IpcServer::onNewConnection() {
  while (QLocalSocket *socket = server_.nextPendingConnection()) {
    receiveBuffers_.insert(socket, {});
    connect(socket, &QLocalSocket::readyRead, this, &IpcServer::onReadyRead);
    connect(socket, &QLocalSocket::disconnected, this, [this, socket] {
      receiveBuffers_.remove(socket);
      socket->deleteLater();
    });
  }
}

void IpcServer::onReadyRead() {
  auto *socket = qobject_cast<QLocalSocket *>(sender());
  if (!socket)
    return;

  QByteArray &buffer = receiveBuffers_[socket];
  buffer += socket->readAll();

  qsizetype newline = -1;
  while ((newline = buffer.indexOf('\n')) >= 0) {
    const QByteArray message = buffer.left(newline).trimmed();
    buffer.remove(0, newline + 1);
    if (message.isEmpty())
      continue;

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(message, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
      continue;

    const QJsonObject object = document.object();
    Q_EMIT commandReceived(object.value(QStringLiteral("command")).toString(),
                           object.value(QStringLiteral("payload")).toObject());
  }
}

} // namespace UltralightWebCursor
