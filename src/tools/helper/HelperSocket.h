//
// Created by Tobias Hieta on 28/08/15.
//

#ifndef KONVERGO_HELPERSOCKET_H
#define KONVERGO_HELPERSOCKET_H

#include "Paths.h"
#include "LocalJsonServer.h"

class HelperSocket : public QObject
{
  Q_OBJECT
public:
  HelperSocket(QObject* parent = 0);

private:
  Q_SLOT void clientConnected(QLocalSocket* socket);
  Q_SLOT void message(const QVariant& message);
  LocalJsonServer* m_server;
};

#endif //KONVERGO_HELPERSOCKET_H
