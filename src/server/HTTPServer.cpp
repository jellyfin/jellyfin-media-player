#include "HTTPServer.h"

#include <QFile>
#include <QCoreApplication>

#include "QsLog.h"
#include "utils/Utils.h"
#include "settings/SettingsComponent.h"
#include "remote/RemoteComponent.h"
#include "Paths.h"

#define WEB_CLIENT_PATH "/web/tv"

/////////////////////////////////////////////////////////////////////////////////////////
HttpServer::HttpServer(QObject* parent) : QObject(parent)
{
  m_server = new QHttpServer(this);
  m_baseUrl = ":/konvergo";
  m_port = (quint16)SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "webserverport").toUInt();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool HttpServer::start()
{
  connect(m_server, &QHttpServer::newRequest, this, &HttpServer::handleRequest);
  if (!m_server->listen(QHostAddress::AnyIPv4, m_port))
  {
    QLOG_WARN() << "Failed to listen to remote control web server. Remote controlling from apps disabled.";
    return false;
  }

  QLOG_DEBUG() << "Listening to port:" << m_port;
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void HttpServer::writeError(QHttpResponse* response, QHttpResponse::StatusCode errorCode)
{
  QByteArray errorStyle = "<style>h1 {color:red;}</style>";
  response->writeHead(errorCode);
  response->write(errorStyle);

  QByteArray error;
  switch (errorCode)
  {
    case QHttpResponse::STATUS_FORBIDDEN:
      error = "Access to file denied. You might want to check permissions";
      break;
    case QHttpResponse::STATUS_NOT_FOUND:
      error = "Could not find that file. Install might be broken?";
      break;
    case QHttpResponse::STATUS_METHOD_NOT_ALLOWED:
      error = "That method is not allowed.";
      break;
    case QHttpResponse::STATUS_NOT_IMPLEMENTED:
      error = "This request is not yet implemented";
      break;
    default:
      error = "Generic error, something went wrong<tm>";
      break;
  }
  response->write("<h1>");
  response->write(error);
  response->write("</h1>");
}

/////////////////////////////////////////////////////////////////////////////////////////
bool HttpServer::writeFile(const QString& file, QHttpResponse* response)
{
  QLOG_DEBUG() << "Going to request file:" << qPrintable(file);
  if (QFile::exists(file))
  {
    QFile fp(file);
    if (fp.open(QFile::ReadOnly))
    {
      response->writeHead(QHttpResponse::STATUS_OK);
      response->write(fp.readAll());
      fp.close();
      return true;
    }
    else
    {
      writeError(response, QHttpResponse::STATUS_FORBIDDEN);
      return false;
    }
  }
  else
  {
    writeError(response, QHttpResponse::STATUS_NOT_FOUND);
    return false;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void HttpServer::handleWebClientRequest(QHttpRequest* request, QHttpResponse* response)
{
  switch (request->method())
  {
    case QHttpRequest::HTTP_GET:
    {
      // cut the WEB_CLIENT_PATH prefix
      QString relativeUrl = request->path();
      relativeUrl.replace(WEB_CLIENT_PATH, "");
      QString rUrl = m_baseUrl + relativeUrl;

      writeFile(rUrl, response);
      break;
    }

    default:
    {
      writeError(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
      QLOG_WARN() << "Method" << qPrintable(request->methodString()) << "is not supported";
    }
  }

  response->end();
}

/////////////////////////////////////////////////////////////////////////////////////////
void HttpServer::handleResource(QHttpRequest* request, QHttpResponse* response)
{
  RemoteComponent::Get().handleResource(request, response);
}

/////////////////////////////////////////////////////////////////////////////////////////
void HttpServer::handleRemoteController(QHttpRequest* request, QHttpResponse* response)
{
  RemoteComponent::Get().handleCommand(request, response);
}

/////////////////////////////////////////////////////////////////////////////////////////
void HttpServer::handleFilesRequest(QHttpRequest* request, QHttpResponse* response)
{
  if (request->path() == "/files/qwebchannel.js")
    writeFile(":/qtwebchannel/qwebchannel.js", response);
  else
    writeError(response, QHttpResponse::STATUS_NOT_FOUND);

  response->end();
}

/////////////////////////////////////////////////////////////////////////////////////////
void HttpServer::handleRequest(QHttpRequest* request, QHttpResponse* response)
{
  QLOG_DEBUG() << "Incoming request to:" << request->url().toString() << "from" << request->remoteAddress();

  QString path = request->path();
  if (path.startsWith(WEB_CLIENT_PATH))
  {
    handleWebClientRequest(request, response);
  }
  else if (path.startsWith("/resources"))
  {
    handleResource(request, response);
  }
  else if (path.startsWith("/player"))
  {
    handleRemoteController(request, response);
  }
  else if (path.startsWith("/files"))
  {
    handleFilesRequest(request, response);
  }
  else if (path == "/")
  {
    response->writeHead(QHttpResponse::STATUS_OK);
    response->end("You should not be here :-)");
  }
  else
  {
    writeError(response, QHttpResponse::STATUS_NOT_FOUND);
    response->end();
  }
}

