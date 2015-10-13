//
// Created by Tobias Hieta on 30/08/15.
//

#include "LocalJsonServer.h"
#include "Paths.h"
#include "QsLog.h"

#include <QFile>

/////////////////////////////////////////////////////////////////////////////////////////
LocalJsonServer::LocalJsonServer(const QString& serverName, QObject* parent) : QObject(parent)
{
  m_server = new QLocalServer(this);
  m_serverName = Paths::dataDir(serverName);

  connect(m_server, &QLocalServer::newConnection, this, &LocalJsonServer::serverClientConnected);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool LocalJsonServer::listen()
{
  while (!m_server->listen(m_serverName))
  {
    if (m_server->serverError() == QAbstractSocket::AddressInUseError)
    {
      QFile(m_serverName).remove();
      continue;
    }

    QLOG_WARN() << "Failed to listen to local socket:" << m_server->errorString();
    return false;
  }

  QLOG_INFO() << "Listening to socket:" << m_serverName;
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void LocalJsonServer::serverClientConnected()
{
  QLocalSocket* socket = m_server->nextPendingConnection();
  if (socket)
  {
    m_clientSockets << socket;
    connect(socket, &QLocalSocket::readyRead, this, &LocalJsonServer::clientReadyRead);
    emit clientConnected(socket);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
bool LocalJsonServer::sendMessage(const QVariantMap& message, QLocalSocket* socket)
{
  QJsonObject obj = QJsonObject::fromVariantMap(message);

  if (obj.isEmpty())
    return false;

  QJsonDocument doc(obj);
  QByteArray data = doc.toJson(QJsonDocument::Compact);
  data.append("\r\n");

  if (!data.isEmpty())
    return (socket->write(data) == data.size());

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void LocalJsonServer::clientReadyRead()
{
  QLocalSocket* socket = dynamic_cast<QLocalSocket*>(sender());
  if (!socket)
    return;

  QVariantList messages = readFromSocket(socket);
  foreach (const QVariant& msg, messages)
    emit messageReceived(msg.toMap());
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariantList LocalJsonServer::readFromSocket(QLocalSocket* socket)
{
  QVariantList lst;

  while (socket->canReadLine())
  {
    QByteArray data = socket->readLine();
    if (!data.isNull())
    {
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(data, &err);
      if (doc.isNull())
      {
        QLOG_WARN() << "Failed to parse message from client:" << err.errorString();
        continue;
      }

      lst << doc.toVariant();
    }
  }

  return lst;
}