
#include <QGuiApplication>
#include <QDebug>
#include "InputLIRC.h"

#define DEFAULT_LIRC_ADDRESS "/run/lirc/lircd"

///////////////////////////////////////////////////////////////////////////////////////////////////
InputLIRC::InputLIRC(QObject* parent) : InputBase(parent)
{
  socket = new QLocalSocket(this);
  socketNotifier = NULL;

  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this,
          SLOT(socketerror(QLocalSocket::LocalSocketError)));
  connect(socket, SIGNAL(connected()), this, SLOT(connected()));
  connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
InputLIRC::~InputLIRC()
{
  if (socket)
    disconnect();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputLIRC::initInput()
{
  return connectToLIRC();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputLIRC::connectToLIRC()
{
  disconnect();

  socket->connectToServer(DEFAULT_LIRC_ADDRESS, QIODevice::ReadWrite);
  if (isConnected())
  {
    socketNotifier = new QSocketNotifier(socket->socketDescriptor(), QSocketNotifier::Read, this);
    connect(socketNotifier, SIGNAL(activated(int)), this, SLOT(read(int)));
    return true;
  }
  else
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputLIRC::disconnectFromLIRC()
{
  if (socket)
    socket->disconnectFromServer();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputLIRC::isConnected()
{
  while (socket->state() == QLocalSocket::ConnectingState)
  {
    QGuiApplication::processEvents();
  }

  return (socket->state() == QLocalSocket::ConnectedState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputLIRC::connected()
{
  qInfo() << "LIRC socket connected ";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputLIRC::disconnected()
{
  qInfo() << "LIRC socket disconnected ";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputLIRC::socketerror(QLocalSocket::LocalSocketError socketError)
{
  qCritical() << "LIRC Socket Error : " << socketError;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputLIRC::read(int handle)
{
  QString input;

  while ((input = socket->readLine()) != "")
  {
    QStringList cmdList = input.split(' ', Qt::KeepEmptyParts);
    if (cmdList.size() == 4)
    {
      // grab the input data
      QString code = cmdList.at(0);
      QString repeat = cmdList.at(1);
      QString command = cmdList.at(2);
      QString remote = cmdList.at(3);

      int repeatCount = repeat.toInt();

      qInfo() << "LIRC Got Key : " << command << ", repeat count:" << repeatCount
                  << ", from remote " << remote;

      // we dont want to have all the IR Bursts when we press a key
      // it makes GUI unusable
      if ((repeatCount % 3) == 0)
      {
        bool up = command.endsWith("_LIRCUP");
        emit receivedInput("LIRC", command, up ? InputBase::KeyUp : InputBase::KeyDown);
      }
    }
    else
      qCritical() << "Unknown LIRC input: " << input;
  }
}
