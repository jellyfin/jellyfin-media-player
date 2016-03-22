//
// Created by Tobias Hieta on 24/08/15.
//

#ifndef KONVERGO_INPUTSOCKET_H
#define KONVERGO_INPUTSOCKET_H

#include "LocalJsonServer.h"
#include "InputComponent.h"

class InputSocket : public InputBase
{
  Q_OBJECT
public:
  explicit InputSocket(QObject* parent = nullptr) : InputBase(parent)
  {
    m_server = new LocalJsonServer("inputSocket");
    connect(m_server, &LocalJsonServer::clientConnected, this, &InputSocket::clientConnected);
    connect(m_server, &LocalJsonServer::messageReceived, this, &InputSocket::messageReceived);
  }

  bool initInput() override;
  const char* inputName() override { return "socket"; };

private Q_SLOTS:
  void clientConnected(QLocalSocket* socket);
  void messageReceived(const QVariant& message);

private:
  LocalJsonServer* m_server;
};

#endif //KONVERGO_INPUTSOCKET_H
