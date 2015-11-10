#include "UpdaterComponent.h"
#include "QsLog.h"
#include "utils/Utils.h"

#include <QTimer>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QIODevice>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStringList>
#include <utils/HelperLauncher.h>

#include "settings/SettingsComponent.h"
#include "UpdateManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
UpdaterComponent::UpdaterComponent(QObject *parent) : ComponentBase(parent), m_netManager(this)
{
  m_file = NULL;
  m_manifest = NULL;

  connect(&m_netManager, &QNetworkAccessManager::finished,
          this, &UpdaterComponent::dlComplete);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UpdaterComponent::dlComplete(QNetworkReply* reply)
{
  QLOG_DEBUG() << "Hello, dlComplete";
  if (reply->error() == QNetworkReply::NoError)
  {
    QUrl redirectURL = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    QLOG_DEBUG() << "Redirect:" << redirectURL.toString();
    if (!redirectURL.isEmpty())
    {
      // redirection, check that we get redirected to something we expect.
      if (redirectURL.toString().startsWith("https://nightlies.plex.tv") ||
          redirectURL.toString().startsWith("https://downloads.plex.tv"))
      {
        QNetworkReply* redirReply = m_netManager.get(QNetworkRequest(redirectURL));
        if (m_manifest->m_reply == reply)
          m_manifest->setReply(redirReply);
        else if (m_file->m_reply == reply)
        {
          m_file->setReply(redirReply);
          //connect(redirReply, &QNetworkReply::downloadProgress, this, &UpdaterComponent::downloadProgress);
        }

        QLOG_DEBUG() << "Redirecting to:" << redirectURL.toString();

        return;
      }
    }
  }
  else
  {
    QLOG_ERROR() << "Error downloading:" << reply->url() << "-" << reply->errorString();
    emit downloadError(reply->errorString());

    if (m_hasManifest)
      m_manifest->abort();

    m_file->abort();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UpdaterComponent::downloadFile(Update* update)
{
  QNetworkRequest request(update->m_url);
  request.setPriority(QNetworkRequest::LowPriority);
  request.setAttribute(QNetworkRequest::BackgroundRequestAttribute, true);
  if (update->setReply(m_netManager.get(request)))
    QLOG_INFO() << "Downloading update:" << update->m_url << "to:" << update->m_localPath;
  else
    QLOG_ERROR() << "Failed to start download:" << update->m_url;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
bool UpdaterComponent::fileComplete(Update* update)
{
  Q_UNUSED(update);
  if (m_file->isReady() && (m_manifest->isReady() || !m_hasManifest))
  {
    QLOG_DEBUG() << "Both files downloaded";
    // create a file that shows that we are ready
    // to apply this update
    //
    QFile readyFile(UpdateManager::GetPath("_readyToApply", m_version, false));
    if (readyFile.open(QFile::WriteOnly))
    {
      readyFile.write("FOO");
      readyFile.close();
    }

    emit downloadComplete(m_version);

    delete m_file;
    delete m_manifest;

    m_file = NULL;
    m_manifest = NULL;

    return true;
  }

  if (update)
  {
    QLOG_DEBUG() << "File " << update->m_localPath << " download completed";
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UpdaterComponent::downloadUpdate(const QVariantMap& updateInfo)
{
  if (isDownloading())
    return;

  QLOG_INFO() << updateInfo;

  if (!updateInfo.contains("version") ||
      !updateInfo.contains("manifestURL") || !updateInfo.contains("manifestHash") ||
      !updateInfo.contains("fileURL") || !updateInfo.contains("fileHash") || !updateInfo.contains("fileName"))
  {
    QLOG_ERROR() << "updateInfo was missing fields required to carry out this action.";
    return;
  }

  m_version = updateInfo["version"].toString();

  m_manifest = new Update(updateInfo["manifestURL"].toString(),
                          UpdateManager::GetPath("manifest.xml.bz2", m_version, false),
                          updateInfo["manifestHash"].toString(), this);

  // determine if we have a manifest (some distros don't like OE)
  m_hasManifest = ((!m_manifest->m_url.isEmpty()) && (!m_manifest->m_hash.isEmpty()));

  m_file = new Update(updateInfo["fileURL"].toString(),
                      UpdateManager::GetPath(updateInfo["fileName"].toString(), m_version, true),
                      updateInfo["fileHash"].toString(), this);


  if (m_hasManifest)
    connect(m_manifest, &Update::fileDone, this, &UpdaterComponent::fileComplete);

  connect(m_file, &Update::fileDone, this, &UpdaterComponent::fileComplete);

  // create directories we need
  QDir dr(QFileInfo(m_file->m_localPath).dir());
  if (!dr.exists())
  {
    if (!dr.mkpath("."))
    {
      QLOG_ERROR() << "Failed to create update directory:" << dr.absolutePath();
      emit downloadError("Failed to create download directory");
      return;
    }
  }

  // this will first check if the files are done
  // and in that case emit the done signal.
  if (fileComplete(NULL))
    return;

  if (!m_manifest->isReady() && m_hasManifest)
    downloadFile(m_manifest);

  if (!m_file->isReady())
    downloadFile(m_file);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool UpdaterComponent::isDownloading()
{
  if ((m_manifest && m_manifest->m_reply && m_manifest->m_reply->isRunning()) ||
      (m_file && m_file->m_reply && m_file->m_reply->isRunning()))
    return true;

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UpdaterComponent::doUpdate()
{
  // make sure we kill off the helper first:
  HelperLauncher::Get().stop();
  UpdateManager::Get()->doUpdate(m_version);
}


