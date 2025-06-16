#include "DiscordComponent.h"
#include "DatabaseComponent.h"
#include "player/PlayerComponent.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QTimer>
#include <csignal>
#include <iostream>
#include <thread>

DiscordComponent::DiscordComponent(QObject* parent) : ComponentBase(parent) {}

void DiscordComponent::valuesUpdated(const QVariantMap& values)
{
  m_richPresenceEnabled =
  SettingsComponent::Get().value(SETTINGS_SECTION_OTHER, "Discord_Integration").toBool();
  qDebug() << "[DiscordSettings] State: " << m_richPresenceEnabled;

  if (m_tryConnectTimer.get() == nullptr)
  {
    qDebug() << "[DiscordSettings] ConnectTimer not avaliable";
    return;
  }

  if (m_richPresenceEnabled)
  {
    m_tryConnectTimer->start();
    qDebug() << "[DiscordSettings] Starting Discord Rich Presence";
  }
  else
  {
    m_tryConnectTimer->stop();
    setIsDisconnected();
    Discord_Shutdown();
    qDebug() << "[DiscordSettings] Discord Rich Presence is disabled";
  }
}

bool DiscordComponent::componentInitialize()
{
  // callback Timer needs to be running the entire time, otherwise we wont receive any feedback from
  // discord (including connecting and disconnecting)
  m_callbackTimer = std::make_unique<QTimer>(new QTimer(this));
  m_callbackTimer->setInterval(1000);
  QObject::connect(m_callbackTimer.get(), SIGNAL(timeout()), this, SLOT(runCallbacks()));
  m_callbackTimer->start();

  // set the handle functions to react to connect, disconnect and error
  memset(&m_handlers, 0, sizeof(m_handlers));
  m_handlers.ready = handleDiscordReady;
  m_handlers.disconnected = handleDiscordDisconnected;
  m_handlers.errored = handleDiscordError;

  memset(&m_discordPresence, 0, sizeof(m_discordPresence));

  connect(&PlayerComponent::Get(), &PlayerComponent::playing, this, &DiscordComponent::onPlaying);
  connect(&PlayerComponent::Get(), &PlayerComponent::onMetaData, this,
          &DiscordComponent::onMetaData);
  connect(&PlayerComponent::Get(), &PlayerComponent::updateDuration, this,
          &DiscordComponent::onUpdateDuration);
  connect(&PlayerComponent::Get(), &PlayerComponent::positionUpdate, this,
          &DiscordComponent::onPositionUpdate);
  connect(&PlayerComponent::Get(), &PlayerComponent::stopped, this, &DiscordComponent::onStop);
  connect(&PlayerComponent::Get(), &PlayerComponent::paused, this, &DiscordComponent::onPause);

  // Background Timer trying to periodically reconnect to discord
  m_tryConnectTimer = std::make_unique<QTimer>(new QTimer(this));
  QObject::connect(m_tryConnectTimer.get(), &QTimer::timeout, this, &DiscordComponent::tryConnect);
  m_tryConnectTimer->setInterval(10000);

  if (m_richPresenceEnabled)
  {
    m_tryConnectTimer->start();
  }

  SettingsSection* discordSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_OTHER);
  connect(discordSection, &SettingsSection::valuesUpdated, this, &DiscordComponent::valuesUpdated);

  return true;
}

void DiscordComponent::setIsConnected()
{
  m_tryConnectTimer->stop();
  updateActivity(m_currentState);
}

void DiscordComponent::setIsDisconnected()
{
  if (m_richPresenceEnabled)
  {
    m_tryConnectTimer->start();
  }
}

void DiscordComponent::updateActivity(State state)
{
  m_currentState = state;

  switch (state)
  {
    case State::PLAYING:
      makeWatchingActivity(state);
      break;
    case State::PAUSED:
    case State::MENU:
      Discord_ClearPresence();
      break;
    default:
      Discord_ClearPresence();
      break;
  }
}

void DiscordComponent::updateRichPresence()
{
  // Update rich presence
  Discord_UpdatePresence(&m_discordPresence);
}

