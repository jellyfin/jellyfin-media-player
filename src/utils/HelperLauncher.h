//
// Created by Tobias Hieta on 28/08/15.
//

#ifndef KONVERGO_HELPERLAUNCHER_H
#define KONVERGO_HELPERLAUNCHER_H

#include <QLocalSocket>
#include <QObject>
#include <QProcess>
#include <QJsonObject>

#include "LocalJsonClient.h"
#include "tools/helper/HelperSocket.h"
#include "Version.h"
#include "utils/Utils.h"

#ifdef Q_OS_MAC
#include "HelperLaunchd.h"
#endif

class HelperLauncher : public QObject
{
  Q_OBJECT
  DEFINE_SINGLETON(HelperLauncher)
public:
  bool connectToHelper();
  static QString HelperPath();
  void stop();
  void start();

private Q_SLOTS:
  void gotMessage(const QVariantMap& message);
  void socketError(QLocalSocket::LocalSocketError error);
  void didConnect();
  void launch();
  void socketDisconnect();
  bool killHelper();

private:
  explicit HelperLauncher(QObject* parent = 0);

  QProcess* m_helperProcess;
  LocalJsonClient* m_jsonClient;

  void updateClientId();

#ifdef Q_OS_MAC
  HelperLaunchd* m_launchd;
#endif
};

#endif //KONVERGO_HELPERLAUNCHER_H
