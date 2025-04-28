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
        qWarning() << "❌ Rich Presence update failed";
      }
    });
}

void DiscordComponent::makeWatchingActivity(){
  m_activity.SetType(discordpp::ActivityTypes::Playing);
  m_activity.SetName("Lord of the Rings");
  m_activity.SetDetails("Time...");
  discordpp::ActivityTimestamps timestamps;
  timestamps.SetStart(time(nullptr));
  timestamps.SetEnd(time(nullptr) + 3600);
  m_activity.SetTimestamps(timestamps);
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

void DiscordComponent::runCallbacks() {
  discordpp::RunCallbacks();
}

const char* DiscordComponent::componentName() { return "DiscordComponent"; }

bool DiscordComponent::componentExport() { return true; }
