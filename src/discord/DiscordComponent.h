#ifndef DISCORDCOMPONENT_H
#define DISCORDCOMPONENT_H

#include "ComponentManager.h"
#include "discordpp.h"
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
  void authorize();
  void setupClientConnectionCallback();
  void setupLoggingCallback();
  void onPlaying();
  void onMetaData(const QVariantMap& meta, QUrl baseUrl);

  discordpp::Activity m_activity;
  std::shared_ptr<discordpp::Client> m_client;

  private:
  static constexpr uint64_t APPLICATION_ID = 1353419508324368404;
  std::unique_ptr<QTimer> m_callbackTimer;
  bool m_isConnected = false;
  QVariantMap metadata;
  QUrl m_baseUrl;
  uint64_t m_position;
  
  enum State { PAUSED, PLAYING, MENU };
  void updateActivity(State state);
  void makeWatchingActivity();
  void updateRichPresence();

private slots:
  void runCallbacks();
};

#endif