#ifndef UPDATERCOMPONENT_H
#define UPDATERCOMPONENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QFile>
#include <QThread>
#include <QTime>
#include <time.h>

#include "ComponentManager.h"
#include "QsLog.h"

#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
class Update : public QObject
{
  Q_OBJECT
public:
  explicit Update(const QString& url = "", const QString& localPath = "",
         const QString& hash = "", QObject* parent = nullptr) : QObject(parent)
  {
    m_url = url;
    m_localPath = localPath;
    m_hash = hash;
    m_reply = nullptr;
    m_openFile = new QFile(m_localPath);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  bool setReply(QNetworkReply* reply)
  {
    if (m_reply)
    {
      disconnect(m_reply, nullptr, nullptr, nullptr);
      m_reply->deleteLater();
      m_reply = nullptr;
      m_openFile->close();
    }

    m_reply = reply;
    m_timeStarted = time(nullptr);

    connect(m_reply, &QNetworkReply::readyRead, this, &Update::write);
    connect(m_reply, &QNetworkReply::finished, this, &Update::finished);

    if (m_openFile->open(QFile::WriteOnly))
      return true;

    m_reply->deleteLater();
    m_reply = nullptr;

    return false;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  void write()
  {
    m_openFile->write(m_reply->read(m_reply->bytesAvailable()));
    QThread::yieldCurrentThread();
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  void finished()
  {
    m_openFile->close();
    m_reply->deleteLater();
    m_reply = nullptr;

    QLOG_DEBUG() << "Update downloaded, took:" << time(nullptr) - m_timeStarted << "seconds";

    emit fileDone(this);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  bool isReady()
  {
    if ((m_reply && m_reply->isRunning()) ||
        (m_openFile && m_openFile->isOpen()))
      return false;

    QFile file(m_localPath);
    if (file.exists())
    {
      QString fileHash = hashFile();
      if (!fileHash.isEmpty() && fileHash == m_hash)
        return true;
    }

    return false;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  QString hashFile()
  {
    QFile file(m_localPath);
    QCryptographicHash hash(QCryptographicHash::Sha1);

    if (file.open(QFile::ReadOnly))
    {
      while (!file.atEnd())
        hash.addData(file.read(8192));

      QByteArray binhash = hash.result();
      return binhash.toHex();
    }

    return "";
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  void abort()
  {
    if (m_reply)
      m_reply->abort();
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  QString m_url;
  QString m_localPath;
  QString m_hash;

  QNetworkReply* m_reply;
  QFile* m_openFile;
  time_t m_timeStarted;

signals:
  void fileDone(Update* update);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class UpdaterComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(UpdaterComponent);

public:
  bool componentExport() override { return true; }
  const char* componentName() override { return "updater"; }
  bool componentInitialize() override { return true; }

  // Disable old API for now
  Q_INVOKABLE void downloadUpdate(const QVariantMap &updateInfo) { };

  Q_INVOKABLE void checkForUpdate();
  Q_INVOKABLE void startUpdateDownload(const QVariantHash& updateInfo);

  Q_INVOKABLE void doUpdate();

  const QVariantHash& updateInfo() const { return m_updateInfo; }

signals:
  void downloadError(const QString& error);
  void downloadComplete(const QString& version);
  void downloadProgress(qint64 bytesReceived, qint64 total);

private slots:
  void dlComplete(QNetworkReply *reply);
  bool fileComplete(Update *update);

private:
  explicit UpdaterComponent(QObject *parent = nullptr);

  bool isDownloading();
  void downloadFile(Update *update);

  QString m_version;

  Update* m_manifest;
  Update* m_file;
  bool m_hasManifest;

  QNetworkAccessManager m_netManager;
  QNetworkReply* m_checkReply;
  QByteArray m_checkData;

  QVariantHash parseUpdateData(const QByteArray& data);
  QString getFinalUrl(const QString& path);

  QVariantHash m_updateInfo;
  QTime m_lastUpdateCheck;
};

#endif // UPDATERCOMPONENT_H
