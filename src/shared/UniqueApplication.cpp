//
// Created by Tobias Hieta on 27/08/15.
// Extended for DeepLink support
//

#include "UniqueApplication.h"
#include <QDebug>

///////////////////////////////////////////////////////////////////////////////////////////
UniqueApplication::UniqueApplication(QObject* parent, const QString& socketname) 
  : QObject(parent), m_server(nullptr)
{
  m_socketName = socketname;
}

///////////////////////////////////////////////////////////////////////////////////////////
void UniqueApplication::listen()
{
  m_server = new LocalJsonServer(m_socketName, this);

  connect(m_server, &LocalJsonServer::messageReceived, this, &UniqueApplication::handleMessage);

  if (!m_server->listen())
    throw FatalException("Failed to listen to uniqueApp socket: " + m_server->errorString());
}

///////////////////////////////////////////////////////////////////////////////////////////
bool UniqueApplication::ensureUnique()
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

///////////////////////////////////////////////////////////////////////////////////////////
bool UniqueApplication::sendDeepLink(const QString& url)
{
  auto socket = new LocalJsonClient(m_socketName, this);
  socket->connectToServer();

  if (!socket->waitForConnected(1000))
  {
    qWarning() << "UniqueApplication: Failed to connect for deeplink";
    socket->deleteLater();
    return false;
  }

  QVariantMap message;
  message.insert("command", "deeplink");
  message.insert("url", url);
  
  bool success = socket->sendMessage(message);
  socket->waitForBytesWritten(2000);

  socket->close();
  socket->deleteLater();

  qDebug() << "UniqueApplication: Sent deeplink:" << url << "success:" << success;
  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////
void UniqueApplication::handleMessage(const QVariant& message)
{
  QVariantMap map = message.toMap();
  QString command = map.value("command").toString();
  
  if (command == "appStart")
  {
    emit otherApplicationStarted();
  }
  else if (command == "deeplink")
  {
    QString url = map.value("url").toString();
    if (!url.isEmpty())
    {
      qDebug() << "UniqueApplication: Received deeplink:" << url;
      emit deepLinkReceived(url);
    }
  }
}