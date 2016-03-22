//
// Created by Tobias Hieta on 02/02/16.
//

#ifndef PLEXMEDIAPLAYER_ROKUREMOTECOMPONENT_H
#define PLEXMEDIAPLAYER_ROKUREMOTECOMPONENT_H

#include "input/InputComponent.h"
#include "qhttpserver.hpp"
#include <QUdpSocket>

class InputRoku : public InputBase
{
  Q_OBJECT

public:
  explicit InputRoku(QObject* parent = nullptr) : InputBase(parent) { }
  bool initInput() override;
  const char* inputName() override { return "roku"; };

private:
  void handleRequest(qhttp::server::QHttpRequest* request, qhttp::server::QHttpResponse* response);
  void handleQueryApps(qhttp::server::QHttpRequest* request, qhttp::server::QHttpResponse* response);
  void handleKeyPress(qhttp::server::QHttpRequest* request, qhttp::server::QHttpResponse* response);
  void handleQueryDeviceInfo(qhttp::server::QHttpRequest* request, qhttp::server::QHttpResponse* response);

  qhttp::server::QHttpServer* m_server;
  QUdpSocket* m_ssdpSocket;

  void ssdpRead();
  void parseSSDPData(const QByteArray& data, const QHostAddress& sender, quint16 port);
  QByteArray getSSDPPacket();
  void handleRootInfo(qhttp::server::QHttpRequest* request, qhttp::server::QHttpResponse* response);
};

#endif //PLEXMEDIAPLAYER_ROKUREMOTECOMPONENT_H
