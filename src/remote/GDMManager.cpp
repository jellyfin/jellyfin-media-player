#include "GDMManager.h"

#include <QUdpSocket>
#include <QHostInfo>

#include "settings/SettingsComponent.h"
#include "QsLog.h"
#include "RemoteComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
GDMManager::GDMManager(QObject *parent) : QObject(parent), m_port(-1)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GDMManager::startAnnouncing()
{
  startListener();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GDMManager::stopAnnouncing()
{
  m_socket.close();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GDMManager::startListener()
{
  m_socket.bind(QHostAddress::AnyIPv4, 32412, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);

  QHostAddress multicast("239.0.0.250");
  m_socket.joinMulticastGroup(multicast);

  connect(&m_socket, &QUdpSocket::readyRead, this, &GDMManager::readData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GDMManager::readData()
{
  while (m_socket.hasPendingDatagrams())
  {
    QByteArray datagram;
    datagram.resize(m_socket.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    m_socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
    parseData(datagram, sender, senderPort);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GDMManager::parseData(const QByteArray& data, const QHostAddress& sender, quint16 port)
{
  if (data.startsWith("M-SEARCH *"))
    m_socket.writeDatagram(getPacket(), sender, port);
}

/////////////////////////////////////////////////////////////////////////////////////////
QByteArray GDMManager::getPacket()
{
  QByteArray packetData;

  // Header
  packetData.append("HTTP/1.0 200 OK\r\n");
  packetData.append("Content-Type: plex/media-player\r\n");

  QVariantMap headers = RemoteComponent::GDMInformation();

  foreach (const QString& key, headers.keys())
    packetData.append(key + ": " + headers[key].toString() + "\r\n");

  // terminate header
  packetData.append("\r\n");
  
  return packetData;
}