//
// Created by Tobias Hieta on 26/08/15.
//

#include "CrashUploader.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QEventLoop>
#include <QFileInfo>
#include <QSysInfo>

#include "Paths.h"
#include "Version.h"
#include "utils/Utils.h"
#include "QsLog.h"
#include "HelperSettings.h"

#define UPLOAD_URL "https://crashdump.plex.tv"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Delete crash dumps that are not from the current version. They are most likely useless.
void CrashUploader::deleteOldCrashDumps()
{
  QDir dir(m_old);
  for(const QString& entry : dir.entryList(QDir::Dirs | QDir::NoDot | QDir::NoDotDot))
  {
    if (entry != Version::GetCanonicalVersionString())
    {
      QDir subdir = dir;
      if (subdir.cd(entry))
      {
        QLOG_INFO() << "Wiping uninteresting crashdumps:" << subdir.path();
        subdir.removeRecursively();
      }
    }
  }

  // clean out any things that are in progress.
  QDir progressDir(m_processing);
  progressDir.removeRecursively();
  QDir().mkpath(m_processing);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CrashUploader::addFormField(QHttpMultiPart* multipart, const QString& name, const QString& value)
{
  QHttpPart textPart;
  textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + name + "\""));
  textPart.setBody(value.toUtf8());
  multipart->append(textPart);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CrashUploader::uploadCrashDump(const QString& version, const QString& path)
{
  // move the file to the in progress directory so that we don't accidentially upload it twice
  QString inProgressDir = m_processing + "/" + version + "/";
  QDir().mkpath(inProgressDir);

  QString inProgressPath = inProgressDir + QFileInfo(path).fileName();
  QString uuid = QFileInfo(path).baseName();
  QFile file(path);
  file.rename(inProgressPath);
  file.setFileName(inProgressPath);

  QLOG_DEBUG() << "Crashdump:" << inProgressPath;

  if (!file.open(QIODevice::ReadOnly))
  {
    QLOG_ERROR() << "Could not open crashdump file. will try again later.";
    moveFileBackToIncoming(version, inProgressPath);
    m_scanTimer->start(5000);
    return;
  }

  QLOG_INFO() << "Uploading crashdump:" << inProgressPath;

  QNetworkRequest req(QUrl(UPLOAD_URL));
  req.setRawHeader("X-Plex-Secret", CRASHDUMP_SECRET);

  auto multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

  addFormField(multiPart, "version", version);
  addFormField(multiPart, "product", "plexmediaplayer");
  addFormField(multiPart, "uuid", uuid);
  addFormField(multiPart, "serverUuid", HelperSettings().value("clientId", "NOCLIENTID").toString());
  addFormField(multiPart, "userId", HelperSettings().value("userId", "NOUSERID").toString());
  addFormField(multiPart, "platform", QSysInfo::productType() + "-" + QSysInfo::currentCpuArchitecture());

  QHttpPart dataPart;
  dataPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
  dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"dumpfileb64\""));
  dataPart.setBody(file.readAll().toBase64());
  multiPart->append(dataPart);

  QNetworkReply* reply = m_manager->post(req, multiPart);
  multiPart->setParent(reply);

  connect(reply, &QNetworkReply::sslErrors, [=](const QList<QSslError> & errors)
  {
    QLOG_WARN() << "SSL errors:" << errors;
  });

  connect(reply, static_cast<void (QNetworkReply::*)()>(&QNetworkReply::finished), [=]()
  {
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    // The only situation in which we retry a failed crash dump upload is when
    // we get a 503 http status code (we got throttled because we are sending
    // crashes to quickly), or if the network was unavailable (no status code).
    // If the server returns any other error, it doesn't want the crash report,
    // and we just drop it.
    //
    if (!statusCode.isValid() || statusCode.toInt() == 503)
    {
      QLOG_WARN() << "Failed to submit report with uuid:" << uuid << "will try again later";
      moveFileBackToIncoming(version, inProgressPath);
      m_scanTimer->start(5000);
      return;
    }

    watchCrashDir(false);
    QDir().mkpath(m_old + "/" + version);
    QFile::rename(inProgressPath, m_old + "/" + version + "/" + QFileInfo(inProgressPath).fileName());
    watchCrashDir(true);

    if (statusCode.toInt() == 200)
    {
      QLOG_INFO() << "Submitted crash report with uuid:" << uuid;
    }
    else
    {
      QLOG_INFO() << "Server didn't want our crash report with uuid:" << uuid << "giving HTTP status" << statusCode.toInt() << "- saving it in old for now";
    }
  });
}

/////////////////////////////////////////////////////////////////////////////////////////
void CrashUploader::moveFileBackToIncoming(const QString& version, const QString& filename)
{
  watchCrashDir(false);
  QFile::rename(filename, incomingCurrentVersion() + "/" + QFileInfo(filename).fileName());
  watchCrashDir(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CrashUploader::uploadAndDeleteCrashDumps()
{
  QMutexLocker lk(&m_scanLock);
  QDir dir(m_incoming);

  QStringList filters;
  filters << "*.dmp";

  QLOG_DEBUG() << "Scanning:" << dir.path() << "for crashdumps.";

  watchCrashDir(false);

  // loop over all incoming directories, should give us version numbers in d
  for(const QString& version : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
  {
    QDir versionDir(dir);
    versionDir.cd(version);

    QLOG_DEBUG() << "Scanning:" << versionDir.path();

    int numUploads = 0;
    for(const QString& entry : versionDir.entryList(filters, QDir::Files))
    {
      // we only upload 20 crash reports per version
      if (numUploads > 20)
      {
        QLOG_DEBUG() << "We have uploaded more than 20 crash reports, removing:" << entry;
        QFile::remove(versionDir.filePath(entry));
      }

      uploadCrashDump(version, versionDir.filePath(entry));
      numUploads ++;
    }
  }

  watchCrashDir(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CrashUploader::watchCrashDir(bool watch)
{
  if (watch)
    m_watcher->addPath(incomingCurrentVersion());
  else
    m_watcher->removePath(incomingCurrentVersion());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CrashUploader::CrashUploader(QObject* parent) : QObject(parent)
{
  m_manager = new QNetworkAccessManager(this);
  m_watcher = new QFileSystemWatcher(this);
  m_scanTimer = new QTimer(this);
  m_scanTimer->setSingleShot(true);

  connect(m_scanTimer, &QTimer::timeout, this, &CrashUploader::uploadAndDeleteCrashDumps);

  connect(m_watcher, &QFileSystemWatcher::directoryChanged, [=](const QString& dir)
  {
    // wait 2 seconds before starting process dumps.
    m_scanTimer->start(2000);
  });

  m_old = Paths::cacheDir("crashdumps/old");
  m_processing = Paths::cacheDir("crashdumps/processing");
  m_incoming = Paths::cacheDir("crashdumps/incoming");

  startUploader();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool CrashUploader::startUploader()
{
  deleteOldCrashDumps();
  uploadAndDeleteCrashDumps();

  m_watcher->addPath(incomingCurrentVersion());

  return true;
}
