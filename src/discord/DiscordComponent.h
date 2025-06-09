#ifndef DISCORDCOMPONENT_H
#define DISCORDCOMPONENT_H

#include "ComponentManager.h"
#include "discord_rpc.h"
#include "imgur.h"
#include <atomic>
#include <QTimer>
#include <QObject>

class DiscordComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(DiscordComponent);

public:
  explicit DiscordComponent(QObject* parent = nullptr);
  ~DiscordComponent() = default;

  virtual bool componentInitialize() override;
  virtual const char* componentName() override;
  virtual bool componentExport() override;
  Q_SLOT void valuesUpdated(const QVariantMap& values);
  Q_SIGNAL void settingsUpdated(const QString& section, const QVariant& description);
  static void handleDiscordReady(const DiscordUser* connectedUser);
  static void handleDiscordDisconnected(int errcode, const char* message);
  static void handleDiscordError(int errcode, const char* message);
  static void handleDiscordJoin(const char* secret);
  static void handleDiscordSpectate(const char* secret);
  static void handleDiscordJoinRequest(const DiscordUser* request);
  void onPlaying();
  void onMetaData(const QVariantMap& meta, QUrl baseUrl);
  void onPositionUpdate(quint64 position);

  DiscordRichPresence m_discordPresence;

  private:
  const char* APPLICATION_ID = "1353419508324368404";
  std::unique_ptr<QTimer> m_callbackTimer;
  bool m_isConnected = true;
  bool m_richPresenceEnabled = false;
  QVariantMap metadata;
  QUrl m_baseUrl;
  quint64 m_position;
  qint64 m_duration;
  
  enum State { PAUSED, PLAYING, MENU };
  State m_currentState = State::MENU;
  void updateActivity(State state);
  void makeWatchingActivity(State state);
  void makeMenuActivity();
  void updateRichPresence();
  void onUpdateDuration(qint64 duration);
  void onStop();
  void onPause();
  bool downloadAndUpload(const std::string& imageUrl, std::string& response);

private slots:
  void runCallbacks();
};

#endif