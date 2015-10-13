//
// Created by Tobias Hieta on 24/08/15.
//

#include "InputSocket.h"
#include "QsLog.h"
#include "Version.h"

#include <QDataStream>

/////////////////////////////////////////////////////////////////////////////////////////
bool InputSocket::initInput()
{
  return m_server->listen();
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputSocket::clientConnected(QLocalSocket* socket)
{
  QVariantMap welcome;
  welcome.insert("version", Version::GetVersionString());
  welcome.insert("builddate", Version::GetBuildDate());

  m_server->sendMessage(welcome, socket);
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputSocket::messageReceived(const QVariant& message)
{
  QVariantMap map = message.toMap();

  if (!map.contains("client") || !map.contains("source") || !map.contains("keycode"))
  {
    QLOG_WARN() << "Got packet from client but it was missing the important fields";
    return;
  }

  QLOG_DEBUG() << "Input from client:" << map.value("client").toString() << " - " <<
               map.value("source").toString() << map.value("keycode").toString();

  emit receivedInput(map.value("source").toString(), map.value("keycode").toString(), 1);
}
