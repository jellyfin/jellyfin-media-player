#include "DiscordComponent.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "player/PlayerComponent.h"

#include <csignal>
#include <iostream>
#include <thread>
#include <QDebug>

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

  // Create our Discord Client
  m_client = std::make_shared<discordpp::Client>();

  connect(&PlayerComponent::Get(), &PlayerComponent::playing, this, &DiscordComponent::onPlaying);
  connect(&PlayerComponent::Get(), &PlayerComponent::onMetaData, this, &DiscordComponent::onMetaData);

  setupLoggingCallback();
  setupClientConnectionCallback();
  authorize();

  return true;
}

void DiscordComponent::authorize(){
  // Generate OAuth2 code verifier for authentication
  auto codeVerifier = m_client->CreateAuthorizationCodeVerifier();

  // Set up authentication arguments
  discordpp::AuthorizationArgs args{};
  args.SetClientId(APPLICATION_ID);
  args.SetScopes(discordpp::Client::GetDefaultPresenceScopes());
  args.SetCodeChallenge(codeVerifier.Challenge());

  // Begin authentication process
  m_client->Authorize(
  args,
  [this, codeVerifier](auto result, auto code, auto redirectUri){
    if (!result.Successful()){
      std::cerr << "❌ Authentication Error: " << result.Error() << std::endl;
      return;
    }
    else{
      qDebug() << "✅ Authorization successful! Getting access token...\n";

      // Exchange auth code for access token
      m_client->GetToken(
      APPLICATION_ID, code, codeVerifier.Verifier(), redirectUri,
      [this](discordpp::ClientResult result, std::string accessToken, std::string refreshToken,
               discordpp::AuthorizationTokenType tokenType, int32_t expiresIn, std::string scope){
        qDebug() << "🔓 Access token received! Establishing connection...\n";
        // Next Step: Update the token and connect
        m_client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
                            [this](discordpp::ClientResult result){
                              if (result.Successful()){
                                qDebug() << "🔑 Token updated, connecting to Discord...\n";
                                m_client->Connect();
                              }
                            });
      });
    }
  });
}

void DiscordComponent::updateActivity(State state){
  // Configure rich presence details
  switch (state){
  case State::PLAYING:
    makeWatchingActivity();
    break;
  
  default:
    break;
  }
}

void DiscordComponent::updateRichPresence(){
  // Update rich presence
  m_client->UpdateRichPresence(m_activity,
    [](discordpp::ClientResult result){
      if (result.Successful()){
        qDebug() << "🎮 Rich Presence updated successfully!\n";
      }
      else{
        qWarning() << "❌ Rich Presence update failed " << result.ToString().c_str();
      }
    });
}

void DiscordComponent::makeWatchingActivity(){
  discordpp::ActivityAssets image;
  discordpp::ActivityTimestamps timestamp;
  QString state;
  QString details;
  QString thumbnailUrl;
  qDebug() << "METADATA " << metadata;
  if (metadata["Type"].toString() == "Movie") {
    state = metadata["Name"].toString();
    details = "Watching a movie";
    thumbnailUrl = QString("%1/Items/%2/Images/Primary").arg(m_baseUrl.toString(), metadata["Id"].toString());
    qDebug() << "THUMBNAIL URL: " << thumbnailUrl;
    // image.SetLargeImage(thumbnailUrl.toStdString().c_str());
    // image.SetLargeImage("https://10.0.0.4:8920/Items/95237878fc8fa852c3f9de9b5cfdd5d0/Images/Primary");
    image.SetLargeImage("movie");
  }
  if (metadata["Type"].toString() == "Episode") {
    state = metadata["Name"].toString();
    details = QString("%1 : %2").arg(metadata["SeriesName"].toString(), metadata["SeasonName"].toString());
    thumbnailUrl = QString("%1/Items/%2/Images/Backdrop").arg(m_baseUrl.toString(), metadata["ParentBackdropItemId"].toString());
    image.SetLargeImage("show");
  }
  if (metadata["Type"].toString() == "Audio" || metadata["Type"].toString() == "AudioBook"){
    QStringList artistNames;
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
    thumbnailUrl = QString("%1/Items/%2/Images/Backdrop").arg(m_baseUrl.toString(), metadata["Id"].toString());
    image.SetLargeImage("music");
  }

  image.SetSmallImage("jellyfin");
  m_activity.SetAssets(image);
  m_activity.SetType(discordpp::ActivityTypes::Playing);
  m_activity.SetDetails(details.toStdString().c_str());
  m_activity.SetState(state.toStdString().c_str());
  m_activity.Timestamps()->Drop();
  updateRichPresence();
}

void DiscordComponent::setupClientConnectionCallback(){
  // Set up status callback to monitor client connection
  m_client->SetStatusChangedCallback(
    [this](discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail){
      qDebug() << "🔄 Status changed: " << QString::fromStdString(discordpp::Client::StatusToString(status));
  
      if (status == discordpp::Client::Status::Ready){
        m_isConnected = true;
        m_activity = discordpp::Activity{};
      }
      else if (error != discordpp::Client::Error::None){
        qDebug() << "❌ Connection Error: " << QString::fromStdString(discordpp::Client::ErrorToString(error))
                  << " - Details: " << errorDetail;
        m_isConnected = false;
      }
    });
}

void DiscordComponent::setupLoggingCallback(){
  // Set up logging callback
  m_client->AddLogCallback(
    [](auto message, auto severity)
    { std::cout << "[" << EnumToString(severity) << "] " << message << std::endl; },
    discordpp::LoggingSeverity::Info);
}

void DiscordComponent::onPlaying() {
  qDebug() << "OnPlaying triggered";
  if(m_isConnected){
    updateActivity(State::PLAYING);
  }
}

void DiscordComponent::onMetaData(const QVariantMap& meta, QUrl baseUrl) {
  metadata = meta;
  m_position = 0;
  m_baseUrl = baseUrl;
}

void DiscordComponent::runCallbacks() {
  discordpp::RunCallbacks();
}

const char* DiscordComponent::componentName() { return "DiscordComponent"; }

bool DiscordComponent::componentExport() { return true; }
