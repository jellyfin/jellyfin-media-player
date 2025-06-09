#include "DiscordComponent.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "player/PlayerComponent.h"

#include <csignal>
#include <iostream>
#include <thread>
#include <QDebug>
#include <QTimer>

DiscordComponent::DiscordComponent(QObject* parent) : ComponentBase(parent) {
  qDebug() << "[DiscordSettings] Init";
  SettingsSection* discordSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_OTHER);
  connect(discordSection, &SettingsSection::valuesUpdated, this, &DiscordComponent::valuesUpdated);
}

void DiscordComponent::valuesUpdated(const QVariantMap& values) {
  bool state = SettingsComponent::Get().value(SETTINGS_SECTION_OTHER, "Discord_Integration").toBool();
  qDebug() << "[DiscordSettings] State: " << state;
}

bool DiscordComponent::componentInitialize(){

  m_callbackTimer = std::make_unique<QTimer>(new QTimer(this));
  QObject::connect(m_callbackTimer.get(), SIGNAL(timeout()), this, SLOT(runCallbacks()));
  m_callbackTimer->setInterval(1000);
  m_callbackTimer->start();

  DiscordEventHandlers handlers;
  memset(&handlers, 0, sizeof(handlers));
  handlers.ready = handleDiscordReady;
  handlers.disconnected = handleDiscordDisconnected;
  handlers.errored = handleDiscordError;
  handlers.joinGame = handleDiscordJoin;
  handlers.spectateGame = handleDiscordSpectate;
  handlers.joinRequest = handleDiscordJoinRequest;
  Discord_Initialize(this->APPLICATION_ID, &handlers, 1, NULL);

  memset(&m_discordPresence, 0, sizeof(m_discordPresence));

  connect(&PlayerComponent::Get(), &PlayerComponent::playing, this, &DiscordComponent::onPlaying);
  connect(&PlayerComponent::Get(), &PlayerComponent::onMetaData, this, &DiscordComponent::onMetaData);
  connect(&PlayerComponent::Get(), &PlayerComponent::updateDuration, this, &DiscordComponent::onUpdateDuration);
  connect(&PlayerComponent::Get(), &PlayerComponent::positionUpdate, this, &DiscordComponent::onPositionUpdate);
  connect(&PlayerComponent::Get(), &PlayerComponent::stopped, this, &DiscordComponent::onStop);
  connect(&PlayerComponent::Get(), &PlayerComponent::paused, this, &DiscordComponent::onPause);
  connect(&PlayerComponent::Get(), &PlayerComponent::onMpvEvents, this, &DiscordComponent::onMpvEvents);
  connect(&PlayerComponent::Get(), &PlayerComponent::finished, this, &DiscordComponent::onMpvEvents);

  return true;
}

void DiscordComponent::updateActivity(State state){
  
  m_currentState = state;

  switch (state){
  case State::PLAYING:
    makeWatchingActivity(state);
    break;
  case State::PAUSED:
    Discord_ClearPresence();
    break;
  case State::MENU:
    Discord_ClearPresence();
    break;
  default:
    break;
  }
}

void DiscordComponent::updateRichPresence(){
  // Update rich presence
  Discord_UpdatePresence(&m_discordPresence);
}

