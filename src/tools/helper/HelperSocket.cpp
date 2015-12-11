//
// Created by Tobias Hieta on 28/08/15.
//

#include "HelperSocket.h"
#include "utils/Utils.h"
#include "Version.h"
#include "QsLog.h"
#include "HelperSettings.h"

#include <QCoreApplication>

/////////////////////////////////////////////////////////////////////////////////////////
HelperSocket::HelperSocket(QObject* parent)
{
  m_server = new LocalJsonServer("pmpHelper", this);
  m_quitTimer = new QTimer(this);

  connect(m_quitTimer, &QTimer::timeout, []()
  {
    QLOG_DEBUG() << "Quit timer ran out, quitting...";
    qApp->quit();
  });

  connect(m_server, &LocalJsonServer::clientConnected, this, &HelperSocket::clientConnected);
  connect(m_server, &LocalJsonServer::messageReceived, this, &HelperSocket::message);

  if (!m_server->listen())
    throw FatalException("Could not listen to helper socket!");
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperSocket::clientConnected(QLocalSocket* socket)
{
  QLOG_DEBUG() << "Server connected.";
  QVariantMap hello;
  hello.insert("version", Version::GetVersionString());
  hello.insert("path", QCoreApplication::applicationFilePath());
  m_server->sendMessage(hello, socket);

  // if we are going to quit, restart the timer.
  m_quitTimer->stop();

  connect(socket, &QLocalSocket::disconnected, [=](){
    // give us 5 minute to upload a crash log if we got one. then quit
    QLOG_DEBUG() << "PMP application quit, let's wait 3 minutes and then exit";
    m_quitTimer->start(3 * 60 * 1000);
  });
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperSocket::message(const QVariant& message)
{
  QVariantMap map = message.toMap();
  if (map.contains("command"))
  {
    if (map.value("command").toString() == "quit")
    {
      QLOG_DEBUG() << "Asked to quit.";
      qApp->quit();
    }
    else if (map.value("command").toString() == "info")
    {
      QLOG_DEBUG() << "Updating clientID.";
      QVariantMap arg = map.value("argument").toMap();
      if (arg.contains("clientId"))
        HelperSettings().setValue("clientId", arg.value("clientId").toString());

      if (arg.contains("userId"))
        HelperSettings().setValue("userId", arg.value("userId").toString());
    }
  }
}