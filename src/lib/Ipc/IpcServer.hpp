#pragma once

#include <QJsonObject>
#include <QLocalServer>
#include <QObject>

namespace UltralightWebCursorM {

// Local-socket IPC server used by the standalone (X11/Windows) cursor process
// so the settings GUI can send live commands (reload / enable / disable /
// quit / autostart) without restarting the renderer.
class IpcServer : public QObject {
  Q_OBJECT

public:
  explicit IpcServer(QObject *parent = nullptr);
  ~IpcServer() override = default;

  bool isListening() const;

Q_SIGNALS:
  void commandReceived(const QString &command, const QJsonObject &payload);

private Q_SLOTS:
  void onNewConnection();
  void onReadyRead();

private:
  QLocalServer *m_server = nullptr;
};

} // namespace UltralightWebCursorM