void DiscordComponent::makeWatchingActivity(State watchingState)
{
  QString state;
  QString details;
  QString thumbnailUrl;
  qDebug() << "METADATA " << metadata;
  if (metadata["Type"].toString() == "Movie")
  {
    m_discordPresence.activityType = DiscordActivityType::WATCHING;
    details = QString(metadata["Name"].toString());
    state = QString("Watching");
    thumbnailUrl =
    QString("%1/Items/%2/Images/Primary").arg(m_baseUrl.toString(), metadata["Id"].toString());
    if (downloadAndUpload(thumbnailUrl.toStdString().c_str(), m_imgurLink))
    {
      qDebug() << "IMGUR LINK: " << m_imgurLink.c_str();
      m_discordPresence.largeImageKey = m_imgurLink.c_str();
    }
    else
    {
      m_discordPresence.largeImageKey = "movie";
    }
  }
  if (metadata["Type"].toString() == "Episode")
  {
    m_discordPresence.activityType = DiscordActivityType::WATCHING;
    state = QString("%1").arg(metadata["Name"].toString());
    details =
    QString("%1 : %2").arg(metadata["SeriesName"].toString(), metadata["SeasonName"].toString());
    thumbnailUrl = QString("%1/Items/%2/Images/Primary")
                   .arg(m_baseUrl.toString(), metadata["ParentBackdropItemId"].toString());
    qDebug() << "THUMBNAIL URL: " << thumbnailUrl;
    if (downloadAndUpload(thumbnailUrl.toStdString().c_str(), m_imgurLink))
    {
      m_discordPresence.largeImageKey = m_imgurLink.c_str();
    }
    else
    {
      m_discordPresence.largeImageKey = "show";
    }
  }
  if (metadata["Type"].toString() == "Audio" || metadata["Type"].toString() == "AudioBook")
  {
    QStringList artistNames;
    m_discordPresence.activityType = DiscordActivityType::LISTENING;
    if (metadata.contains("Artists"))
    {
      QVariantList artistsList = metadata["Artists"].toList();
      for (const QVariant& artist : artistsList)
      {
        artistNames << artist.toString();
      }
      artistNames.removeDuplicates();

      state = artistNames.join(", ");
    }
    else
    {
      state = "Unknown Artist";
    }
    details = metadata["Name"].toString();
    qDebug() << "DETAILS " << details;
    qDebug() << "STATE " << state;
    thumbnailUrl =
    QString("%1/Items/%2/Images/Primary").arg(m_baseUrl.toString(), metadata["Id"].toString());
    qDebug() << "THUMBNAIL URL: " << thumbnailUrl;
    if (metadata["Type"].toString() == "Audio")
    {
      if (downloadAndUpload(thumbnailUrl.toStdString().c_str(), m_imgurLink))
      {
        m_discordPresence.largeImageKey = m_imgurLink.c_str();
      }
      else
      {
        m_discordPresence.largeImageKey = "music";
      }
    }
    else if (metadata["Type"].toString() == "AudioBook")
    {
      if (downloadAndUpload(thumbnailUrl.toStdString().c_str(), m_imgurLink))
      {
        m_discordPresence.largeImageKey = m_imgurLink.c_str();
      }
      else
      {
        m_discordPresence.largeImageKey = "audiobook";
      }
    }
  }
  qDebug() << "IMGUR LINK: " << m_imgurLink.c_str();
  qint64 currentEpochMs = QDateTime::currentMSecsSinceEpoch();
  qint64 startTimeSeconds = (currentEpochMs - m_position); // When playback actually started
  qint64 endTimeSeconds = 0;

  if (m_duration > 0)
  {
    endTimeSeconds = (currentEpochMs + (m_duration - m_position)); // When playback will finish
  }

  m_discordPresence.startTimestamp = startTimeSeconds;

  if (endTimeSeconds > 0)
  {
    m_discordPresence.endTimestamp = endTimeSeconds;
  }

  m_discordPresence.smallImageKey = "jellyfin";
  std::string tmp_details = details.toStdString();
  std::string tmp_state = state.toStdString();
  m_discordPresence.details = tmp_details.c_str();
  m_discordPresence.state = tmp_state.c_str();
  updateRichPresence();
}

void DiscordComponent::onPlaying() { updateActivity(State::PLAYING); }

void DiscordComponent::onStop() { updateActivity(State::MENU); }

void DiscordComponent::onPause() { updateActivity(State::PAUSED); }

void DiscordComponent::onFinished() { updateActivity(State::MENU); }

void DiscordComponent::onUpdateDuration(qint64 duration) { m_duration = duration; }

void DiscordComponent::onPositionUpdate(quint64 position) { m_position = position; }

void DiscordComponent::onMetaData(const QVariantMap& meta, QUrl baseUrl)
{
  metadata = meta;
  m_position = 0;
  m_baseUrl = baseUrl;
}

void DiscordComponent::handleDiscordReady(const DiscordUser* connectedUser)
{
  qDebug() << "Discord: connected to user";
  DiscordComponent::Get().setIsConnected();
}

void DiscordComponent::handleDiscordDisconnected(int errcode, const char* message)
{
  qDebug() << "Discord: disconnected";
  DiscordComponent::Get().setIsDisconnected();
}

void DiscordComponent::handleDiscordError(int errcode, const char* message)
{
  qDebug() << "Discord Error [" << errcode << "] occured: " << message;
}

void DiscordComponent::tryConnect()
{
  qDebug() << "[DiscordComponent] trying to Connect to Discord! ";
  Discord_Initialize(this->APPLICATION_ID, &m_handlers, 1, NULL);
}

void DiscordComponent::runCallbacks() { Discord_RunCallbacks(); }

const char* DiscordComponent::componentName() { return "DiscordComponent"; }

bool DiscordComponent::componentExport() { return true; }

bool DiscordComponent::downloadAndUpload(const std::string& imageUrl, std::string& response)
{
  Imgur uploader;
  std::vector<char> imageData = uploader.downloadImage(imageUrl);

  // Hashing the imageData
  QByteArray byteArray(imageData.data(), static_cast<int>(imageData.size()));
  QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
  QString hashHex = hash.toHex();

  if (imageData.empty())
  {
    return false;
  }

  // Check if the URL for this hash already exists in the database
  QString resultURL = DatabaseComponent::Get().getUrlForHash(hashHex);
  if (!resultURL.isEmpty())
  {
    qDebug() << "Found URL for hash" << hashHex << ": " << resultURL;
    response = resultURL.toStdString();
    std::vector<char> downloadData = uploader.downloadImage(resultURL.toStdString());
  }
  else
  {
    std::string imgur_link;
    uploader.uploadRaw(imageData, imgur_link);
    DatabaseComponent::Get().storeUrl(hashHex, QString::fromStdString(imgur_link));
    response = imgur_link;
  }
  return true;
}