void DiscordComponent::makeWatchingActivity(State watchingState){
  //memset(&m_discordPresence, 0, sizeof(m_discordPresence));
  QString state;
  QString details;
  QString thumbnailUrl;
  Imgur uploader;
  qDebug() << "METADATA " << metadata;
  if (metadata["Type"].toString() == "Movie") {
    m_discordPresence.activityType = DiscordActivityType::WATCHING;
    details = QString(metadata["Name"].toString());
    state = QString("Watching a movie");
    thumbnailUrl = QString("%1/Items/%2/Images/Primary").arg(m_baseUrl.toString(), metadata["Id"].toString());
    // qDebug() << "THUMBNAIL URL: " << thumbnailUrl;
    // image.SetLargeImage(thumbnailUrl.toStdString().c_str());
    // image.SetLargeImage("https://10.0.0.4:8920/Items/95237878fc8fa852c3f9de9b5cfdd5d0/Images/Primary");
    if(uploader.downloadAndUpload(thumbnailUrl.toStdString().c_str(), m_imgurLink)){
      m_discordPresence.largeImageKey = m_imgurLink.c_str();
    }else{
      m_discordPresence.largeImageKey = "movie";
    }
  }
  if (metadata["Type"].toString() == "Episode") {
    m_discordPresence.activityType = DiscordActivityType::WATCHING;
    state = QString(metadata["Name"].toString());
    details = QString("%1 : %2").arg(metadata["SeriesName"].toString(), metadata["SeasonName"].toString());
    thumbnailUrl = QString("%1/Items/%2/Images/Primary").arg(m_baseUrl.toString(), metadata["ParentBackdropItemId"].toString());
    qDebug() << "THUMBNAIL URL: " << thumbnailUrl;
    if (uploader.downloadAndUpload(thumbnailUrl.toStdString().c_str(), m_imgurLink)){
      m_discordPresence.largeImageKey = m_imgurLink.c_str();
    }else{
      m_discordPresence.largeImageKey = "show";
    }
  }
  if (metadata["Type"].toString() == "Audio" || metadata["Type"].toString() == "AudioBook"){
    QStringList artistNames;
    m_discordPresence.activityType = DiscordActivityType::LISTENING;
    if (metadata.contains("Artists")) {
      QVariantList artistsList = metadata["Artists"].toList();
      for (const QVariant& artist : artistsList) {
        artistNames << artist.toString();
      }
      artistNames.removeDuplicates();
      
      state = artistNames.join(", ");
    }else{
      state = "Unknown Artist";
    }
    details = metadata["Name"].toString();
    qDebug() << "DETAILS " << details;
    qDebug() << "STATE " << state;
    thumbnailUrl = QString("%1/Items/%2/Images/Primary").arg(m_baseUrl.toString(), metadata["Id"].toString());
    qDebug() << "THUMBNAIL URL: " << thumbnailUrl;
    if (metadata["Type"].toString() == "Audio"){
      if (uploader.downloadAndUpload(thumbnailUrl.toStdString().c_str(), m_imgurLink)){
        m_discordPresence.largeImageKey = m_imgurLink.c_str();
      }else{
        m_discordPresence.largeImageKey = "music";
      }
    }else if(metadata["Type"].toString() == "AudioBook"){
      if (uploader.downloadAndUpload(thumbnailUrl.toStdString().c_str(), m_imgurLink)){
        m_discordPresence.largeImageKey = m_imgurLink.c_str();
      }else{
        m_discordPresence.largeImageKey = "audiobook";
      }
    }
  }
  qDebug() << "IMGUR LINK: " << m_imgurLink.c_str();
  qint64 currentEpochMs = QDateTime::currentMSecsSinceEpoch();
  qint64 startTimeSeconds = (currentEpochMs - m_position); // When playback actually started
  qint64 endTimeSeconds = 0;
  
  if (m_duration > 0) {
    endTimeSeconds = (currentEpochMs + (m_duration - m_position)); // When playback will finish
  }
  
  m_discordPresence.startTimestamp = startTimeSeconds;

  if (endTimeSeconds > 0) {
    m_discordPresence.endTimestamp = endTimeSeconds;
  }
  std::string tmp_details = details.toStdString();
  std::string tmp_state = state.toStdString();
  
  //m_discordPresence.smallImageKey = "jellyfin";
  m_discordPresence.details = tmp_details.c_str();
  m_discordPresence.state = tmp_state.c_str();
  //m_discordPresence.state = state.toStdString().c_str();
  updateRichPresence();
}

void DiscordComponent::makeMenuActivity(){
  //memset(&m_discordPresence, 0, sizeof(m_discordPresence));
  QString details = "In Menu";
  QString state = "Browsing";
  m_discordPresence.activityType = DiscordActivityType::LISTENING;
  // m_discordPresence.smallImageKey = "jellyfin";
  // m_discordPresence.largeImageKey = "jellyfin";
  // m_discordPresence.startTimestamp = QDateTime::currentMSecsSinceEpoch();
  // m_discordPresence.endTimestamp = QDateTime::currentMSecsSinceEpoch();
  m_discordPresence.startTimestamp = 0;
  m_discordPresence.endTimestamp = 0;
  m_discordPresence.details = details.toStdString().c_str();
  m_discordPresence.state = state.toStdString().c_str();
  updateRichPresence();
}

void DiscordComponent::onPlaying() {
  if(m_isConnected){
    updateActivity(State::PLAYING);
  }
}

void DiscordComponent::onStop() {
  updateActivity(State::MENU);
}

void DiscordComponent::onPause() {
  updateActivity(State::PAUSED);
}

void DiscordComponent::onFinished() {
  updateActivity(State::MENU);
}

void DiscordComponent::onMpvEvents() {
  double speed = qvariant_cast<double>(PlayerComponent::Get().property("speed"));
  //double speed2 = qvariant_cast<double>(mpv::qt::get_property(PlayerComponent::Get().getMpvHandle(), "speed"));
  qDebug() << "SPEED: " << speed;
  //qDebug() << "SPEED2: " << speed2;
}

void DiscordComponent::onUpdateDuration(qint64 duration){
  m_duration = duration;
  // updateActivity(State::PLAYING);
}

void DiscordComponent::onPositionUpdate(quint64 position){
  m_position = position;
  // updateActivity(State::PLAYING);
}

void DiscordComponent::onMetaData(const QVariantMap& meta, QUrl baseUrl) {
  metadata = meta;
  m_position = 0;
  m_baseUrl = baseUrl;
}

void DiscordComponent::handleDiscordReady(const DiscordUser* connectedUser){
  qDebug() << "Discord: connected to user";
}

void DiscordComponent::handleDiscordDisconnected(int errcode, const char* message){
  qDebug() << "Discord: disconnected";
}

void DiscordComponent::handleDiscordError(int errcode, const char* message){
  qDebug() << "Discord: error";
}

void DiscordComponent::handleDiscordJoin(const char* secret){
  qDebug() << "Discord: join";
}

void DiscordComponent::handleDiscordSpectate(const char* secret){
  qDebug() << "Discord: spectate";
}

void DiscordComponent::handleDiscordJoinRequest(const DiscordUser* request){
  qDebug() << "Join";
}

void DiscordComponent::runCallbacks() {
  Discord_RunCallbacks();

  // if(m_currentState == State::PAUSED){
  //   makeWatchingActivity(m_currentState);
  // }
}

const char* DiscordComponent::componentName() { return "DiscordComponent"; }

bool DiscordComponent::componentExport() { return true; }