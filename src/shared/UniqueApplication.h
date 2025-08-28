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
  explicit UniqueApplication(QObject* parent = nullptr, const QString& socketname = SOCKET_NAME) : QObject(parent)
  {
    m_socketName = socketname;
  }

  void listen()
  {
    m_server = new LocalJsonServer(m_socketName, this);

    connect(m_server, &LocalJsonServer::messageReceived, [=](const QVariant& message)
    {
      QVariantMap map = message.toMap();
      if (map.contains("command"))
      {
        QString command = map.value("command").toString();
        if (command == "appStart")
          emit otherApplicationStarted();
        else if (command == "deeplink")
        {
          QString url = map.value("url").toString();
          emit deeplinkReceived(url);
        }
      }
    });

    if (!m_server->listen())
      throw FatalException("Failed to listen to uniqueApp socket: " + m_server->errorString());
  }

  bool ensureUnique(const QString& deepLinkUrl = QString())
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
    if (!deepLinkUrl.isEmpty())
    {
      m.insert("command", "deeplink");
      m.insert("url", deepLinkUrl);
    }
    else
    {
      m.insert("command", "appStart");
    }
    
    socket->sendMessage(m);
    socket->waitForBytesWritten(2000);

    socket->close();
    socket->deleteLater();

    return false;
  }

  Q_SIGNAL void otherApplicationStarted();
  Q_SIGNAL void deeplinkReceived(const QString& url);

private:
  LocalJsonServer* m_server;
  QString m_socketName;
};

#endif //KONVERGO_UNIQUEAPPLICATION_H
