#include "DiscordComponent.h"
#include "discord.h"
#include "QsLog.h"
#include <QTimer>
#include "PlayerComponent.h"
/*
Here are all the possible map keys

Key:  "BackdropImageTags"  Value:  QVariant(QVariantList, (QVariant(QString, "76373451dfa5e3aeb8e6f14139b74e6a"))) 
Key:  "CanDelete"  Value:  QVariant(bool, true) 
Key:  "CanDownload"  Value:  QVariant(bool, true) 
Key:  "ChannelId"  Value:  QVariant(std::nullptr_t, (nullptr)) 
Key:  "Chapters"  Value:  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 1"))("StartPositionTicks", QVariant(qlonglong, 0)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 2"))("StartPositionTicks", QVariant(qlonglong, 2792370000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 3"))("StartPositionTicks", QVariant(qlonglong, 6658320000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 4"))("StartPositionTicks", QVariant(qlonglong, 9349760000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 5"))("StartPositionTicks", QVariant(qlonglong, 13480130000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 6"))("StartPositionTicks", QVariant(qlonglong, 18114760000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 7"))("StartPositionTicks", QVariant(qlonglong, 25784930000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 8"))("StartPositionTicks", QVariant(qlonglong, 31324630000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 9"))("StartPositionTicks", QVariant(qlonglong, 36400950000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 10"))("StartPositionTicks", QVariant(qlonglong, 39752630000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 11"))("StartPositionTicks", QVariant(qlonglong, 42789000000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 12"))("StartPositionTicks", QVariant(qlonglong, 50590120000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 13"))("StartPositionTicks", QVariant(qlonglong, 53262380000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 14"))("StartPositionTicks", QVariant(qlonglong, 54149090000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 15"))("StartPositionTicks", QVariant(qlonglong, 60929200000)))), QVariant(QVariantMap, QMap(("ImageDateModified", QVariant(QString, "0001-01-01T00:00:00.0000000Z"))("Name", QVariant(QString, "Chapter 16"))("StartPositionTicks", QVariant(qlonglong, 66543140000)))))) 
Key:  "CommunityRating"  Value:  QVariant(double, 7.759) 
Key:  "Container"  Value:  QVariant(QString, "mkv,webm") 
Key:  "CriticRating"  Value:  QVariant(qlonglong, 86) 
Key:  "DateCreated"  Value:  QVariant(QString, "2023-01-21T02:39:17.7635776Z") 
Key:  "DisplayPreferencesId"  Value:  QVariant(QString, "dbf7709c41faaa746463d67978eb863d") 
Key:  "EnableMediaSourceDisplay"  Value:  QVariant(bool, true) 
Key:  "Etag"  Value:  QVariant(QString, "4745a4a89c6fcdcad287161df88fa215") 
Key:  "ExternalUrls"  Value:  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("Name", QVariant(QString, "IMDb"))("Url", QVariant(QString, "https://www.imdb.com/title/tt0099077")))), QVariant(QVariantMap, QMap(("Name", QVariant(QString, "TheMovieDb"))("Url", QVariant(QString, "https://www.themoviedb.org/movie/11005")))), QVariant(QVariantMap, QMap(("Name", QVariant(QString, "Trakt"))("Url", QVariant(QString, "https://trakt.tv/movies/tt0099077")))))) 
Key:  "GenreItems"  Value:  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("Id", QVariant(QString, "090eac6e9de4fe1fbc194e5b96691277"))("Name", QVariant(QString, "Drama")))))) 
Key:  "Genres"  Value:  QVariant(QVariantList, (QVariant(QString, "Drama"))) 
Key:  "HasSubtitles"  Value:  QVariant(bool, true) 
Key:  "Height"  Value:  QVariant(qlonglong, 1040) 
Key:  "Id"  Value:  QVariant(QString, "c18c2aee68814162f28ccb1043059198") 
Key:  "ImageBlurHashes"  Value:  QVariant(QVariantMap, QMap(("Backdrop", QVariant(QVariantMap, QMap(("76373451dfa5e3aeb8e6f14139b74e6a", QVariant(QString, "WSGum*x^0LIUIVIpMwRPbct7s;t74njX%MkDxaof%go#RiRiocs:")))))("Logo", QVariant(QVariantMap, QMap(("ed57fe419a582a8c7c5325f79bd5a069", QVariant(QString, "H2Cpr2}FS2=K63AWoL5+EzEzWVNuoLayjtW;n%w{")))))("Primary", QVariant(QVariantMap, QMap(("28c37030879c6df89143779bac676c2c", QVariant(QString, "d?NdzMxvR*x]_Nofoft7D$WBj[WB-pt6WBj[%Mj[oeof"))))))) 
Key:  "ImageTags"  Value:  QVariant(QVariantMap, QMap(("Logo", QVariant(QString, "ed57fe419a582a8c7c5325f79bd5a069"))("Primary", QVariant(QString, "28c37030879c6df89143779bac676c2c")))) 
Key:  "IsFolder"  Value:  QVariant(bool, false) 
Key:  "IsHD"  Value:  QVariant(bool, true) 
Key:  "LocalTrailerCount"  Value:  QVariant(qlonglong, 0) 
Key:  "LocationType"  Value:  QVariant(QString, "FileSystem") 
Key:  "LockData"  Value:  QVariant(bool, false) 
Key:  "LockedFields"  Value:  QVariant(QVariantList, ()) 
Key:  "MediaSources"  Value:  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("Bitrate", QVariant(qlonglong, 11907419))("Container", QVariant(QString, "mkv"))("DefaultAudioStreamIndex", QVariant(qlonglong, 1))("ETag", QVariant(QString, "55473cb2969435fc9cf92c5748de9e7f"))("Formats", QVariant(QVariantList, ()))("GenPtsInput", QVariant(bool, false))("Id", QVariant(QString, "c18c2aee68814162f28ccb1043059198"))("IgnoreDts", QVariant(bool, false))("IgnoreIndex", QVariant(bool, false))("IsInfiniteStream", QVariant(bool, false))("IsRemote", QVariant(bool, false))("MediaAttachments", QVariant(QVariantList, ()))("MediaStreams", QVariant(QVariantList, (QVariant(QVariantMap, QMap(("AspectRatio", QVariant(QString, "1.85:1"))("AverageFrameRate", QVariant(double, 23.976))("BitDepth", QVariant(qlonglong, 8))("BitRate", QVariant(qlonglong, 10371419))("Codec", QVariant(QString, "h264"))("DisplayTitle", QVariant(QString, "1080p H264 SDR"))("Height", QVariant(qlonglong, 1040))("Index", QVariant(qlonglong, 0))("IsAVC", QVariant(bool, true))("IsDefault", QVariant(bool, true))("IsExternal", QVariant(bool, false))("IsForced", QVariant(bool, false))("IsInterlaced", QVariant(bool, false))("IsTextSubtitleStream", QVariant(bool, false))("Language", QVariant(QString, "eng"))("Level", QVariant(qlonglong, 41))("NalLengthSize", QVariant(QString, "4"))("PixelFormat", QVariant(QString, "yuv420p"))("Profile", QVariant(QString, "High"))("RealFrameRate", QVariant(double, 23.976))("RefFrames", QVariant(qlonglong, 1))("SupportsExternalStream", QVariant(bool, false))("TimeBase", QVariant(QString, "1/1000"))("Type", QVariant(QString, "Video"))("VideoRange", QVariant(QString, "SDR"))("VideoRangeType", QVariant(QString, "SDR"))("Width", QVariant(qlonglong, 1920)))), QVariant(QVariantMap, QMap(("BitRate", QVariant(qlonglong, 1536000))("ChannelLayout", QVariant(QString, "5.1"))("Channels", QVariant(qlonglong, 6))("Codec", QVariant(QString, "dts"))("DisplayTitle", QVariant(QString, "English - DTS - 5.1 - Default"))("Index", QVariant(qlonglong, 1))("IsDefault", QVariant(bool, true))("IsExternal", QVariant(bool, false))("IsForced", QVariant(bool, false))("IsInterlaced", QVariant(bool, false))("IsTextSubtitleStream", QVariant(bool, false))("Language", QVariant(QString, "eng"))("Level", QVariant(qlonglong, 0))("Profile", QVariant(QString, "DTS"))("SampleRate", QVariant(qlonglong, 48000))("SupportsExternalStream", QVariant(bool, false))("TimeBase", QVariant(QString, "1/1000"))("Type", QVariant(QString, "Audio")))), QVariant(QVariantMap, QMap(("Codec", QVariant(QString, "subrip"))("DisplayTitle", QVariant(QString, "English - SUBRIP"))("Index", QVariant(qlonglong, 2))("IsDefault", QVariant(bool, false))("IsExternal", QVariant(bool, false))("IsForced", QVariant(bool, false))("IsInterlaced", QVariant(bool, false))("IsTextSubtitleStream", QVariant(bool, true))("Language", QVariant(QString, "eng"))("Level", QVariant(qlonglong, 0))("LocalizedDefault", QVariant(QString, "Default"))("LocalizedExternal", QVariant(QString, "External"))("LocalizedForced", QVariant(QString, "Forced"))("LocalizedUndefined", QVariant(QString, "Undefined"))("SupportsExternalStream", QVariant(bool, true))("TimeBase", QVariant(QString, "1/1000"))("Type", QVariant(QString, "Subtitle")))), QVariant(QVariantMap, QMap(("Codec", QVariant(QString, "subrip"))("DisplayTitle", QVariant(QString, "Spanish - SUBRIP"))("Index", QVariant(qlonglong, 3))("IsDefault", QVariant(bool, false))("IsExternal", QVariant(bool, false))("IsForced", QVariant(bool, false))("IsInterlaced", QVariant(bool, false))("IsTextSubtitleStream", QVariant(bool, true))("Language", QVariant(QString, "spa"))("Level", QVariant(qlonglong, 0))("LocalizedDefault", QVariant(QString, "Default"))("LocalizedExternal", QVariant(QString, "External"))("LocalizedForced", QVariant(QString, "Forced"))("LocalizedUndefined", QVariant(QString, "Undefined"))("SupportsExternalStream", QVariant(bool, true))("TimeBase", QVariant(QString, "1/1000"))("Type", QVariant(QString, "Subtitle")))))))("Name", QVariant(QString, "Awakenings.1990.1080p.BluRay.X264-AMIABLE"))("Path", QVariant(QString, "/data/media/movies/Awakenings (1990)/Awakenings.1990.1080p.BluRay.X264-AMIABLE.mkv"))("Protocol", QVariant(QString, "File"))("ReadAtNativeFramerate", QVariant(bool, false))("RequiredHttpHeaders", QVariant(QVariantMap, QMap()))("RequiresClosing", QVariant(bool, false))("RequiresLooping", QVariant(bool, false))("RequiresOpening", QVariant(bool, false))("RunTimeTicks", QVariant(qlonglong, 72411512832))("Size", QVariant(qlonglong, 9387627520))("SupportsDirectPlay", QVariant(bool, true))("SupportsDirectStream", QVariant(bool, true))("SupportsProbing", QVariant(bool, true))("SupportsTranscoding", QVariant(bool, true))("Type", QVariant(QString, "Default"))("VideoType", QVariant(QString, "VideoFile")))))) 
Key:  "MediaStreams"  Value:  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("AspectRatio", QVariant(QString, "1.85:1"))("AverageFrameRate", QVariant(double, 23.976))("BitDepth", QVariant(qlonglong, 8))("BitRate", QVariant(qlonglong, 10371419))("Codec", QVariant(QString, "h264"))("DisplayTitle", QVariant(QString, "1080p H264 SDR"))("Height", QVariant(qlonglong, 1040))("Index", QVariant(qlonglong, 0))("IsAVC", QVariant(bool, true))("IsDefault", QVariant(bool, true))("IsExternal", QVariant(bool, false))("IsForced", QVariant(bool, false))("IsInterlaced", QVariant(bool, false))("IsTextSubtitleStream", QVariant(bool, false))("Language", QVariant(QString, "eng"))("Level", QVariant(qlonglong, 41))("NalLengthSize", QVariant(QString, "4"))("PixelFormat", QVariant(QString, "yuv420p"))("Profile", QVariant(QString, "High"))("RealFrameRate", QVariant(double, 23.976))("RefFrames", QVariant(qlonglong, 1))("SupportsExternalStream", QVariant(bool, false))("TimeBase", QVariant(QString, "1/1000"))("Type", QVariant(QString, "Video"))("VideoRange", QVariant(QString, "SDR"))("VideoRangeType", QVariant(QString, "SDR"))("Width", QVariant(qlonglong, 1920)))), QVariant(QVariantMap, QMap(("BitRate", QVariant(qlonglong, 1536000))("ChannelLayout", QVariant(QString, "5.1"))("Channels", QVariant(qlonglong, 6))("Codec", QVariant(QString, "dts"))("DisplayTitle", QVariant(QString, "English - DTS - 5.1 - Default"))("Index", QVariant(qlonglong, 1))("IsDefault", QVariant(bool, true))("IsExternal", QVariant(bool, false))("IsForced", QVariant(bool, false))("IsInterlaced", QVariant(bool, false))("IsTextSubtitleStream", QVariant(bool, false))("Language", QVariant(QString, "eng"))("Level", QVariant(qlonglong, 0))("Profile", QVariant(QString, "DTS"))("SampleRate", QVariant(qlonglong, 48000))("SupportsExternalStream", QVariant(bool, false))("TimeBase", QVariant(QString, "1/1000"))("Type", QVariant(QString, "Audio")))), QVariant(QVariantMap, QMap(("Codec", QVariant(QString, "subrip"))("DisplayTitle", QVariant(QString, "English - SUBRIP"))("Index", QVariant(qlonglong, 2))("IsDefault", QVariant(bool, false))("IsExternal", QVariant(bool, false))("IsForced", QVariant(bool, false))("IsInterlaced", QVariant(bool, false))("IsTextSubtitleStream", QVariant(bool, true))("Language", QVariant(QString, "eng"))("Level", QVariant(qlonglong, 0))("LocalizedDefault", QVariant(QString, "Default"))("LocalizedExternal", QVariant(QString, "External"))("LocalizedForced", QVariant(QString, "Forced"))("LocalizedUndefined", QVariant(QString, "Undefined"))("SupportsExternalStream", QVariant(bool, true))("TimeBase", QVariant(QString, "1/1000"))("Type", QVariant(QString, "Subtitle")))), QVariant(QVariantMap, QMap(("Codec", QVariant(QString, "subrip"))("DisplayTitle", QVariant(QString, "Spanish - SUBRIP"))("Index", QVariant(qlonglong, 3))("IsDefault", QVariant(bool, false))("IsExternal", QVariant(bool, false))("IsForced", QVariant(bool, false))("IsInterlaced", QVariant(bool, false))("IsTextSubtitleStream", QVariant(bool, true))("Language", QVariant(QString, "spa"))("Level", QVariant(qlonglong, 0))("LocalizedDefault", QVariant(QString, "Default"))("LocalizedExternal", QVariant(QString, "External"))("LocalizedForced", QVariant(QString, "Forced"))("LocalizedUndefined", QVariant(QString, "Undefined"))("SupportsExternalStream", QVariant(bool, true))("TimeBase", QVariant(QString, "1/1000"))("Type", QVariant(QString, "Subtitle")))))) 
Key:  "MediaType"  Value:  QVariant(QString, "Video") 
Key:  "Name"  Value:  QVariant(QString, "Awakenings") 
Key:  "OfficialRating"  Value:  QVariant(QString, "PG-13") 
Key:  "OriginalTitle"  Value:  QVariant(QString, "Awakenings") 
Key:  "Overview"  Value:  QVariant(QString, "Dr. Malcolm Sayer, a shy research physician, uses an experimental drug to \"awaken\" the catatonic victims of a rare disease. Leonard is the first patient to receive the controversial treatment. His awakening, filled with awe and enthusiasm, proves a rebirth for Sayer too, as the exuberant patient reveals life's simple but unutterably sweet pleasures to the introverted doctor.") 
Key:  "ParentId"  Value:  QVariant(QString, "f7e7e63b37457bf249703e993260a2e6") 
Key:  "Path"  Value:  QVariant(QString, "/data/media/movies/Awakenings (1990)/Awakenings.1990.1080p.BluRay.X264-AMIABLE.mkv") 
Key:  "People"  Value:  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("Id", QVariant(QString, "390da78b6dc3e04b14278f448a6be5ac"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("a3d87bb7ff496ba07eace584d8a6966c", QVariant(QString, "deFrbOxt0KNGROj@%gofS~R*Vst6NHkCxaWBRjjZxukC"))))))))("Name", QVariant(QString, "Robert De Niro"))("PrimaryImageTag", QVariant(QString, "a3d87bb7ff496ba07eace584d8a6966c"))("Role", QVariant(QString, "Leonard Lowe"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "500693e4f1af515ed2824e904bd6dd66"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("77b0a10007954ca404dc9193b8ac3f8d", QVariant(QString, "dgI}hjxt%hxv~qofxvt7E1WVnioL%gkCxut7%goft7t7"))))))))("Name", QVariant(QString, "Robin Williams"))("PrimaryImageTag", QVariant(QString, "77b0a10007954ca404dc9193b8ac3f8d"))("Role", QVariant(QString, "Malcolm Sayer"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "34ce23fcc4efbf7f6c92d5126fd4c8a7"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("e22cc4634dfc5ab907f85a7b4fdd530d", QVariant(QString, "dTJtrHxFFvOX_NjZ_3tR.TozspxutRWVxut7.8WCi_oK"))))))))("Name", QVariant(QString, "John Heard"))("PrimaryImageTag", QVariant(QString, "e22cc4634dfc5ab907f85a7b4fdd530d"))("Role", QVariant(QString, "Dr. Kaufman"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "9f188553dcab736a158954658a60bc9f"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("cc15245b8dd16dfa7268880a540dda21", QVariant(QString, "diHw=KoJ0fxa57WBxZWC9aof%2j?WBaykCjus;ayocWV"))))))))("Name", QVariant(QString, "Julie Kavner"))("PrimaryImageTag", QVariant(QString, "cc15245b8dd16dfa7268880a540dda21"))("Role", QVariant(QString, "Eleanor Costello"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "ce84d65c8677da6a9d2c53819a7d2631"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("785a669359d507e6219e894bd490a70b", QVariant(QString, "dqH,#0NH0#%1xaayRka}I=xZjZNHNHWXs.jZR+j[oLay"))))))))("Name", QVariant(QString, "Penelope Ann Miller"))("PrimaryImageTag", QVariant(QString, "785a669359d507e6219e894bd490a70b"))("Role", QVariant(QString, "Paula"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "a2dbfebaac71c658afe3e1dcd6565a2c"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("c7fadcabbbca5bb0b31f0849c948b1e5", QVariant(QString, "dTIOhG-;?b_3~qt7t7ofRjofM{IU_3t7WBofofM{M{of"))))))))("Name", QVariant(QString, "Ruth Nelson"))("PrimaryImageTag", QVariant(QString, "c7fadcabbbca5bb0b31f0849c948b1e5"))("Role", QVariant(QString, "Mrs. Lowe"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "f20172d90e00b84b8ef36aa37dcb8d22"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("dacbcbdc4341199552794da584a97f81", QVariant(QString, "ddJH?eM{-;t7?HRj%Lt701RjIURjRjRjayR*t7of%Lt7"))))))))("Name", QVariant(QString, "Max von Sydow"))("PrimaryImageTag", QVariant(QString, "dacbcbdc4341199552794da584a97f81"))("Role", QVariant(QString, "Peter Ingham"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "d8ac845ee938c29d6a7a30d521248b34"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("38334dd2cf9359d71338306b8f06ff6c", QVariant(QString, "dXK_E~D%Rj_3ofayRjt7~q%M%Mxu_3j[xuoft7j[RjRj"))))))))("Name", QVariant(QString, "Anne Meara"))("PrimaryImageTag", QVariant(QString, "38334dd2cf9359d71338306b8f06ff6c"))("Role", QVariant(QString, "Miriam"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "4623763b5546b9519c75cf0139196637"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("5ce758d1ef16fca8eee726ae07786edb", QVariant(QString, "dSL}BED%?bt7of%MD%of00xuxuRjIUD%Rjt7M{RjRjRj"))))))))("Name", QVariant(QString, "Dexter Gordon"))("PrimaryImageTag", QVariant(QString, "5ce758d1ef16fca8eee726ae07786edb"))("Role", QVariant(QString, "Rolando"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "43103bb2d085e36c583e272b7a3b8676"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("198944f534025f1d0e56931f3aca34e0", QVariant(QString, "dPCrTIxZ0gNG-noeENNaI;ayoKoeI;WVsAn%EMWV%1oe"))))))))("Name", QVariant(QString, "George Martin"))("PrimaryImageTag", QVariant(QString, "198944f534025f1d0e56931f3aca34e0"))("Role", QVariant(QString, "Frank"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "cb4021db4eddfc5b7ca04157c19d8e75"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("fc1d2756dee53e04455f960121c889dd", QVariant(QString, "dOI#x_-;xu%M~qj[%Mxu?bt7-;%M-;Rjxuxu-;j[M{of"))))))))("Name", QVariant(QString, "Alice Drummond"))("PrimaryImageTag", QVariant(QString, "fc1d2756dee53e04455f960121c889dd"))("Role", QVariant(QString, "Lucy Fishman"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "78f2e4177652908b060f03356a483b6e"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("742692385479aafe6bf36fd2c2ddd098", QVariant(QString, "dHF#O9?GGZOT}s-VEgNtK5WEM_M{J-NGxGn%bbW.NGax"))))))))("Name", QVariant(QString, "Richard Libertini"))("PrimaryImageTag", QVariant(QString, "742692385479aafe6bf36fd2c2ddd098"))("Role", QVariant(QString, "Sidney"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "05b6266ea0d7fa12f3181e68d1560fe9"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("1490ba6580e06279fd697a62317ee623", QVariant(QString, "dhHTv,xa9]s:~AxaS3WCS~ae%1WVf+NHozoLt7WBbbWV"))))))))("Name", QVariant(QString, "Laura Esterman"))("PrimaryImageTag", QVariant(QString, "1490ba6580e06279fd697a62317ee623"))("Role", QVariant(QString, "Lolly"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "2e4294ea6f1bf2a5301820604abf8908"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("e3821d4c64d6555291f5dbf997373711", QVariant(QString, "dDECIHDh%#~WDOx_E1RO%gSgIo-;%Maeo#kCg3xuxuWq"))))))))("Name", QVariant(QString, "Barton Heyman"))("PrimaryImageTag", QVariant(QString, "e3821d4c64d6555291f5dbf997373711"))("Role", QVariant(QString, "Bert"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "2b3ceebe12bdcf784df297f83a8c02f8"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("9c0b1d7bb7c85a4253d7ca5f8aed8589", QVariant(QString, "dTFY$2~qM{D%xut7-;t7%MRjxuxuRjay-;t7%Mj[xuof"))))))))("Name", QVariant(QString, "Judith Malina"))("PrimaryImageTag", QVariant(QString, "9c0b1d7bb7c85a4253d7ca5f8aed8589"))("Role", QVariant(QString, "Rose"))("Type", QVariant(QString, "Actor")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "013c55b24f56cb1de6dd1f6a5eac94cb"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("beb67d47985374651df5bf46b7cd1720", QVariant(QString, "dPK+ip=e03-Bv3wc}YVtNHS#t7S3$PV@ozxt#9$jwdRj"))))))))("Name", QVariant(QString, "Penny Marshall"))("PrimaryImageTag", QVariant(QString, "beb67d47985374651df5bf46b7cd1720"))("Role", QVariant(QString, "Director"))("Type", QVariant(QString, "Director")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "b9ba02a3961e37ed33826db09cd09b94"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("eac460613415ca20d75db98ca26a5a75", QVariant(QString, "dYFXuANG0fs:IUt7ofWBAZe.$jayD%W;%2oLt8WBn$bH"))))))))("Name", QVariant(QString, "Steven Zaillian"))("PrimaryImageTag", QVariant(QString, "eac460613415ca20d75db98ca26a5a75"))("Role", QVariant(QString, "Screenplay"))("Type", QVariant(QString, "Writer")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "1da203a623bb0bb032469663ede89705"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("e6e6b28d264770ced074fd1dd0ebf8ca", QVariant(QString, "dnJHdH?uXNR%~BxbNGV@NaRjRjWBtQR+aeWBV@RkjGaf"))))))))("Name", QVariant(QString, "Oliver Sacks"))("PrimaryImageTag", QVariant(QString, "e6e6b28d264770ced074fd1dd0ebf8ca"))("Role", QVariant(QString, "Book"))("Type", QVariant(QString, "Writer")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "853b203a00d73da16fbdd20ac394e524"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap())))))("Name", QVariant(QString, "Walter F. Parkes"))("Role", QVariant(QString, "Producer"))("Type", QVariant(QString, "Producer")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "fd6ee8d6266c09b69d568199e639348e"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap())))))("Name", QVariant(QString, "Arne Schmidt"))("Role", QVariant(QString, "Executive Producer"))("Type", QVariant(QString, "Producer")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "fb96bf5143e7329f22142aa871d3d29b"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap())))))("Name", QVariant(QString, "Lawrence Lasker"))("Role", QVariant(QString, "Producer"))("Type", QVariant(QString, "Producer")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "013c55b24f56cb1de6dd1f6a5eac94cb"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap(("beb67d47985374651df5bf46b7cd1720", QVariant(QString, "dPK+ip=e03-Bv3wc}YVtNHS#t7S3$PV@ozxt#9$jwdRj"))))))))("Name", QVariant(QString, "Penny Marshall"))("PrimaryImageTag", QVariant(QString, "beb67d47985374651df5bf46b7cd1720"))("Role", QVariant(QString, "Executive Producer"))("Type", QVariant(QString, "Producer")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "99be0bbd118b99d9427bb81df0b310a0"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap())))))("Name", QVariant(QString, "Elliot Abbott"))("Role", QVariant(QString, "Executive Producer"))("Type", QVariant(QString, "Producer")))), QVariant(QVariantMap, QMap(("Id", QVariant(QString, "54242bdba4be250d01c7aa1b89ae8806"))("ImageBlurHashes", QVariant(QVariantMap, QMap(("Primary", QVariant(QVariantMap, QMap())))))("Name", QVariant(QString, "Amy Lemisch"))("Role", QVariant(QString, "Associate Producer"))("Type", QVariant(QString, "Producer")))))) 
Key:  "PlayAccess"  Value:  QVariant(QString, "Full") 
Key:  "PremiereDate"  Value:  QVariant(QString, "1990-12-04T00:00:00.0000000Z") 
Key:  "PrimaryImageAspectRatio"  Value:  QVariant(double, 0.666667) 
Key:  "ProductionLocations"  Value:  QVariant(QVariantList, (QVariant(QString, "United States of America"))) 
Key:  "ProductionYear"  Value:  QVariant(qlonglong, 1990) 
Key:  "ProviderIds"  Value:  QVariant(QVariantMap, QMap(("Imdb", QVariant(QString, "tt0099077"))("Tmdb", QVariant(QString, "11005")))) 
Key:  "RemoteTrailers"  Value:  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("Name", QVariant(QString, "Awakenings (1990) Trailer #1 | Movieclips Classic Trailers"))("Url", QVariant(QString, "https://www.youtube.com/watch?v=7exeVt7CaE4")))), QVariant(QVariantMap, QMap(("Name", QVariant(QString, "Awakenings 1990 TV trailer"))("Url", QVariant(QString, "https://www.youtube.com/watch?v=GgWMS1KTSBA")))))) 
Key:  "RunTimeTicks"  Value:  QVariant(qlonglong, 72411512832) 
Key:  "ServerId"  Value:  QVariant(QString, "dee86eb4af42483a9163236958775427") 
Key:  "SortName"  Value:  QVariant(QString, "awakenings") 
Key:  "SpecialFeatureCount"  Value:  QVariant(qlonglong, 0) 
Key:  "Studios"  Value:  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("Id", QVariant(QString, "52bc14c019eae1aa0f5b123c45253dfc"))("Name", QVariant(QString, "Parkes/Lasker productions")))))) 
Key:  "Taglines"  Value:  QVariant(QVariantList, (QVariant(QString, "There is no such thing as a simple miracle."))) 
Key:  "Tags"  Value:  QVariant(QVariantList, (QVariant(QString, "coma"), QVariant(QString, "based on novel or book"), QVariant(QString, "experiment"), QVariant(QString, "miracle"), QVariant(QString, "hope"), QVariant(QString, "based on true story"), QVariant(QString, "hospital"), QVariant(QString, "illness"), QVariant(QString, "woman director"), QVariant(QString, "comatose"))) 
Key:  "Type"  Value:  QVariant(QString, "Movie") 
Key:  "UserData"  Value:  QVariant(QVariantMap, QMap(("IsFavorite", QVariant(bool, false))("Key", QVariant(QString, "11005"))("LastPlayedDate", QVariant(QString, "2023-01-22T21:57:06.1890699Z"))("PlayCount", QVariant(qlonglong, 2))("PlaybackPositionTicks", QVariant(qlonglong, 0))("Played", QVariant(bool, false)))) 
Key:  "VideoType"  Value:  QVariant(QString, "VideoFile") 
Key:  "Width"  Value:  QVariant(qlonglong, 1920) 
Key:  "playOptions"  Value:  QVariant(QVariantMap, QMap(("fullscreen", QVariant(bool, true))("isFirstItem", QVariant(bool, true))("items", QVariant(std::nullptr_t, (nullptr)))("startIndex", QVariant(qlonglong, 0))("startPositionTicks", QVariant(qlonglong, 0)))) 
*/
discord::Core* core{};

