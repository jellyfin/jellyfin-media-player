//
// Created by Tobias Hieta on 27/08/15.
//

#ifndef KONVERGO_UNIQUEAPPLICATION_H
#define KONVERGO_UNIQUEAPPLICATION_H

#include <QObject>
#include "Paths.h"
#include "LocalJsonServer.h"
#include "LocalJsonClient.h"
#include "utils/Utils.h"

#define SOCKET_NAME "pmpUniqueApplication"

class UniqueApplication : public QObject
{
  Q_OBJECT
public:
  explicit UniqueApplication(QObject* parent = nullptr, const QString& socketname = SOCKET_NAME);

  void listen();
  bool ensureUnique();
  
  // Send deeplink URL to running instance
  bool sendDeepLink(const QString& url);

Q_SIGNALS:
  void otherApplicationStarted();
  void deepLinkReceived(const QString& url);

private Q_SLOTS:
  void handleMessage(const QVariant& message);

private:
  LocalJsonServer* m_server;
  QString m_socketName;
};

#endif //KONVERGO_UNIQUEAPPLICATION_H
