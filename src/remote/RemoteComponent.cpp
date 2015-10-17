//
// Created by Tobias Hieta on 24/03/15.
//

#include "RemoteComponent.h"

#include <QXmlStreamWriter>
#include <QUrlQuery>

#include "QsLog.h"
#include "settings/SettingsComponent.h"
#include "utils/Utils.h"
#include "Version.h"

static QMap<QString, QString> resourceKeyMap = {
  { "Name", "title" },
  { "Resource-Identifier", "machineIdentifier" },
  { "Product", "product" },
  { "Version", "version" },
  { "Protocol-Version", "protocolVersion" },
  { "Protocol-Capabilities", "protocolCapabilities" },
  { "Device-Class", "deviceClass" }
};

static QMap<QString, QString> headerKeyMap = {
  { "Name", "X-Plex-Device-Name" },
  { "Resource-Identifier", "X-Plex-Client-Identifier" },
  { "Product", "X-Plex-Product" },
  { "Version", "X-Plex-Version" }
};

/////////////////////////////////////////////////////////////////////////////////////////
RemoteComponent::RemoteComponent(QObject* parent) : ComponentBase(parent), m_commandId(0)
{
  m_gdmManager = new GDMManager(this);
  m_networkAccessManager = new QNetworkAccessManager(this);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool RemoteComponent::componentInitialize()
{
  m_gdmManager->startAnnouncing();

  // check for timed out subscribers
  m_subscriberTimer.setInterval(5000);
  connect(&m_subscriberTimer, &QTimer::timeout, this, &RemoteComponent::checkSubscribers);
  m_subscriberTimer.start();

  // connect the network access stuff
  connect(m_networkAccessManager, &QNetworkAccessManager::finished, this, &RemoteComponent::timelineFinished);

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariantMap RemoteComponent::HeaderInformation()
{
  QVariantMap gdmInfo = GDMInformation();
  QVariantMap headerInfo;

  foreach (const QString& key, gdmInfo.keys())
  {
    if (headerKeyMap.contains(key))
      headerInfo[headerKeyMap[key]] = gdmInfo[key];
  }

  headerInfo["X-Plex-Platform"] = QSysInfo::productType();
  headerInfo["X-Plex-Platform-Version"] = QSysInfo::productVersion();
  
  return headerInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariantMap RemoteComponent::ResourceInformation()
{
  QVariantMap gdmInfo = GDMInformation();
  QVariantMap resourceInfo;

  foreach (const QString& key, gdmInfo.keys())
  {
    if (resourceKeyMap.contains(key))
      resourceInfo[resourceKeyMap[key]] = gdmInfo[key];
  }

  resourceInfo["platform"] = QSysInfo::productType();
  resourceInfo["platformVersion"] = QSysInfo::productVersion();

  return resourceInfo;
};

/////////////////////////////////////////////////////////////////////////////////////////
QVariantMap RemoteComponent::GDMInformation()
{
  QVariantMap headers = {
    {"Name", Utils::ComputerName()},
    {"Port", SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "webserverport")},
    {"Version", Version::GetVersionString()},
    {"Product", "Plex Media Player"},
    {"Protocol", "plex"},
    {"Protocol-Version", "1"},
    {"Protocol-Capabilities", "navigation,playback,timeline,mirror,playqueues"},
    {"Device-Class", "pc"},
    {"Resource-Identifier", SettingsComponent::Get().value(SETTINGS_SECTION_WEBCLIENT, "clientID")}
  };

  return headers;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::handleResource(QHttpRequest* request, QHttpResponse* response)
{
  if (request->method() == QHttpRequest::HTTP_GET)
  {
    QVariantMap headers = ResourceInformation();

    QByteArray outputData;
    QXmlStreamWriter output(&outputData);
    output.setAutoFormatting(true);
    output.writeStartDocument();
    output.writeStartElement("MediaContainer");
    output.writeStartElement("Player");

    foreach (const QString& key, headers.keys())
      output.writeAttribute(key, headers[key].toString());

    output.writeEndElement();
    output.writeEndDocument();

    response->writeHead(QHttpResponse::STATUS_OK);
    response->write(outputData);
    response->end();
  }
  else
  {
    response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    response->end();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariantMap RemoteComponent::QueryToMap(const QUrl& url)
{
  QUrlQuery query(url);
  QVariantMap queryMap;

  QPair<QString, QString> stringPair;

  foreach (stringPair, query.queryItems())
  {
    QString key = stringPair.first;
    QString value = stringPair.second;

    QVariantList l;
    if (queryMap.contains(key))
    {
      l = queryMap[key].toList();
      l.append(value);
    }
    else
    {
      l.append(value);
    }
    queryMap[key] = l;
  }

  return queryMap;
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariantMap RemoteComponent::HeaderToMap(const HeaderHash& hash)
{
  QVariantMap variantMap;
  foreach (const QString& key, hash.keys())
    variantMap[key] = hash[key];
  return variantMap;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::handleCommand(QHttpRequest* request, QHttpResponse* response)
{

  QVariantMap queryMap = QueryToMap(request->url());
  QVariantMap headerMap = HeaderToMap(request->headers());
  QString identifier = headerMap["x-plex-client-identifier"].toString();

  response->setHeader("Access-Control-Allow-Origin", "*");
  response->setHeader("X-Plex-Client-Identifier",  SettingsComponent::Get().value(SETTINGS_SECTION_WEBCLIENT, "clientID").toString());

  // handle CORS requests here
  if ((request->method() == QHttpRequest::HTTP_OPTIONS) && headerMap.contains("access-control-request-method"))
  {    
    response->setHeader("Content-Type", "text/plain");
    response->setHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT, HEAD");
    response->setHeader("Access-Control-Max-Age", "1209600");
    response->setHeader("Connection", "close");

    if (headerMap.contains("access-control-request-headers"))
    {
      response->setHeader("Access-Control-Allow-Headers", headerMap["access-control-request-headers"].toString());
    }

    response->writeHead(QHttpResponse::STATUS_OK);
    response->end();
    return;
  }

  // we want to handle the subscription events in the host
  // since we are going to handle the updating later.
  //
  if (request->path() == "/player/timeline/subscribe")
  {
    handleSubscription(request, response, false);
    return;
  }
  else if (request->path() == "/player/timeline/unsubscribe")
  {
    subscriberRemove(request->headers()["x-plex-client-identifier"]);
    response->writeHead(QHttpResponse::STATUS_OK);
    response->end();
    return;
  }
  else if ((request->path() == "/player/timeline/poll"))
  {
    if (m_subscriberMap.contains(identifier) == false)
      handleSubscription(request, response, true);

    RemotePollSubscriber *subscriber = (RemotePollSubscriber *)m_subscriberMap[identifier];
    subscriber->reSubscribe();
    subscriber->setHTTPResponse(response);

    // if we don't have to wait, just ship the update right away
    // otherwise, this will wait until next update
    if (!(queryMap.contains("wait") && ( queryMap["wait"].toList()[0].toInt() == 1)))
    {
     subscriber->sendUpdate();
    }

    return;
  }


  // handle commandID
  if (headerMap.contains("x-plex-client-identifier") == false || queryMap.contains("commandID") == false)
  {
    QLOG_WARN() << "Can't find a X-Plex-Client-Identifier header";
    response->writeHead(QHttpResponse::STATUS_NOT_ACCEPTABLE);
    response->end();
    return;
  }

  quint64 commandId = 0;
  {
    QMutexLocker lk(&m_responseLock);
    commandId = ++m_commandId;
    m_responseMap[commandId] = response;

    connect(response, &QHttpResponse::done, this, &RemoteComponent::responseDone);
  }

  {
    QMutexLocker lk(&m_subscriberLock);
    if (m_subscriberMap.contains(identifier) == false)
    {
      QLOG_WARN() << "Failed to lock up subscriber" << identifier;
      response->writeHead(QHttpResponse::STATUS_NOT_ACCEPTABLE);
      response->end();
      return;
    }

    RemoteSubscriber* subscriber = m_subscriberMap[identifier];
    subscriber->setCommandId(m_commandId, queryMap["commandID"].toList()[0].toInt());
  }

  QVariantMap arg = {
    { "method", request->methodString() },
    { "headers", headerMap },
    { "path", request->path() },
    { "query", queryMap },
    { "commandID", m_commandId}
  };

  emit commandReceived(arg);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::responseDone()
{
  QHttpResponse* response = dynamic_cast<QHttpResponse*>(sender());
  if (response)
  {
    QMutexLocker lk(&m_responseLock);

    int foundId = -1;
    foreach(int responseId, m_responseMap.keys())
    {
      if (m_responseMap[responseId] == response)
      {
        foundId = responseId;
        break;
      }
    }

    if (foundId != -1)
      m_responseMap.remove(foundId);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::commandResponse(const QVariantMap& responseArguments)
{
  // check for minimum requirements in the responseArguments
  if (responseArguments.contains("commandID") == false ||
      responseArguments.contains("responseCode") == false)
  {
    QLOG_WARN() << "responseArguments did not contain a commandId or responseCode";
    return;
  }

  quint64 commandId = responseArguments["commandID"].toULongLong();
  uint responseCode = responseArguments["responseCode"].toUInt();

  QMutexLocker lk(&m_responseLock);
  if (m_responseMap.contains(commandId) == false)
  {
    QLOG_WARN() << "Could not find responseId:" << commandId << " - maybe it was removed because of a timeout?";
    return;
  }

  QHttpResponse* response = m_responseMap[commandId];

  // no need to hold the lock when we have changed m_responseMap
  lk.unlock();

  // add headers if we have them
  if (responseArguments.contains("headers") && responseArguments["headers"].type() == QVariant::Map)
  {
    QVariantMap headers = responseArguments["headers"].toMap();
      foreach (const QString& key, headers.keys())
        response->setHeader(key, headers[key].toString());
  }

  // write the response HTTP code
  response->writeHead(responseCode);

  // handle optional body argument
  if (responseArguments.contains("body") && responseArguments["body"].type() == QVariant::String)
    response->write(responseArguments["body"].toString().toUtf8());

  response->end();
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::handleSubscription(QHttpRequest* request, QHttpResponse* response, bool poll)
{
  QVariantMap headers = HeaderToMap(request->headers());

  // check for required headers
  if (headers.contains("x-plex-client-identifier") == false ||
      headers.contains("x-plex-device-name") == false)
  {
    QLOG_ERROR() << "Missing X-Plex headers in /timeline/subscribe request";
    response->writeHead(QHttpResponse::STATUS_BAD_REQUEST);
    response->end();
    return;
  }

  // check for required arguments
  QVariantMap query = QueryToMap(request->url());

  if (query.contains("commandID") == false || ((query.contains("port") == false) && !poll))
  {
    QLOG_ERROR() << "Missing arguments to /timeline/subscribe request";
    response->writeHead(QHttpResponse::STATUS_BAD_REQUEST);
    response->end();
    return;
  }

  QString clientIdentifier(request->headers()["x-plex-client-identifier"]);

  QMutexLocker lk(&m_subscriberLock);
  RemoteSubscriber* subscriber = 0;

  if (m_subscriberMap.contains(clientIdentifier))
  {
    QLOG_DEBUG() << "Refreshed subscriber:" << clientIdentifier;
    subscriber = m_subscriberMap[clientIdentifier];
    subscriber->reSubscribe();
  }
  else
  {
    if (poll)
    {
      QLOG_DEBUG() << "New poll subscriber:" << clientIdentifier << request->headers()["x-plex-device-name"];
      subscriber = new RemotePollSubscriber(clientIdentifier, request->headers()["x-plex-device-name"], response, this);
    }
    else
    {
      QUrl address;
      QString protocol = query.contains("protocol") ? query["protocol"].toList()[0].toString() : "http";
      int port = query.contains("port") ? query["port"].toList()[0].toInt() : 32400;

      address.setScheme(protocol);
      address.setHost(request->remoteAddress());
      address.setPort(port);

      QLOG_DEBUG() << "New subscriber:" << clientIdentifier << request->headers()["x-plex-device-name"] << address.toString();
      subscriber = new RemoteSubscriber(clientIdentifier, request->headers()["x-plex-device-name"], address, this);
    }

    m_subscriberMap[clientIdentifier] = subscriber;

    // if it's our first controller, we notify web for subscription
    if (m_subscriberMap.size() == 1)
    {
      QLOG_DEBUG() << "First subscriber added, subscribing to web";
      subscribeToWeb(true);
    }
  }

  subscriber->setCommandId(m_commandId, query["commandID"].toList()[0].toInt());

  if (!poll)
  {
    subscriber->sendUpdate();

    response->writeHead(QHttpResponse::STATUS_OK);
    response->end();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::subscribeToWeb(bool subscribe)
{
  QVariantMap arg = {
    { "method", "GET" },
    { "path",  "/player/timeline/" +  (subscribe ? QLatin1String("subscribe") : QLatin1String("unsubscribe")) },
    { "commandID", m_commandId}
  };

  emit commandReceived(arg);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::checkSubscribers()
{
  QMutexLocker lk(&m_subscriberLock);
  QList<RemoteSubscriber*> subsToRemove;
  foreach(RemoteSubscriber* subscriber, m_subscriberMap.values())
  {
    // was it more than 10 seconds since this client checked in last?
    if (subscriber->lastSubscribe() > 90 * 1000)
    {
      QLOG_DEBUG() << "more than 10 seconds since we heard from:" << subscriber->deviceName() << "- unsubscribing..";
      subsToRemove << subscriber;
    }
  }

  lk.unlock();

  foreach(RemoteSubscriber* sub, subsToRemove)
    subscriberRemove(sub->clientIdentifier());
}

/////////////////////////////////////////////////////////////////////////////////////////
QNetworkAccessManager* RemoteComponent::getNetworkAccessManager()
{
  // we might want to set common options here.
  return m_networkAccessManager;
}


/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::timelineFinished(QNetworkReply* reply)
{
  QString identifier = reply->request().attribute(QNetworkRequest::User).toString();

  // ignore requests with no identifier
  if (identifier.isEmpty())
    return;

  QMutexLocker lk(&m_subscriberLock);
  if (m_subscriberMap.contains(identifier) == false)
  {
    QLOG_WARN() << "Got a networkreply with a identifier we don't know about:" << identifier;
    return;
  }

  RemoteSubscriber* sub = m_subscriberMap[identifier];
  lk.unlock();

  sub->timelineFinished(reply);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::subscriberRemove(const QString& identifier)
{
  QMutexLocker lk(&m_subscriberLock);
  if (m_subscriberMap.contains(identifier) == false)
  {
    QLOG_ERROR() << "Can't remove client:" << identifier << "since we don't know about it.";
    return;
  }

  RemoteSubscriber* subscriber = m_subscriberMap[identifier];
  m_subscriberMap.remove(identifier);
  subscriber->deleteLater();

  // if it's our first controller, we notify web for subscription
  if (m_subscriberMap.size() == 0)
  {
    QLOG_DEBUG() << "Last subscriber removed, unsubscribing from web";
    subscribeToWeb(false);
  }
  QLOG_DEBUG() << "Removed subscriber:" << identifier;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RemoteComponent::timelineUpdate(quint64 commandID, const QString& timeline)
{
  QMutexLocker lk(&m_subscriberLock);

  foreach (RemoteSubscriber* subscriber, m_subscriberMap.values())
  {
    subscriber->queueTimeline(commandID, timeline.toUtf8());
    subscriber->sendUpdate();
  }
}
