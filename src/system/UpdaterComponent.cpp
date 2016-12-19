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
#include <QUrlQuery>
#include <QDomDocument>
#include "qhttpclient.hpp"
#include "qhttpclientresponse.hpp"

#include "settings/SettingsComponent.h"
#include "UpdateManager.h"
#include "SystemComponent.h"

using namespace qhttp::client;

///////////////////////////////////////////////////////////////////////////////////////////////////
UpdaterComponent::UpdaterComponent(QObject* parent) :
  ComponentBase(parent),
  m_netManager(this),
  m_checkReply(nullptr),
  m_enabled(true)
{
  m_file = nullptr;
  m_manifest = nullptr;

#ifndef NDEBUG
  m_enabled = false;
#endif

  connect(&m_netManager, &QNetworkAccessManager::finished, this, &UpdaterComponent::dlComplete);

  connect(&SystemComponent::Get(), &SystemComponent::userInfoChanged, [&](){
    if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "automaticUpdates").toBool())
      QTimer::singleShot(10 * 1000, this, &UpdaterComponent::checkForUpdate);
  });

  auto updateTimer = new QTimer(this);
  connect(updateTimer, &QTimer::timeout, [&]{
    auto diff = m_lastUpdateCheck.secsTo(QTime::currentTime());
    QLOG_DEBUG() << "It has gone" << diff << "seconds since last update check.";
    if (diff >= (3 * 60 * 60))
      checkForUpdate();
  });

  if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "automaticUpdates").toBool())
    updateTimer->start(5 * 60 * 1000);
}