bool DiscordComponent::componentInitialize() {
    QTimer *timer = new QTimer(this);

    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(RunCallbacks()));

    timer->start(1000);
    
    connect(&PlayerComponent::Get(), &PlayerComponent::onMetaData, this, &DiscordComponent::onMetaData);
    connect(&PlayerComponent::Get(), &PlayerComponent::updateDuration, this, &DiscordComponent::onUpdateDuration);
    connect(&PlayerComponent::Get(), &PlayerComponent::stopped, this, &DiscordComponent::onStop);

    return true;
}

void DiscordComponent::RunCallbacks() {
    core->RunCallbacks();

}

void DiscordComponent::componentPostInitialize() {
    auto discordCore = discord::Core::Create(1063276729617092729, DiscordCreateFlags_Default, &core);
    // auto activityManager = core->ActivityManager();
    // auto userManager = discord::Core.UserManager();

    discord::Activity activity = buildMenuActivity();

    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result == discord::Result::Ok) {
		    QLOG_DEBUG() << "Discord : New activity success";
            
        } else {
		    QLOG_DEBUG() << "Discord : Error = " << (int)result;
        }
    });    
}

void DiscordComponent::onMetaData(const QVariantMap& meta, QUrl baseUrl) {
    metadata = meta;
    m_baseUrl = baseUrl;

    // for (auto i = meta.begin(); i != meta.end(); i++) {
    //     QLOG_DEBUG() << "Key: " << i.key() << " Value: " << i.value();
    // }
     
}

