//
// Created by Tobias Hieta on 02/02/16.
//

#include "InputRoku.h"
#include "QsLog.h"

#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include <QUrl>
#include <QUdpSocket>
#include <QXmlStreamWriter>

using namespace qhttp::server;

#define ROKU_SERIAL_NUMBER "12345678900"

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

  m_ssdpSocket = new QUdpSocket(this);
  if (!m_ssdpSocket->bind(QHostAddress::AnyIPv4, 1900, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
  {
    QLOG_WARN() << "Failed to bind to SSDP socket";
    return false;
  }

  QHostAddress multicast("239.255.255.250");
  m_ssdpSocket->joinMulticastGroup(multicast);

  connect(m_ssdpSocket, &QUdpSocket::readyRead, this, &InputRoku::ssdpRead);

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::ssdpRead()
{
  while (m_ssdpSocket->hasPendingDatagrams())
  {
    QLOG_DEBUG() << "Got SSDP datagram";

    QByteArray datagram;
    datagram.resize((int)m_ssdpSocket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    m_ssdpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

    QLOG_DEBUG() << "datagram:" << QString::fromUtf8(datagram);
    parseSSDPData(datagram, sender, senderPort);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::parseSSDPData(const QByteArray& data, const QHostAddress& sender, quint16 port)
{
  if (data.contains("M-SEARCH * HTTP/1.1"))
    m_ssdpSocket->writeDatagram(getSSDPPacket(), sender, port);
}

/////////////////////////////////////////////////////////////////////////////////////////
QByteArray InputRoku::getSSDPPacket()
{

  /*
    HTTP/1.1 200 OK
    Cache-Control: max-age=300
    ST: roku:ecp
    Location: http://192.168.1.134:8060/
    USN: uuid:roku:ecp:P0A070000007
   */

  QByteArray packetData;

  // Header
  packetData.append("HTTP/1.1 200 OK\r\n");
  packetData.append("Cache-Control: max-age=300\r\n");
  packetData.append("ST: roku:ecp\r\n");
  packetData.append("Ext: \r\n");
  packetData.append("Server: Roku UPnP/1.0 MiniUPnPd/1.4\r\n");

  packetData.append("Location: http://");
  packetData.append(Utils::PrimaryIPv4Address());
  packetData.append(":8060/\r\n");

  packetData.append("USN: uuid:roku:ecp:");
  packetData.append(ROKU_SERIAL_NUMBER);
  packetData.append("\r\n\r\n");

  QLOG_DEBUG() << "Reply: " << QString::fromUtf8(packetData);

  return packetData;
}
/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::handleRequest(QHttpRequest* request, QHttpResponse* response)
{
  QString path = request->url().path();

  QLOG_DEBUG() << "Request:" << path;

  if (path == "/")
  {
    handleRootInfo(request, response);
  }
  else if (path == "/query/apps")
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
    QLOG_WARN() << "Could not handle roku input:" << path;
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

/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::handleRootInfo(QHttpRequest* request, QHttpResponse* response)
{
  QByteArray data;
  QXmlStreamWriter writer(&data);
  writer.setAutoFormatting(true);

  writer.writeStartDocument();
  writer.writeStartElement("urn:schemas-upnp-org:device-1-0", "root");

  writer.writeStartElement("specVersion");
  writer.writeTextElement("major", "1");
  writer.writeTextElement("minor", "0");
  writer.writeEndElement(); // specVersion

  writer.writeStartElement("device");
  writer.writeTextElement("deviceType", "urn:roku-com:device:player:1-0");
  writer.writeTextElement("friendlyName", Utils::ComputerName());
  writer.writeTextElement("manufacturer", "Roku");
  writer.writeTextElement("manufacturerURL", "http://www.roku.com");
  writer.writeTextElement("modelDescription", "Roku Streaming Player Network Media");
  writer.writeTextElement("modelName", "Roku 3");
  writer.writeTextElement("modelNumber", "4200X");
  writer.writeTextElement("modelURL", "http://www.roku.com");
  writer.writeTextElement("serialNumber", ROKU_SERIAL_NUMBER);
  writer.writeTextElement("UDN", "uuid:" + Utils::ClientUUID());

  writer.writeStartElement("serviceList");
  writer.writeStartElement("service");
  writer.writeTextElement("serviceType", "urn:roku-com:service:ecp:1");
  writer.writeTextElement("serviceId", "urn:roku-com:serviceId:ecp1-0");
  writer.writeTextElement("controlURL", "");
  writer.writeTextElement("eventSubURL", "");
  writer.writeTextElement("SCPDURL", "ecp_SCPD.xml");
  writer.writeEndElement(); // service
  writer.writeEndElement(); // serviceList

  writer.writeEndElement(); // root
  writer.writeEndDocument();

  response->setStatusCode(qhttp::ESTATUS_OK);
  response->write(data);
  response->end();
}
