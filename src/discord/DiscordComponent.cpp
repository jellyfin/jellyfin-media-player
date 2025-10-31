#include "DiscordComponent.h"
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
    m_callbackTimer->start();
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
  // callback Timer needs to be running the while we are interacting with disocrd, otherwise we wont
  // receive any feedback from discord (including connecting and disconnecting)
  m_callbackTimer = std::make_unique<QTimer>(new QTimer(this));
  m_callbackTimer->setInterval(1000);
  QObject::connect(m_callbackTimer.get(), SIGNAL(timeout()), this, SLOT(runCallbacks()));

  // set the handle functions to react to connect, disconnect and error
  memset(&m_handlers, 0, sizeof(m_handlers));
  m_handlers.ready = handleDiscordReady;
  m_handlers.disconnected = handleDiscordDisconnected;
  m_handlers.errored = handleDiscordError;

  memset(&m_discordRichPresence, 0, sizeof(m_discordRichPresence));

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
    m_callbackTimer->start();
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
    m_callbackTimer->start();
    m_tryConnectTimer->start();
  }
  else
  {
    m_callbackTimer->stop();
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
  Discord_UpdatePresence(&m_discordRichPresence);
}

void DiscordComponent::makeWatchingActivity(State watchingState)
{
  QString state;
  QString details;
  qDebug() << "METADATA " << metadata;
  if (metadata["Type"].toString() == "Movie")
  {
    m_discordRichPresence.activityType = DiscordActivityType::WATCHING;
    details = QString(metadata["Name"].toString());
    state = QString("Watching a movie");
  }
  else if (metadata["Type"].toString() == "Episode")
  {
    m_discordRichPresence.activityType = DiscordActivityType::WATCHING;
    state = QString("%1").arg(metadata["Name"].toString());
    details =
    QString("%1 : %2").arg(metadata["SeriesName"].toString(), metadata["SeasonName"].toString());
  }
  else if (metadata["Type"].toString() == "Audio" || metadata["Type"].toString() == "AudioBook")
  {
    QStringList artistNames;
    m_discordRichPresence.activityType = DiscordActivityType::LISTENING;
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
  }
  qint64 currentEpochMs = QDateTime::currentMSecsSinceEpoch();
  qint64 startTimeSeconds = (currentEpochMs - m_position); // When playback actually started
  qint64 endTimeSeconds = 0;

  if (m_duration > 0)
  {
    endTimeSeconds = (currentEpochMs + (m_duration - m_position)); // When playback will finish
  }

  m_discordRichPresence.startTimestamp = startTimeSeconds;

  if (endTimeSeconds > 0)
  {
    m_discordRichPresence.endTimestamp = endTimeSeconds;
  }

  m_discordRichPresence.largeImageKey = "jellyfin2";
  std::string tmp_details = details.toStdString();
  std::string tmp_state = state.toStdString();
  m_discordRichPresence.details = tmp_details.c_str();
  m_discordRichPresence.state = tmp_state.c_str();
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