// IpcServer.cpp
#include "Icpserver.hpp"
#include <QLocalSocket>
#include <QJsonDocument>

IpcServer::IpcServer(QObject *parent) : QObject(parent) {
    QLocalServer::removeServer("ultralightwebcursor_ipc");
    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &IpcServer::onNewConnection);
    m_server->listen("ultralightwebcursor_ipc");
}

void IpcServer::onNewConnection() {
    QLocalSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QLocalSocket::readyRead, this, &IpcServer::onReadyRead);
    connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}

void IpcServer::onReadyRead() {
    auto *socket = qobject_cast<QLocalSocket *>(sender());
    const auto data = socket->readAll();
    const auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    const auto obj = doc.object();
    emit commandReceived(obj.value("command").toString(), obj.value("payload").toObject());
}