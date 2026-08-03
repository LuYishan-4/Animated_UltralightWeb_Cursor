// IpcServer.hpp
#pragma once
#include <QObject>
#include <QLocalServer>
#include <QJsonObject>

class IpcServer : public QObject {
    Q_OBJECT
public:
    explicit IpcServer(QObject *parent = nullptr);

signals:
    void commandReceived(const QString &command, const QJsonObject &payload);

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QLocalServer *m_server;
};