//
// Created by Tobias Hieta on 30/08/15.
//

#include "Paths.h"
#include "LocalJsonClient.h"
#include "LocalJsonServer.h"

/////////////////////////////////////////////////////////////////////////////////////////
LocalJsonClient::LocalJsonClient(const QString serverPath, QObject* parent) : QLocalSocket(parent)
{
  m_serverPath = Paths::socketName(serverPath);
  connect(this, &QLocalSocket::readyRead, this, &LocalJsonClient::readyRead);
}

/////////////////////////////////////////////////////////////////////////////////////////
void LocalJsonClient::connectToServer()
{
  QLocalSocket::connectToServer(m_serverPath, QIODevice::ReadWrite);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool LocalJsonClient::sendMessage(const QVariantMap& message)
{
  return LocalJsonServer::sendMessage(message, this);
}

/////////////////////////////////////////////////////////////////////////////////////////
void LocalJsonClient::readyRead()
{
  QVariantList list = LocalJsonServer::readFromSocket(this);
  for(const QVariant& msg : list)
    emit messageReceived(msg.toMap());
}
