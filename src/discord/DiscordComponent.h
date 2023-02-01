#ifndef DISCORDCOMPONENT_H
#define DISCORDCOMPONENT_H

#include <QObject>
#include "utils/Utils.h"
#include "ComponentManager.h"
#include "discord.h"

class DiscordComponent : public ComponentBase
{
    Q_OBJECT
    DEFINE_SINGLETON(DiscordComponent);
public:
    bool componentInitialize() override;
    void componentPostInitialize() override;
    // ~DiscordComponent() override;
    const char* componentName() override { return "discord"; }
    bool componentExport() override { return true; }
    explicit DiscordComponent(QObject* parent = nullptr): ComponentBase(parent) {}
    void onMetaData(const QVariantMap &meta, QUrl baseUrl);
    void onPositionUpdate(quint64 position);

private:
    qint64 m_duration;
    QVariantMap metadata;
    QUrl m_baseUrl;
    void onUpdateDuration(qint64 duration);
    discord::Activity buildWatchingActivity();
    discord::Activity buildMenuActivity();
    void onStop();
    void onPause();
    void updateActivity(discord::Activity& activity);
    void onPlaying();
    uint64_t m_position;
    std::unique_ptr<QTimer> m_timer;

private slots:
    void runCallbacks();
    void tryConnect();

};

#endif