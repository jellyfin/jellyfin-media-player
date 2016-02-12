//
// Created by Tobias Hieta on 24/03/15.
//

#ifndef _KONVERGO_REMOTECOMPONENT_H_
#define _KONVERGO_REMOTECOMPONENT_H_

#include <QJsonObject>
#include <QMutex>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "ComponentManager.h"
#include "GDMManager.h"
#include "server/HTTPServer.h"
#include "qhttpserverresponse.hpp"
#include "qhttpserver.hpp"
#include "RemoteSubscriber.h"

class RemoteComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(RemoteComponent);

public:
  QNetworkAccessManager* getNetworkAccessManager();

  virtual bool componentInitialize();
  virtual const char* componentName() { return "remote"; }
  virtual bool componentExport() { return true; }

  static QVariantMap ResourceInformation();
  static QVariantMap GDMInformation();
  static QVariantMap HeaderInformation();

  void handleResource(QHttpRequest* request, QHttpResponse* response);
  void handleCommand(QHttpRequest* request, QHttpResponse* response);

  static QVariantMap HeaderToMap(const qhttp::THeaderHash& hash);
  static QVariantMap QueryToMap(const QUrl& url);

  Q_INVOKABLE void commandResponse(const QVariantMap& responseArguments);
  Q_INVOKABLE QVariantMap resourceInfo() { return ResourceInformation(); }
  Q_INVOKABLE void timelineUpdate(quint64 commandID, const QString& timeline);

  void subscriberRemove(const QString& identifier);

Q_SIGNALS:
  void commandReceived(const QVariantMap& commandInfo);

private Q_SLOTS:
  void checkSubscribers();
  void timelineFinished(QNetworkReply* reply);
  void responseDone();

private:
  RemoteComponent(QObject* parent = 0);
  void handleSubscription(QHttpRequest * request, QHttpResponse * response, bool poll=false);
  void subscribeToWeb(bool subscribe);

  GDMManager* m_gdmManager;

  quint64 m_commandId;
  QMap<quint64, QHttpResponse*> m_responseMap;
  QMutex m_responseLock;

  QMutex m_subscriberLock;
  QMap<QString, RemoteSubscriber*> m_subscriberMap;
  QTimer m_subscriberTimer;
  QNetworkAccessManager* m_networkAccessManager;
};


#endif //_KONVERGO_REMOTECOMPONENT_H_
