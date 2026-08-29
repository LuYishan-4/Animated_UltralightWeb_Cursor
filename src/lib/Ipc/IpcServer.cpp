#include "IpcServer.hpp"

#include <QJsonDocument>
#include <QLocalSocket>

namespace UltralightWebCursorM {

namespace {
constexpr auto kSocketName = "ultralightwebcursor_ipc";
}

IpcServer::IpcServer(QObject *parent) : QObject(parent) {
  QLocalServer::removeServer(QLatin1String(kSocketName));
  m_server = new QLocalServer(this);
  connect(m_server, &QLocalServer::newConnection, this,
          &IpcServer::onNewConnection);
  m_server->listen(QLatin1String(kSocketName));
}

bool IpcServer::isListening() const {
  return m_server && m_server->isListening();
}

void IpcServer::onNewConnection() {
  QLocalSocket *socket = m_server->nextPendingConnection();
  if (!socket)
    return;
  connect(socket, &QLocalSocket::readyRead, this, &IpcServer::onReadyRead);
  connect(socket, &QLocalSocket::disconnected, socket,
          &QLocalSocket::deleteLater);
}

void IpcServer::onReadyRead() {
  auto *socket = qobject_cast<QLocalSocket *>(sender());
  if (!socket)
    return;

  const auto data = socket->readAll();
  const auto doc = QJsonDocument::fromJson(data);
  if (!doc.isObject())
    return;

  const auto obj = doc.object();
  Q_EMIT commandReceived(obj.value(QStringLiteral("command")).toString(),
                         obj.value(QStringLiteral("payload")).toObject());
}

} // namespace UltralightWebCursorM
