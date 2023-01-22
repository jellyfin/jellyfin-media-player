#ifndef DISCORDCOMPONENT_H
#define DISCORDCOMPONENT_H

#include <QObject>
#include "utils/Utils.h"
#include "ComponentManager.h"

class DiscordComponent : public ComponentBase
{
    Q_OBJECT
    DEFINE_SINGLETON(DiscordComponent);
public:
    bool componentInitialize() override;
    void componentPostInitialize() override;

    const char* componentName() override { return "discord"; }
    bool componentExport() override { return true; }
    explicit DiscordComponent(QObject* parent = nullptr): ComponentBase(parent) {}
    void onMetaData(const QVariantMap &meta, QUrl baseUrl);

private:
    // test
private slots:
    void RunCallbacks();
};

#endif