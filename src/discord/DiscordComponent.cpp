#include <QTimer>
#include "DiscordComponent.h"
#include "discord.h"
#include "QsLog.h"
#include "PlayerComponent.h"
#include "system/SystemComponent.h"

discord::Core* core{};

bool DiscordComponent::componentInitialize() {
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(runCallbacks()));
    timer->start(1000);

    m_position = 0;    

    connect(&PlayerComponent::Get(), &PlayerComponent::onMetaData, this, &DiscordComponent::onMetaData);
    connect(&PlayerComponent::Get(), &PlayerComponent::updateDuration, this, &DiscordComponent::onUpdateDuration);
    connect(&PlayerComponent::Get(), &PlayerComponent::positionUpdate, this, &DiscordComponent::onPositionUpdate);
    connect(&PlayerComponent::Get(), &PlayerComponent::stopped, this, &DiscordComponent::onStop);
    connect(&PlayerComponent::Get(), &PlayerComponent::paused, this, &DiscordComponent::onPause);
    connect(&PlayerComponent::Get(), &PlayerComponent::playing, this, &DiscordComponent::onPlaying);

    m_timer = std::make_unique<QTimer>(new QTimer(this));
    QObject::connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(tryConnect()));
    m_timer->setInterval(10000);

    return true;
}

void DiscordComponent::componentPostInitialize() {
    // tryConnect();
    m_timer->start();

}

void DiscordComponent::runCallbacks() {
    if (core == nullptr) {
        return;
    }
    core->RunCallbacks();
}

void DiscordComponent::tryConnect() {
    auto result = discord::Core::Create(1063276729617092729, DiscordCreateFlags_NoRequireDiscord, &core);

    if (result == discord::Result::Ok) {
        m_timer->stop();
        QLOG_DEBUG() << "Successfully connected to Discord";
        discord::Activity activity = buildMenuActivity();
        updateActivity(activity);   
    } else {
        QLOG_DEBUG() << "Failed to connect to Discord. Retrying in 10 seconds";
        m_timer->start();
    }
}

void DiscordComponent::onMetaData(const QVariantMap& meta, QUrl baseUrl) {
    metadata = meta;
    m_position = 0;
    m_baseUrl = baseUrl;
    // for (auto i = meta.begin(); i != meta.end(); i++) {
    //     QLOG_DEBUG() << "Key: " << i.key() << " Value: " << i.value();
    // }
     
}

void DiscordComponent::onUpdateDuration(qint64 duration) {
    m_duration = duration;
}

void DiscordComponent::onPositionUpdate(quint64 position) {
    m_position = position;
}

void DiscordComponent::onStop() {
    discord::Activity activity = buildMenuActivity();

    updateActivity(activity);
}

void DiscordComponent::onPause() {
    discord::Activity activity = buildWatchingActivity();

    updateActivity(activity);
}

void DiscordComponent::onPlaying() {
    discord::Activity activity = buildWatchingActivity();

    qint64 formatted = (m_duration - m_position + QDateTime::currentMSecsSinceEpoch()) / 1000;
    activity.GetTimestamps().SetEnd(formatted);
    
    updateActivity(activity); 
}

discord::Activity DiscordComponent::buildWatchingActivity() {
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

    return activity;
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
            tryConnect();
        }
    }); 
}