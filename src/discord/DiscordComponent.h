#ifndef DISCORDCOMPONENT_H
#define DISCORDCOMPONENT_H

#include "ComponentManager.h"
#include "discord_rpc.h"
#include "imgur.h"
#include <QObject>
#include <QTimer>
#include <atomic>

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

  static void handleDiscordReady(const DiscordUser* connectedUser);
  static void handleDiscordDisconnected(int errcode, const char* message);
  static void handleDiscordError(int errcode, const char* message);

  void onPlaying();
  void onMetaData(const QVariantMap& meta, QUrl baseUrl);
  void onPositionUpdate(quint64 position);

public slots:
  void valuesUpdated(const QVariantMap& values);

signals:
  void settingsUpdated(const QString& section, const QVariant& description);

private:
  const char* APPLICATION_ID = "1353419508324368404";
  std::unique_ptr<QTimer> m_callbackTimer;
  std::unique_ptr<QTimer> m_tryConnectTimer;

  DiscordRichPresence m_discordPresence;
  DiscordEventHandlers m_handlers;

  bool m_isConnected = false;
  bool m_richPresenceEnabled = false;
  QVariantMap metadata;
  QUrl m_baseUrl;
  quint64 m_position;
  qint64 m_duration;

  enum State
  {
    PAUSED,
    PLAYING,
    MENU
  };
  State m_currentState = State::MENU;

private:
  void setIsConnected();
  void setIsDisconnected();

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
  void tryConnect();
};

#endif