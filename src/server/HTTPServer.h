#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QString>
#include "qhttpserverrequest.hpp"
#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"

using namespace qhttp::server;

class HttpServer : public QObject
{
Q_OBJECT
public:
  explicit HttpServer(QObject* parent);
  bool start();

private slots:
  void handleRequest(QHttpRequest* request, QHttpResponse* response);
  void handleWebClientRequest(QHttpRequest* request, QHttpResponse* response);
  void handleResource(QHttpRequest* request, QHttpResponse* response);
  void handleRemoteController(QHttpRequest* request, QHttpResponse* response);
  void handleFilesRequest(QHttpRequest* request, QHttpResponse* response);
  void writeError(QHttpResponse* response, qhttp::TStatusCode errorCode);

private:
  bool writeFile(const QString& file, QHttpResponse* response);

  QHttpServer* m_server;

  QString m_baseUrl;

  quint16 m_port;
};

#endif // HTTPSERVER_H
