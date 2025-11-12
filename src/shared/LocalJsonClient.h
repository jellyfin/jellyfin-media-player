//
// Created by Tobias Hieta on 30/08/15.
//

#ifndef KONVERGO_LOCALJSONCLIENT_H
#define KONVERGO_LOCALJSONCLIENT_H

#include <QLocalSocket>
#include <QVariant>

class LocalJsonClient : public QLocalSocket
{
  Q_OBJECT
public:
  explicit LocalJsonClient(const QString serverPath, QObject* parent = nullptr);
  void connectToServer();
  bool sendMessage(const QVariantMap& message);

Q_SIGNALS:
  void messageReceived(const QVariantMap& message);

private:
  Q_SLOT void readyRead();
  QString m_serverPath;
};

#endif //KONVERGO_LOCALJSONCLIENT_H
