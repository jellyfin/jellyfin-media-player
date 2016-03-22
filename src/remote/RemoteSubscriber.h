//
// Created by Tobias Hieta on 31/03/15.
//

#ifndef KONVERGO_REMOTESUBSCRIBER_H
#define KONVERGO_REMOTESUBSCRIBER_H

#include <QObject>
#include <QUrl>
#include <QDateTime>
#include <QDomDocument>
#include <QNetworkReply>
#include <QQueue>

#include "qhttpserverresponse.hpp"

/////////////////////////////////////////////////////////////////////////////////////////
class RemoteSubscriber : public QObject
{
public:
  RemoteSubscriber(const QString& clientIdentifier, const QString& deviceName, const QUrl& address, QObject* parent = nullptr);
  void reSubscribe();
  int lastSubscribe() const { return m_subscribeTime.elapsed(); }

  QString deviceName();
  QString clientIdentifier();
  virtual void sendUpdate();
  void timelineFinished(QNetworkReply* reply);
  void queueTimeline(quint64 playerCommandID, const QByteArray& timelineData);
  QByteArray getTimeline();
  void setCommandId(quint64 playerCommandId, quint64 controllerCommandId);

  quint64 mostRecentCommandId()
  {
    QMutexLocker lk(&m_commandIdMapLock);
    if (!m_commandIdQueue.isEmpty() && !m_commandIdMap.isEmpty())
      return m_commandIdMap[m_commandIdQueue.last()];
    else
      return 0;
  }

  quint64 commandId(quint64 playerCommandId)
  {
    QMutexLocker lk(&m_commandIdMapLock);
    if (m_commandIdMap.contains(playerCommandId))
      return m_commandIdMap[playerCommandId];
    lk.unlock();
    return mostRecentCommandId();
  }

private:
  QMutex m_commandIdMapLock;
  QHash<quint64, quint64> m_commandIdMap;
  QQueue<quint64> m_commandIdQueue;

  QNetworkAccessManager* m_netAccess;
  QUrl m_address;
  QTime m_subscribeTime;

  QMutex m_timelineLock;
  QDomDocument m_timeline;

protected:
  QString m_clientIdentifier;
  QString m_deviceName;

#if 0
  quint16 m_errors;
 #endif
};

/////////////////////////////////////////////////////////////////////////////////////////
class RemotePollSubscriber : public RemoteSubscriber
{
public:
  RemotePollSubscriber(const QString& clientIdentifier, const QString& deviceName, qhttp::server::QHttpResponse *response, QObject* parent = nullptr);
  void setHTTPResponse(qhttp::server::QHttpResponse *response);
  void sendUpdate() override;

private :
   qhttp::server::QHttpResponse* m_response;

public Q_SLOTS:
   void responseDone();
};

#endif //KONVERGO_REMOTESUBSCRIBER_H
