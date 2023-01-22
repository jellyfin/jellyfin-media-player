#include "DiscordComponent.h"
#include "discord.h"
#include "QsLog.h"
#include <QTimer>
#include "PlayerComponent.h"
/*
Here are all the possible map keys

Map:  "BackdropImageTags" 
Map:  "ChannelId" 
Map:  "Chapters" 
Map:  "CommunityRating" 
Map:  "Container" 
Map:  "HasSubtitles" 
Map:  "Id" 
Map:  "ImageBlurHashes" 
Map:  "ImageTags" 
Map:  "IndexNumber" 
Map:  "IsFolder" 
Map:  "LocationType" 
Map:  "MediaType" 
Map:  "Name" 
Map:  "OfficialRating" 
Map:  "ParentBackdropImageTags" 
Map:  "ParentBackdropItemId" 
Map:  "ParentIndexNumber" 
Map:  "ParentLogoImageTag" 
Map:  "ParentLogoItemId" 
Map:  "PremiereDate" 
Map:  "ProductionYear" 
Map:  "RunTimeTicks" 
Map:  "SeasonId" 
Map:  "SeasonName" 
Map:  "SeriesId" 
Map:  "SeriesName" 
Map:  "SeriesPrimaryImageTag" 
Map:  "ServerId" 
Map:  "Type" 
Map:  "UserData" 
Map:  "VideoType" 
Map:  "playOptions" 
*/

discord::Core* core{};

bool DiscordComponent::componentInitialize() {
    QTimer *timer = new QTimer(this);

    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(RunCallbacks()));

    timer->start(1000);
    
    connect(&PlayerComponent::Get(), &PlayerComponent::onMetaData, this, &DiscordComponent::onMetaData);

    return true;
}

void DiscordComponent::RunCallbacks() {
    core->RunCallbacks();

}

void DiscordComponent::componentPostInitialize() {
    auto discordCore = discord::Core::Create(1063276729617092729, DiscordCreateFlags_Default, &core);
    // auto activityManager = core->ActivityManager();
    // auto userManager = discord::Core.UserManager();

    discord::Activity activity{};
    activity.SetDetails("In the menus");

    QLOG_DEBUG() << "Discord: inside postinit";
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result == discord::Result::Ok) {
		    QLOG_DEBUG() << "Discord : New activity success";
            
        } else {
		    QLOG_DEBUG() << "Discord : Error = " << (int)result;
        }
    });    
}

void DiscordComponent::onMetaData(const QVariantMap& meta, QUrl baseUrl) {
    discord::Activity activity{};
    QString details = QString("%1 - Season %2 Episode %3 : %4").arg(meta["SeriesName"].toString(), meta["ParentIndexNumber"].toString(), meta["IndexNumber"].toString(), meta["Name"].toString());

    activity.SetState(details.toStdString().c_str());
    activity.SetDetails("Watching a show on Jellyfin");

    QLOG_DEBUG() << "Discord: Received metadata";
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result == discord::Result::Ok) {
		    QLOG_DEBUG() << "Discord : New activity success";
        } else {
		    QLOG_DEBUG() << "Discord : Error = " << (int)result;
        }
    }); 
}