/////////////////////////////////////////////////////////////////////////////////////////
void UpdaterComponent::checkForUpdate()
{
  if (!m_enabled)
    return;

  auto systemInfo = SystemComponent::Get().systemInformation();
  QUrl baseUrl = QString("https://plex.tv/updater/products/%0/check.xml").arg(systemInfo["productid"].toInt());
  QUrlQuery query;

  query.addQueryItem("version", systemInfo["version"].toString());
  // Use the next line for debugging.
  // query.addQueryItem("version", "1.1.5.405-43e1569b");
  query.addQueryItem("build", systemInfo["build"].toString());
  query.addQueryItem("distribution", systemInfo["dist"].toString());
  query.addQueryItem("channel", SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "updateChannel").toString());

  auto authToken = SystemComponent::Get().authenticationToken();
  if (!authToken.isEmpty())
    query.addQueryItem("X-Plex-Token", authToken);

  baseUrl.setQuery(query);

  if (!m_checkReply)
  {
    QLOG_DEBUG() << "Checking for updates at:" << baseUrl.toString();
    QNetworkRequest req(baseUrl);
    req.setPriority(QNetworkRequest::HighPriority);
    m_checkReply = m_netManager.get(req);
    connect(m_checkReply, &QNetworkReply::readyRead, [&]()
    {
      m_checkData.append(m_checkReply->read(m_checkReply->bytesAvailable()));
    });
    connect(m_checkReply, &QNetworkReply::finished, [&]()
    {
      auto updateData = parseUpdateData(m_checkData);
      if (!updateData.isEmpty())
        startUpdateDownload(updateData);

      m_checkData.clear();
      m_checkReply = nullptr;
    });
  }

  m_lastUpdateCheck = QTime::currentTime();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UpdaterComponent::dlComplete(QNetworkReply* reply)
{
  if (reply->error() == QNetworkReply::NoError)
  {
    QUrl redirectURL = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirectURL.isEmpty())
    {
      QLOG_DEBUG() << "Redirect:" << redirectURL.toString();

      // redirection, check that we get redirected to something we expect.
      if (redirectURL.toString().startsWith("https://nightlies.plex.tv") ||
          redirectURL.toString().startsWith("https://downloads.plex.tv") ||
          redirectURL.toString().startsWith("https://plex.tv"))
      {
        QNetworkReply* redirReply = m_netManager.get(QNetworkRequest(redirectURL));
        if (m_manifest->m_reply == reply)
          m_manifest->setReply(redirReply);
        else if (m_file->m_reply == reply)
          m_file->setReply(redirReply);

        QLOG_DEBUG() << "Redirecting to:" << redirectURL.toString();

        return;
      }
    }
  }
  else
  {
    QLOG_ERROR() << "Error downloading:" << reply->url() << "-" << reply->errorString();
    emit downloadError(reply->errorString());

    if (m_hasManifest && m_manifest)
    {
      m_manifest->abort();
    }

    if (m_file)
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
  else QLOG_ERROR() << "Failed to start download:" << update->m_url;
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

    m_file = nullptr;
    m_manifest = nullptr;

    return true;
  }

  if (update)
  {
    QLOG_DEBUG() << "File " << update->m_localPath << " download completed";
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UpdaterComponent::startUpdateDownload(const QVariantHash& updateInfo)
{
  if (isDownloading())
  {
    return;
  }

  if (!updateInfo.contains("version") || !updateInfo.contains("manifestURL") || !updateInfo.contains("manifestHash") ||
      !updateInfo.contains("fileURL") || !updateInfo.contains("fileHash") || !updateInfo.contains("fileName"))
  {
    QLOG_ERROR() << "updateInfo was missing fields required to carry out this action.";
    return;
  }

  m_updateInfo = updateInfo;
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
  {
    connect(m_manifest, &Update::fileDone, this, &UpdaterComponent::fileComplete);
  }

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
  if (fileComplete(nullptr))
  {
    return;
  }

  if (!m_manifest->isReady() && m_hasManifest)
  {
    downloadFile(m_manifest);
  }

  if (!m_file->isReady())
  {
    downloadFile(m_file);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool UpdaterComponent::isDownloading()
{
  return (m_manifest && m_manifest->m_reply && m_manifest->m_reply->isRunning()) ||
         (m_file && m_file->m_reply && m_file->m_reply->isRunning());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UpdaterComponent::doUpdate()
{
  // make sure we kill off the helper first:
  HelperLauncher::Get().stop();
  UpdateManager::Get()->doUpdate(m_version);
}

#define BASE_PLEX_TV_URL "https://plex.tv"

/////////////////////////////////////////////////////////////////////////////////////////
QString UpdaterComponent::getFinalUrl(const QString& path)
{
  QUrl baseUrl(BASE_PLEX_TV_URL);
  baseUrl.setPath(path);

  auto token = SystemComponent::Get().authenticationToken();
  if (!token.isEmpty())
    baseUrl.setQuery("X-Plex-Token=" + token);

  return baseUrl.toString();
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariantHash UpdaterComponent::parseUpdateData(const QByteArray& data)
{
  QDomDocument doc("Update data");
  QVariantHash updateInfo;

  if (!doc.setContent(data))
  {
    QLOG_ERROR() << "Failed to parse check.xml data!";
    return updateInfo;
  }


  QVariantList releases;

  auto root = doc.documentElement();
  auto releaseElement = root.firstChildElement();
  while (!releaseElement.isNull())
  {
    if (releaseElement.nodeName() == "Release")
    {
      QVariantHash rel;
      rel["version"] = releaseElement.attribute("version");
      rel["fixed"] = releaseElement.attribute("fixed", "Nothing");
      rel["new"] = releaseElement.attribute("new", "Nothing");

      QVariantList packages;

      auto packageElement = releaseElement.firstChildElement();
      while (!packageElement.isNull())
      {
        if (packageElement.nodeName() == "Package")
        {
          QVariantHash package;
          package["delta"] = packageElement.attribute("delta");

          package["manifest"] = packageElement.attribute("manifest");
          package["manifestHash"] = packageElement.attribute("manifestHash");

          package["file"] = packageElement.attribute("file");
          package["fileHash"] = packageElement.attribute("fileHash");

          package["fileName"] = packageElement.attribute("fileName");

          if (package["delta"].toString() == "true")
            rel["delta_package"] = package;
          else
            rel["full_package"] = package;
        }
        packageElement = packageElement.nextSiblingElement();
      }

      releases.append(rel);
    }

    releaseElement = releaseElement.nextSiblingElement();
  }

  if (releases.isEmpty())
  {
    QLOG_DEBUG() << "No updates found!";
    return updateInfo;
  }

  QLOG_DEBUG() << releases;

  auto release = releases.last().toHash();

  if (release.contains("delta_package"))
    updateInfo = release["delta_package"].toHash();
  else
    updateInfo = release["full_package"].toHash();

  updateInfo["version"] = release["version"];
  updateInfo["fixed"] = release["fixed"];
  updateInfo["new"] = release["new"];
  updateInfo["fileURL"] = getFinalUrl(updateInfo["file"].toString());
  updateInfo["manifestURL"] = getFinalUrl(updateInfo["manifest"].toString());

  QLOG_DEBUG() << updateInfo;

  return updateInfo;
}


