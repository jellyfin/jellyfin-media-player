#include <QTimer>
#include "DiscordComponent.h"
#include "discord.h"
#include "QsLog.h"
#include "PlayerComponent.h"
#include "system/SystemComponent.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"


discord::Core* core{};

bool DiscordComponent::componentInitialize() {
    m_callbackTimer = std::make_unique<QTimer>(new QTimer(this));
    QObject::connect(m_callbackTimer.get(), SIGNAL(timeout()), this, SLOT(runCallbacks()));
    m_callbackTimer->setInterval(1000);
    m_callbackTimer->start();

    m_position = 0;    

    connect(&PlayerComponent::Get(), &PlayerComponent::onMetaData, this, &DiscordComponent::onMetaData);
    connect(&PlayerComponent::Get(), &PlayerComponent::updateDuration, this, &DiscordComponent::onUpdateDuration);
    connect(&PlayerComponent::Get(), &PlayerComponent::positionUpdate, this, &DiscordComponent::onPositionUpdate);
    connect(&PlayerComponent::Get(), &PlayerComponent::stopped, this, &DiscordComponent::onStop);
    connect(&PlayerComponent::Get(), &PlayerComponent::paused, this, &DiscordComponent::onPause);
    connect(&PlayerComponent::Get(), &PlayerComponent::playing, this, &DiscordComponent::onPlaying);
    connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_MAIN), &SettingsSection::valuesUpdated,
          this, &DiscordComponent::updateMainSectionSettings);

    m_tryConnectTimer = std::make_unique<QTimer>(new QTimer(this));
    QObject::connect(m_tryConnectTimer.get(), SIGNAL(timeout()), this, SLOT(tryConnect()));
    m_tryConnectTimer->setInterval(10000);


    return true;
}

void DiscordComponent::componentPostInitialize() {
    // m_tryConnectTimer->start();
    if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "EnableDiscordRichPresence").toBool()) {
        tryConnect();
    }
}

void DiscordComponent::runCallbacks() {
    // If discord integration is disabled, do nothing
    if (core == nullptr) {
        return;
    }
    auto result = core->RunCallbacks();

    if (result != discord::Result::Ok) {
        QLOG_DEBUG() << "RunCallback failed: " << int(result);
        if (!m_tryConnectTimer->isActive()) {
            m_tryConnectTimer->start();
        }
    }
}

void DiscordComponent::tryConnect() {
    auto result = discord::Core::Create(743296148592263240, DiscordCreateFlags_NoRequireDiscord, &core);

    if (result == discord::Result::Ok) {
        m_tryConnectTimer->stop();
        QLOG_DEBUG() << "Successfully connected to Discord";
        discord::Activity activity = buildActivity(State::MENU);
        updateActivity(activity);   
    } else {
        QLOG_DEBUG() << "Failed to connect to Discord. Retrying in 10 seconds";
        m_tryConnectTimer->start();
    }
}

void DiscordComponent::onMetaData(const QVariantMap& meta, QUrl baseUrl) {
    metadata = meta;
    m_position = 0;
    m_baseUrl = baseUrl;
}

void DiscordComponent::onUpdateDuration(qint64 duration) {
    m_duration = duration;
}

void DiscordComponent::onPositionUpdate(quint64 position) {
    m_position = position;
}

void DiscordComponent::onStop() {
    discord::Activity activity = buildActivity(State::MENU);

    updateActivity(activity);
}

void DiscordComponent::onPause() {
    discord::Activity activity = buildActivity(State::PAUSED);

    updateActivity(activity);
}

void DiscordComponent::onPlaying() {
    discord::Activity activity = buildActivity(State::PLAYING);
    
    updateActivity(activity); 
}

void DiscordComponent::updateMainSectionSettings(const QVariantMap& values) {
    if (!SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "EnableDiscordRichPresence").toBool()) {
        core->ActivityManager().ClearActivity([this](discord::Result result) {
            if (result == discord::Result::Ok) {
                QLOG_DEBUG() << "Cleared discord activity";
            } else {
                QLOG_DEBUG() << "Error clearing activity. Error = " << int(result);
            }
            delete core;
            core = nullptr;
        });
    } else {
        tryConnect();
    }
}

discord::Activity DiscordComponent::buildWatchingActivity(bool isPaused) {
    discord::Activity activity{};
    QString state;
    QString details;
    QString thumbnailUrl;
    
    if (metadata["Type"].toString() == "Movie") {
        state = metadata["Name"].toString();
        details = "Watching a movie";
        thumbnailUrl = QString("%1/Items/%2/Images/Backdrop").arg(m_baseUrl.toString(), metadata["Id"].toString());
    } else {
        state = QString("%1 - Season %2 Episode %3 : %4").arg(metadata["SeriesName"].toString(), metadata["ParentIndexNumber"].toString(), metadata["IndexNumber"].toString(), metadata["Name"].toString());
        details = "Watching a show";
        thumbnailUrl = QString("%1/Items/%2/Images/Backdrop").arg(m_baseUrl.toString(), metadata["ParentBackdropItemId"].toString());
    }

    activity.SetState(state.toStdString().c_str());
    activity.SetDetails(details.toStdString().c_str());
    activity.GetAssets().SetLargeImage(thumbnailUrl.toStdString().c_str());

    if (!isPaused) {
        qint64 formatted = (m_duration - m_position + QDateTime::currentMSecsSinceEpoch()) / 1000;
        activity.GetTimestamps().SetEnd(formatted);
    }
    return activity;
}

discord::Activity DiscordComponent::buildActivity(State state) {
    switch (state) {
        case State::PAUSED:
            return buildWatchingActivity(true);
        case State::PLAYING:
            return buildWatchingActivity(false);
        case State::MENU:
            return buildMenuActivity();
        default:
            QLOG_INFO() << "Invalid state";
    }
}


discord::Activity DiscordComponent::buildMenuActivity() {
    discord::Activity activity{};

    activity.SetDetails("In the menus");

    return activity;
}

void DiscordComponent::updateActivity(discord::Activity& activity) {
	QLOG_DEBUG() << "Setting new activity";

    if (core == nullptr) {
        QLOG_DEBUG() << "Discord core not set";
        return;
    }

    core->ActivityManager().UpdateActivity(activity, [this](discord::Result result) {
        if (result == discord::Result::Ok) {
		    QLOG_DEBUG() << "Discord : New activity success";
        } else {
		    QLOG_DEBUG() << "Unable to launch activity. Error = " << int(result);
        }
    }); 
}