void DiscordComponent::onUpdateDuration(qint64 duration) {
    // m_duration = duration;
    discord::Activity activity = buildWatchingActivity();

    auto start = metadata["playOptions"].toMap()["startPositionTicks"].toLongLong();
    qint64 formatted = duration + QDateTime::currentMSecsSinceEpoch();
    activity.GetTimestamps().SetEnd(formatted - start);

    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result == discord::Result::Ok) {
		    QLOG_DEBUG() << "Discord : New activity success";
        } else {
		    QLOG_DEBUG() << "Discord : Error = " << (int)result;
        }
    }); 
}

void DiscordComponent::handlePositionUpdate(quint64 position) {
    // discord::Activity activity = buildActivity();


    // core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
    //     if (result == discord::Result::Ok) {
	// 	    QLOG_DEBUG() << "Discord : New activity success";
    //     } else {
	// 	    QLOG_DEBUG() << "Discord : Error = " << (int)result;
    //     }
    // }); 
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
    activity.SetType(discord::ActivityType::Watching);

    QLOG_DEBUG() << "Discord: set activity to menu";

    return activity;
}

void DiscordComponent::onStop() {
    discord::Activity activity = buildMenuActivity();

    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result == discord::Result::Ok) {
		    QLOG_DEBUG() << "Discord : New activity success";
            
        } else {
		    QLOG_DEBUG() << "Discord : Error = " << (int)result;
        }
    }); 
}
