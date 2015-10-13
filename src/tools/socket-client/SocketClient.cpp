//
// Created by Tobias Hieta on 26/08/15.
//

#include <QCoreApplication>
#include <QLocalSocket>
#include <qqueue.h>
#include "LocalJsonClient.h"

class SocketClient : public QObject
{
  Q_OBJECT
public:
  SocketClient() : QObject(NULL)
  {
    m_socket = new LocalJsonClient("inputSocket", this);
    m_socket->connectToServer();

    connect(m_socket, &LocalJsonClient::messageReceived, this, &SocketClient::gotMessage);
  }

  void sendCommand(const QString& command)
  {
    m_commands.enqueue(command);
  }

  Q_SLOT void gotMessage(const QVariantMap& message)
  {
    qDebug() << message;
    doSendCommand();
  }

  void doSendCommand()
  {
    while (!m_commands.isEmpty())
    {
      QString cmd = m_commands.dequeue();

      QVariantMap obj;
      obj.insert("client", "socket-client");
      obj.insert("source", "direct");
      obj.insert("keycode", cmd);

      m_socket->sendMessage(obj);

      qDebug() << "Sending:" << cmd;
    }

    m_socket->waitForBytesWritten(60 * 1000);
    m_socket->close();
    exit(EXIT_SUCCESS);
  }

private:
  LocalJsonClient* m_socket;
  QQueue<QString> m_commands;
};

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);

  SocketClient* client = new SocketClient();

  for (int i = 1; i < argc; i ++)
    client->sendCommand(QString::fromUtf8(argv[i]));

  return app.exec();
}


#include "SocketClient.moc"