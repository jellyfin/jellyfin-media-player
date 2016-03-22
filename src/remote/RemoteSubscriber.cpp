//
// Created by Tobias Hieta on 31/03/15.
//

#include <QNetworkAccessManager>
#include <QsLog.h>
#include <QtCore/qxmlstream.h>
#include <QDomDocument>
#include <QMutex>

#include "RemoteSubscriber.h"
#include "RemoteComponent.h"
#include "settings/SettingsComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////
RemoteSubscriber::RemoteSubscriber(const QString& clientIdentifier, const QString& deviceName, const QUrl& address, QObject* parent)
  : QObject(parent), m_address(address), m_clientIdentifier(clientIdentifier), m_deviceName(deviceName)
{
  m_subscribeTime.start();

  if (!address.isEmpty())
  {
    RemoteComponent* remote = dynamic_cast<RemoteComponent*>(parent);
    Q_ASSERT(remote);

    m_netAccess = remote->getNetworkAccessManager();

    // make first access faster by connecting directly to the host now.
    if (address.scheme() == "https")
      m_netAccess->connectToHostEncrypted(address.host(), address.port());
    else
      m_netAccess->connectToHost(address.host(), address.port());
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteSubscriber::reSubscribe()
{
  m_subscribeTime.restart();
}

/////////////////////////////////////////////////////////////////////////////////////////
QString RemoteSubscriber::deviceName()
{
  return m_deviceName;
}

/////////////////////////////////////////////////////////////////////////////////////////
QString RemoteSubscriber::clientIdentifier()
{
  return m_clientIdentifier;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteSubscriber::sendUpdate()
{
  QUrl url(m_address);
  url.setPath("/:/timeline");

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
  request.setAttribute(QNetworkRequest::User, m_clientIdentifier);

  QVariantMap headers = RemoteComponent::HeaderInformation();
  foreach (const QString& key, headers.keys())
  {
    request.setRawHeader(key.toUtf8(), headers[key].toString().toUtf8());
  }

  m_netAccess->post(request, getTimeline());
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteSubscriber::timelineFinished(QNetworkReply* reply)
{
  if (reply->error() != QNetworkReply::NoError)
  {
    QLOG_ERROR() << "got error code when sending timeline:" << reply->errorString();
#if 0
    if (++m_errors > 10)
    {
      QLOG_ERROR() << "More than 10 errors received. Dropping this subscriber";
      RemoteComponent::Get()->subscriberRemove(clientIdentifier());
    }
#endif
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteSubscriber::queueTimeline(quint64 playerCommandID, const QByteArray& timelineData)
{
  QMutexLocker lk(&m_timelineLock);

  QDomDocument doc;
  if (doc.setContent(timelineData) && !doc.firstChildElement("MediaContainer").isNull())
  {
    QDomElement node = doc.firstChildElement("MediaContainer");
    node.setAttribute("commandID", commandId(playerCommandID));
    m_timeline = doc;
  }
  else
    QLOG_WARN() << "Failed to parse timeline data from player";
}

/////////////////////////////////////////////////////////////////////////////////////////
QByteArray RemoteSubscriber::getTimeline()
{
  QMutexLocker lk(&m_timelineLock);

  if (m_timeline.isNull())
  {
    QByteArray xmlData;
    QXmlStreamWriter writer(&xmlData);

    writer.writeStartDocument();
    writer.writeStartElement("MediaContainer");
    writer.writeAttribute("location", "navigation");
    writer.writeAttribute("commandID", QString::number(mostRecentCommandId()));

    writer.writeStartElement("Timeline");
    writer.writeAttribute("type", "music");
    writer.writeAttribute("state", "stopped");
    writer.writeEndElement();

    writer.writeStartElement("Timeline");
    writer.writeAttribute("type", "video");
    writer.writeAttribute("state", "stopped");
    writer.writeEndElement();

    writer.writeStartElement("Timeline");
    writer.writeAttribute("type", "photo");
    writer.writeAttribute("state", "stopped");
    writer.writeEndElement();

    writer.writeEndElement();
    writer.writeEndDocument();

    return xmlData;
  }
  else
  {
    return m_timeline.toByteArray(2);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteSubscriber::setCommandId(quint64 playerCommandId, quint64 controllerCommandId)
{
  QMutexLocker lk(&m_commandIdMapLock);

  m_commandIdMap[playerCommandId] = controllerCommandId;

  // maintain a queue of playerCommandId's so we know in what order to
  // drop them if we get a lot of them. before we queue the current one
  // make sure that we remove any old reference to it so it won't get
  // deleted from the map to early
  //
  m_commandIdQueue.removeAll(playerCommandId);
  m_commandIdQueue.enqueue(playerCommandId);

  if (m_commandIdQueue.size() > 5)
  {
    // remove the last one
    quint64 keyToRemove = m_commandIdQueue.dequeue();
    m_commandIdMap.remove(keyToRemove);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
RemotePollSubscriber::RemotePollSubscriber(const QString &clientIdentifier, const QString &deviceName, QHttpResponse *response, QObject *parent) :
  RemoteSubscriber(clientIdentifier, deviceName, QUrl(""), parent), m_response(response)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
void RemotePollSubscriber::setHTTPResponse(QHttpResponse *response)
{
  m_response = response;
  m_response->addHeader("Content-Type", "application/xml");
  m_response->addHeader("Access-Control-Expose-Headers", "X-Plex-Client-Identifier");

  connect(m_response, &QHttpResponse::done, this, &RemotePollSubscriber::responseDone);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemotePollSubscriber::sendUpdate()
{
  if (m_response)
  {
    // if we have a response, we are handling a poll request
    m_response->setStatusCode(qhttp::ESTATUS_OK);
    m_response->write(getTimeline());

    m_response->end();
    m_response = nullptr;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemotePollSubscriber::responseDone()
{
  m_response = nullptr;
}
