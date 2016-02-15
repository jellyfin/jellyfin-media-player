//
// Created by Tobias Hieta on 02/02/16.
//

#include "InputRoku.h"
#include "QsLog.h"

#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include <QUrl>
#include <QUdpSocket>
#include <QTimeZone>
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
    QByteArray datagram;
    datagram.resize((int)m_ssdpSocket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    m_ssdpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

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

  return packetData;
}
/////////////////////////////////////////////////////////////////////////////////////////
void InputRoku::handleRequest(QHttpRequest* request, QHttpResponse* response)
{
  QString path = request->url().path();

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
  writer.writeTextElement("udn", Utils::ClientUUID());
  writer.writeTextElement("serial-number", ROKU_SERIAL_NUMBER);
  writer.writeTextElement("device-id", ROKU_SERIAL_NUMBER);
  writer.writeTextElement("vendor-name", "Roku");
  writer.writeTextElement("model-number", "4200X");
  writer.writeTextElement("model-name", "Roku 3");
  writer.writeTextElement("wifi-mac", "00:00:00:00:00:00");
  writer.writeTextElement("ethernet-mac", "00:00:00:00:00:00");
  writer.writeTextElement("network-type", "ethernet");
  writer.writeTextElement("user-device-name", Utils::ComputerName());
  writer.writeTextElement("software-version", "7.00");
  writer.writeTextElement("software-build", "09021");

  QLocale locale = QLocale::system();
  QString lcl = locale.name();
  QStringList landc = lcl.split("_");

  writer.writeTextElement("language", landc.value(0));
  writer.writeTextElement("country", landc.value(1));
  writer.writeTextElement("locale", locale.name());

  QTimeZone tz = QTimeZone::systemTimeZone();
  writer.writeTextElement("time-zone", tz.displayName(QTimeZone::StandardTime, QTimeZone::LongName));
  writer.writeTextElement("time-zone-offset", QString::number(tz.offsetFromUtc(QDateTime::currentDateTime()) / 60));
  writer.writeEndElement(); // device-info
  writer.writeEndDocument();

  response->setStatusCode(qhttp::ESTATUS_OK);
  response->write(data);
  response->end();
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
