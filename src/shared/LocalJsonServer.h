//
// Created by Tobias Hieta on 30/08/15.
//

#ifndef KONVERGO_LOCALJSONSEVER_H
#define KONVERGO_LOCALJSONSEVER_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonDocument>

class LocalJsonServer : public QObject
{
  Q_OBJECT
public:
  explicit LocalJsonServer(const QString& serverName, QObject* parent = 0);

  bool listen();
  static bool sendMessage(const QVariantMap& message, QLocalSocket* socket);
  static QVariantList readFromSocket(QLocalSocket* socket);
  QString errorString() const { return m_server->errorString(); }

Q_SIGNALS:
  void clientConnected(QLocalSocket* socket);
  void messageReceived(const QVariant& message);

private Q_SLOTS:
  void serverClientConnected();
  void clientReadyRead();

private:
  QString m_serverName;
  QLocalServer* m_server;
  QList<QLocalSocket*> m_clientSockets;
};

#endif //KONVERGO_LOCALJSONSEVER_H
