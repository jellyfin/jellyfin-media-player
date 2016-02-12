//
// Created by Tobias Hieta on 02/02/16.
//

#include "InputRoku.h"
#include "QsLog.h"

#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include <QUrl>
#include <QXmlStreamWriter>

using namespace qhttp::server;

/////////////////////////////////////////////////////////////////////////////////////////
bool InputRoku::initInput()
{
  m_server = new QHttpServer(this);

  if (!m_server->listen(QHostAddress::Any, 8060))
  {
    QLOG_WARN() << "Failed to start roku component on port 8060";
    return false;
  }

  connect(m_server, &QHttpServer::newRequest, this, &InputRoku::handleRequest);

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::handleRequest(QHttpRequest* request, QHttpResponse* response)
{
  QString path = request->url().path();
  if (path == "/query/apps")
  {
    handleQueryApps(request, response);
  }
  else if (path == "/query/device-info")
  {
    handleQueryDeviceInfo(request, response);
  }
  else if (path.startsWith("/keypress/") || path.startsWith("/keydown/"))
  {
    handleKeyPress(request, response);
  }
  else if (path.startsWith("/keyup/"))
  {
    response->setStatusCode(qhttp::ESTATUS_OK);
    response->end();
  }
  else
  {
    response->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
    response->end();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::handleQueryApps(QHttpRequest* request, QHttpResponse* response)
{
  if (request->method() != qhttp::EHTTP_GET)
  {
    response->setStatusCode(qhttp::ESTATUS_METHOD_NOT_ALLOWED);
    response->end();
    return;
  }

  QByteArray data;
  QXmlStreamWriter writer(&data);
  writer.setAutoFormatting(true);
  writer.writeStartDocument();
  writer.writeStartElement("apps");
  writer.writeStartElement("app");
  writer.writeAttribute("id", "1");
  writer.writeCharacters("Plex Media Player");
  writer.writeEndElement(); // app
  writer.writeEndElement(); // apps
  writer.writeEndDocument();

  response->setStatusCode(qhttp::ESTATUS_OK);
  response->write(data);
  response->end();
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::handleQueryDeviceInfo(QHttpRequest* request, QHttpResponse* response)
{

  QByteArray data;
  QXmlStreamWriter writer(&data);
  writer.setAutoFormatting(true);

  writer.writeStartDocument();
  writer.writeStartElement("device-info");
  writer.writeEndElement(); // device-info
  writer.writeEndDocument();

  response->setStatusCode(qhttp::ESTATUS_OK);
  response->write(data);
  response->end();

  /*
   <device-info>
    <udn>015e5108-9000-1046-8035-b0a737964dfb</udn>
    <serial-number>1GU48T017973</serial-number>
    <device-id>1GU48T017973</device-id>
    <vendor-name>Roku</vendor-name>
    <model-number>4200X</model-number>
    <model-name>Roku 3</model-name>
    <wifi-mac>b0:a7:37:96:4d:fb</wifi-mac>
    <ethernet-mac>b0:a7:37:96:4d:fa</ethernet-mac>
    <network-type>ethernet</network-type>
    <user-device-name/>
    <software-version>7.00</software-version>
    <software-build>09021</software-build>
    <secure-device>true</secure-device>
    <language>en</language>
    <country>US</country>
    <locale>en_US</locale>
    <time-zone>US/Pacific</time-zone>
    <time-zone-offset>-480</time-zone-offset>
    <power-mode>PowerOn</power-mode>
    <developer-enabled>true</developer-enabled>
    <keyed-developer-id>70f6ed9c90cf60718a26f3a7c3e5af1c3ec29558</keyed-developer-id>
    <search-enabled>true</search-enabled>
    <voice-search-enabled>true</voice-search-enabled>
    <notifications-enabled>true</notifications-enabled>
    <notifications-first-use>false</notifications-first-use>
    <headphones-connected>false</headphones-connected>
  </device-info>
   */
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::handleKeyPress(QHttpRequest* request, QHttpResponse* response)
{
  QString path = request->url().path();
  QStringList pathsplit = path.split("/");
  if (pathsplit.count() != 3)
  {
    response->setStatusCode(qhttp::ESTATUS_BAD_REQUEST);
    response->end();
    return;
  }

  QLOG_DEBUG() << "We got roku input:" << pathsplit.value(2);

  emit receivedInput("roku", pathsplit.value(2));

  response->setStatusCode(qhttp::ESTATUS_OK);
  response->end();
  return;
}
