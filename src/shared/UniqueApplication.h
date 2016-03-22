//
// Created by Tobias Hieta on 27/08/15.
//

#ifndef KONVERGO_UNIQUEAPPLICATION_H
#define KONVERGO_UNIQUEAPPLICATION_H

#include <QObject>
#include "Paths.h"
#include "LocalJsonServer.h"
#include "LocalJsonClient.h"
#include "utils/Utils.h"

#define SOCKET_NAME "pmpUniqueApplication"

class UniqueApplication : public QObject
{
  Q_OBJECT
public:
  UniqueApplication(QObject* parent = nullptr, const QString& socketname = SOCKET_NAME) : QObject(parent)
  {
    m_socketName = socketname;
  }

  void listen()
  {
    m_server = new LocalJsonServer(m_socketName, this);

    connect(m_server, &LocalJsonServer::messageReceived, [=](const QVariant& message)
    {
      QVariantMap map = message.toMap();
      if (map.contains("command") && map.value("command").toString() == "appStart")
        emit otherApplicationStarted();
    });

    if (!m_server->listen())
      throw FatalException("Failed to listen to uniqueApp socket: " + m_server->errorString());
  }

  bool ensureUnique()
  {
    auto socket = new LocalJsonClient(m_socketName, this);
    socket->connectToServer();

    // we will just assume that the app isn't running if we get a error here
    if (!socket->waitForConnected(1000))
    {
      if (socket->error() != QLocalSocket::SocketTimeoutError)
      {
        // since we are unique we will start to listen and claim this socket.
        listen();
        return true;
      }
    }

    QVariantMap m;
    m.insert("command", "appStart");
    socket->sendMessage(m);
    socket->waitForBytesWritten(2000);

    socket->close();
    socket->deleteLater();

    return false;
  }

  Q_SIGNAL void otherApplicationStarted();

private:
  LocalJsonServer* m_server;
  QString m_socketName;
};

#endif //KONVERGO_UNIQUEAPPLICATION_H
