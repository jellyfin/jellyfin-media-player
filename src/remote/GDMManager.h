#ifndef GDMANNOUNCER_H
#define GDMANNOUNCER_H

#include <QObject>
#include <QUdpSocket>
#include <qbytearray.h>
#include <qhostaddress.h>
#include <qglobal.h>

class GDMManager : public QObject
{
  Q_OBJECT
public:
  explicit GDMManager(QObject *parent = nullptr);
  ~GDMManager() override { stopAnnouncing(); }
  void startAnnouncing();
  void stopAnnouncing();

private:
  void startListener();
  void parseData(const QByteArray& data, const QHostAddress& sender, quint16 port);
  void readData();

  QByteArray getPacket();

  QUdpSocket m_socket;
  qint32 m_port;
};

#endif // GDMANNOUNCER_H
