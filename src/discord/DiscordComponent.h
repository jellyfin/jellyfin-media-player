#ifndef DISCORDCOMPONENT_H
#define DISCORDCOMPONENT_H

#include "ComponentManager.h"
#include "discordpp.h"
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
  Q_SLOT void valuesUpdated(const QVariantMap& values);
  Q_SIGNAL void settingsUpdated(const QString& section, const QVariant& description);
  
  discordpp::Client m_client;
};

#endif