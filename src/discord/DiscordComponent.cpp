#include "DiscordComponent.h"
#include "discord.h"
#include "QsLog.h"
#include <QTimer>

discord::Core* core{};

bool DiscordComponent::componentInitialize() {
    QTimer *timer = new QTimer(this);

    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(RunCallbacks()));

    timer->start(1000);
    
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
    activity.SetName("Mega test");
	activity.SetState("Testing");
	activity.SetDetails("Fruit Loops");
    QLOG_DEBUG() << "Discord: inside postinit";
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result == discord::Result::ServiceUnavailable) {
		    QLOG_DEBUG() << "Discord : New activity success";
            
        } else {
		    QLOG_DEBUG() << "Discord : New activity failed";
        }
    });    
        // activity.SetName("TEST");
    // var activity = new Discord.Activity
    // {
    //     State = "In Play Mode",
    //     Details = "Playing the Trumpet!",
    //     Timestamps =
    //     {
    //         Start = 5,
    //     },
    //             Assets =
    //     {
    //         LargeImage = "foo largeImageKey", // Larger Image Asset Value
    //         LargeText = "foo largeImageText", // Large Image Tooltip
    //         SmallImage = "foo smallImageKey", // Small Image Asset Value
    //         SmallText = "foo smallImageText", // Small Image Tooltip
    //     },
    //             Party =
    //     {
    //         Id = "foo partyID",
    //         Size = {
    //             CurrentSize = 1,
    //             MaxSize = 4,
    //         },
    //     },
    //             Secrets =
    //     {
    //         Match = "foo matchSecret",
    //         Join = "foo joinSecret",
    //         Spectate = "foo spectateSecret",
    //     },
    //     Instance = true,
    // };

    // activityManager.UpdateActivity(activity, (result) =>
    // {
    //     if (result == Discord.Result.Ok)
    //     {
    //         Console.WriteLine("Success!");
    //     }
    //     else
    //     {
    //         Console.WriteLine("Failed");
    //     }
    // });

    // // Return normally
    // userManager.OnCurrentUserUpdate += () =>
    // {
    //     var currentUser = userManager.GetCurrentUser();
    //     Console.WriteLine(currentUser.Username);
    //     Console.WriteLine(currentUser.Discriminator);
    //     Console.WriteLine(currentUser.Id);
    // };
}