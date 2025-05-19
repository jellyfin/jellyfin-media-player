/* Generated with <3 by Discord.Sdk.Derive */
#pragma once
#ifndef DISCORD_HEADER_CDISCORD_H_
#define DISCORD_HEADER_CDISCORD_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef DISCORD_API
#if defined(_WIN32)
#define DISCORD_API __declspec(dllexport)
#pragma warning(disable : 4251)
#else
#define DISCORD_API __attribute__((visibility("default")))
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct Discord_String;

typedef struct Discord_Properties {
    size_t size;
    struct Discord_String* keys;
    struct Discord_String* values;
} Discord_Properties;

typedef void* (*Discord_MallocFn)(size_t size);
typedef void (*Discord_FreeFn)(void* ptr);

typedef struct Discord_Allocator {
    Discord_MallocFn fnMalloc;
    Discord_FreeFn fnFree;
} Discord_Allocator;

DISCORD_API void Discord_SetAllocator(Discord_Allocator* allocator);
DISCORD_API void* Discord_Alloc(size_t size);
DISCORD_API void Discord_Free(void* ptr);
DISCORD_API void Discord_FreeProperties(Discord_Properties props);

DISCORD_API void Discord_SetFreeThreaded();
DISCORD_API void Discord_ResetCallbacks();
DISCORD_API void Discord_RunCallbacks();

typedef enum Discord_ActivityActionTypes {
    Discord_ActivityActionTypes_Join = 1,
    Discord_ActivityActionTypes_JoinRequest = 5,
    Discord_ActivityActionTypes_forceint = 0x7FFFFFFF
} Discord_ActivityActionTypes;

typedef enum Discord_ActivityPartyPrivacy {
    Discord_ActivityPartyPrivacy_Private = 0,
    Discord_ActivityPartyPrivacy_Public = 1,
    Discord_ActivityPartyPrivacy_forceint = 0x7FFFFFFF
} Discord_ActivityPartyPrivacy;

typedef enum Discord_ActivityTypes {
    Discord_ActivityTypes_Playing = 0,
    Discord_ActivityTypes_Streaming = 1,
    Discord_ActivityTypes_Listening = 2,
    Discord_ActivityTypes_Watching = 3,
    Discord_ActivityTypes_CustomStatus = 4,
    Discord_ActivityTypes_Competing = 5,
    Discord_ActivityTypes_HangStatus = 6,
    Discord_ActivityTypes_forceint = 0x7FFFFFFF
} Discord_ActivityTypes;

typedef enum Discord_ActivityGamePlatforms {
    Discord_ActivityGamePlatforms_Desktop = 1,
    Discord_ActivityGamePlatforms_Xbox = 2,
    Discord_ActivityGamePlatforms_Samsung = 4,
    Discord_ActivityGamePlatforms_IOS = 8,
    Discord_ActivityGamePlatforms_Android = 16,
    Discord_ActivityGamePlatforms_Embedded = 32,
    Discord_ActivityGamePlatforms_PS4 = 64,
    Discord_ActivityGamePlatforms_PS5 = 128,
    Discord_ActivityGamePlatforms_forceint = 0x7FFFFFFF
} Discord_ActivityGamePlatforms;

typedef enum Discord_ErrorType {
    Discord_ErrorType_None = 0,
    Discord_ErrorType_NetworkError = 1,
    Discord_ErrorType_HTTPError = 2,
    Discord_ErrorType_ClientNotReady = 3,
    Discord_ErrorType_Disabled = 4,
    Discord_ErrorType_ClientDestroyed = 5,
    Discord_ErrorType_ValidationError = 6,
    Discord_ErrorType_Aborted = 7,
    Discord_ErrorType_AuthorizationFailed = 8,
    Discord_ErrorType_RPCError = 9,
    Discord_ErrorType_forceint = 0x7FFFFFFF
} Discord_ErrorType;

typedef enum Discord_HttpStatusCode {
    Discord_HttpStatusCode_None = 0,
    Discord_HttpStatusCode_Continue = 100,
    Discord_HttpStatusCode_SwitchingProtocols = 101,
    Discord_HttpStatusCode_Processing = 102,
    Discord_HttpStatusCode_EarlyHints = 103,
    Discord_HttpStatusCode_Ok = 200,
    Discord_HttpStatusCode_Created = 201,
    Discord_HttpStatusCode_Accepted = 202,
    Discord_HttpStatusCode_NonAuthoritativeInfo = 203,
    Discord_HttpStatusCode_NoContent = 204,
    Discord_HttpStatusCode_ResetContent = 205,
    Discord_HttpStatusCode_PartialContent = 206,
    Discord_HttpStatusCode_MultiStatus = 207,
    Discord_HttpStatusCode_AlreadyReported = 208,
    Discord_HttpStatusCode_ImUsed = 209,
    Discord_HttpStatusCode_MultipleChoices = 300,
    Discord_HttpStatusCode_MovedPermanently = 301,
    Discord_HttpStatusCode_Found = 302,
    Discord_HttpStatusCode_SeeOther = 303,
    Discord_HttpStatusCode_NotModified = 304,
    Discord_HttpStatusCode_TemporaryRedirect = 307,
    Discord_HttpStatusCode_PermanentRedirect = 308,
    Discord_HttpStatusCode_BadRequest = 400,
    Discord_HttpStatusCode_Unauthorized = 401,
    Discord_HttpStatusCode_PaymentRequired = 402,
    Discord_HttpStatusCode_Forbidden = 403,
    Discord_HttpStatusCode_NotFound = 404,
    Discord_HttpStatusCode_MethodNotAllowed = 405,
    Discord_HttpStatusCode_NotAcceptable = 406,
    Discord_HttpStatusCode_ProxyAuthRequired = 407,
    Discord_HttpStatusCode_RequestTimeout = 408,
    Discord_HttpStatusCode_Conflict = 409,
    Discord_HttpStatusCode_Gone = 410,
    Discord_HttpStatusCode_LengthRequired = 411,
    Discord_HttpStatusCode_PreconditionFailed = 412,
    Discord_HttpStatusCode_PayloadTooLarge = 413,
    Discord_HttpStatusCode_UriTooLong = 414,
    Discord_HttpStatusCode_UnsupportedMediaType = 415,
    Discord_HttpStatusCode_RangeNotSatisfiable = 416,
    Discord_HttpStatusCode_ExpectationFailed = 417,
    Discord_HttpStatusCode_MisdirectedRequest = 421,
    Discord_HttpStatusCode_UnprocessableEntity = 422,
    Discord_HttpStatusCode_Locked = 423,
    Discord_HttpStatusCode_FailedDependency = 424,
    Discord_HttpStatusCode_TooEarly = 425,
    Discord_HttpStatusCode_UpgradeRequired = 426,
    Discord_HttpStatusCode_PreconditionRequired = 428,
    Discord_HttpStatusCode_TooManyRequests = 429,
    Discord_HttpStatusCode_RequestHeaderFieldsTooLarge = 431,
    Discord_HttpStatusCode_InternalServerError = 500,
    Discord_HttpStatusCode_NotImplemented = 501,
    Discord_HttpStatusCode_BadGateway = 502,
    Discord_HttpStatusCode_ServiceUnavailable = 503,
    Discord_HttpStatusCode_GatewayTimeout = 504,
    Discord_HttpStatusCode_HttpVersionNotSupported = 505,
    Discord_HttpStatusCode_VariantAlsoNegotiates = 506,
    Discord_HttpStatusCode_InsufficientStorage = 507,
    Discord_HttpStatusCode_LoopDetected = 508,
    Discord_HttpStatusCode_NotExtended = 510,
    Discord_HttpStatusCode_NetworkAuthorizationRequired = 511,
    Discord_HttpStatusCode_forceint = 0x7FFFFFFF
} Discord_HttpStatusCode;

typedef enum Discord_AuthenticationCodeChallengeMethod {
    Discord_AuthenticationCodeChallengeMethod_S256 = 0,
    Discord_AuthenticationCodeChallengeMethod_forceint = 0x7FFFFFFF
} Discord_AuthenticationCodeChallengeMethod;

typedef enum Discord_AdditionalContentType {
    Discord_AdditionalContentType_Other = 0,
    Discord_AdditionalContentType_Attachment = 1,
    Discord_AdditionalContentType_Poll = 2,
    Discord_AdditionalContentType_VoiceMessage = 3,
    Discord_AdditionalContentType_Thread = 4,
    Discord_AdditionalContentType_Embed = 5,
    Discord_AdditionalContentType_Sticker = 6,
    Discord_AdditionalContentType_forceint = 0x7FFFFFFF
} Discord_AdditionalContentType;

typedef enum Discord_Call_Error {
    Discord_Call_Error_None = 0,
    Discord_Call_Error_SignalingConnectionFailed = 1,
    Discord_Call_Error_SignalingUnexpectedClose = 2,
    Discord_Call_Error_VoiceConnectionFailed = 3,
    Discord_Call_Error_JoinTimeout = 4,
    Discord_Call_Error_Forbidden = 5,
    Discord_Call_Error_forceint = 0x7FFFFFFF
} Discord_Call_Error;

typedef enum Discord_AudioModeType {
    Discord_AudioModeType_MODE_UNINIT = 0,
    Discord_AudioModeType_MODE_VAD = 1,
    Discord_AudioModeType_MODE_PTT = 2,
    Discord_AudioModeType_forceint = 0x7FFFFFFF
} Discord_AudioModeType;

typedef enum Discord_Call_Status {
    Discord_Call_Status_Disconnected = 0,
    Discord_Call_Status_Joining = 1,
    Discord_Call_Status_Connecting = 2,
    Discord_Call_Status_SignalingConnected = 3,
    Discord_Call_Status_Connected = 4,
    Discord_Call_Status_Reconnecting = 5,
    Discord_Call_Status_Disconnecting = 6,
    Discord_Call_Status_forceint = 0x7FFFFFFF
} Discord_Call_Status;

typedef enum Discord_ChannelType {
    Discord_ChannelType_GuildText = 0,
    Discord_ChannelType_Dm = 1,
    Discord_ChannelType_GuildVoice = 2,
    Discord_ChannelType_GroupDm = 3,
    Discord_ChannelType_GuildCategory = 4,
    Discord_ChannelType_GuildNews = 5,
    Discord_ChannelType_GuildStore = 6,
    Discord_ChannelType_GuildNewsThread = 10,
    Discord_ChannelType_GuildPublicThread = 11,
    Discord_ChannelType_GuildPrivateThread = 12,
    Discord_ChannelType_GuildStageVoice = 13,
    Discord_ChannelType_GuildDirectory = 14,
    Discord_ChannelType_GuildForum = 15,
    Discord_ChannelType_GuildMedia = 16,
    Discord_ChannelType_Lobby = 17,
    Discord_ChannelType_EphemeralDm = 18,
    Discord_ChannelType_forceint = 0x7FFFFFFF
} Discord_ChannelType;

typedef enum Discord_RelationshipType {
    Discord_RelationshipType_None = 0,
    Discord_RelationshipType_Friend = 1,
    Discord_RelationshipType_Blocked = 2,
    Discord_RelationshipType_PendingIncoming = 3,
    Discord_RelationshipType_PendingOutgoing = 4,
    Discord_RelationshipType_Implicit = 5,
    Discord_RelationshipType_Suggestion = 6,
    Discord_RelationshipType_forceint = 0x7FFFFFFF
} Discord_RelationshipType;

typedef enum Discord_UserHandle_AvatarType {
    Discord_UserHandle_AvatarType_Gif = 0,
    Discord_UserHandle_AvatarType_Webp = 1,
    Discord_UserHandle_AvatarType_Png = 2,
    Discord_UserHandle_AvatarType_Jpeg = 3,
    Discord_UserHandle_AvatarType_forceint = 0x7FFFFFFF
} Discord_UserHandle_AvatarType;

typedef enum Discord_StatusType {
    Discord_StatusType_Online = 0,
    Discord_StatusType_Offline = 1,
    Discord_StatusType_Blocked = 2,
    Discord_StatusType_Idle = 3,
    Discord_StatusType_Dnd = 4,
    Discord_StatusType_Invisible = 5,
    Discord_StatusType_Streaming = 6,
    Discord_StatusType_Unknown = 7,
    Discord_StatusType_forceint = 0x7FFFFFFF
} Discord_StatusType;

typedef enum Discord_DisclosureTypes {
    Discord_DisclosureTypes_MessageDataVisibleOnDiscord = 3,
    Discord_DisclosureTypes_forceint = 0x7FFFFFFF
} Discord_DisclosureTypes;

typedef enum Discord_Client_Error {
    Discord_Client_Error_None = 0,
    Discord_Client_Error_ConnectionFailed = 1,
    Discord_Client_Error_UnexpectedClose = 2,
    Discord_Client_Error_ConnectionCanceled = 3,
    Discord_Client_Error_forceint = 0x7FFFFFFF
} Discord_Client_Error;

typedef enum Discord_Client_Status {
    Discord_Client_Status_Disconnected = 0,
    Discord_Client_Status_Connecting = 1,
    Discord_Client_Status_Connected = 2,
    Discord_Client_Status_Ready = 3,
    Discord_Client_Status_Reconnecting = 4,
    Discord_Client_Status_Disconnecting = 5,
    Discord_Client_Status_HttpWait = 6,
    Discord_Client_Status_forceint = 0x7FFFFFFF
} Discord_Client_Status;

typedef enum Discord_Client_Thread {
    Discord_Client_Thread_Client = 0,
    Discord_Client_Thread_Voice = 1,
    Discord_Client_Thread_Network = 2,
    Discord_Client_Thread_forceint = 0x7FFFFFFF
} Discord_Client_Thread;

typedef enum Discord_AuthorizationTokenType {
    Discord_AuthorizationTokenType_User = 0,
    Discord_AuthorizationTokenType_Bearer = 1,
    Discord_AuthorizationTokenType_forceint = 0x7FFFFFFF
} Discord_AuthorizationTokenType;

typedef enum Discord_AuthenticationExternalAuthType {
    Discord_AuthenticationExternalAuthType_OIDC = 0,
    Discord_AuthenticationExternalAuthType_EpicOnlineServicesAccessToken = 1,
    Discord_AuthenticationExternalAuthType_EpicOnlineServicesIdToken = 2,
    Discord_AuthenticationExternalAuthType_SteamSessionTicket = 3,
    Discord_AuthenticationExternalAuthType_UnityServicesIdToken = 4,
    Discord_AuthenticationExternalAuthType_forceint = 0x7FFFFFFF
} Discord_AuthenticationExternalAuthType;

typedef enum Discord_LoggingSeverity {
    Discord_LoggingSeverity_Verbose = 1,
    Discord_LoggingSeverity_Info = 2,
    Discord_LoggingSeverity_Warning = 3,
    Discord_LoggingSeverity_Error = 4,
    Discord_LoggingSeverity_None = 5,
    Discord_LoggingSeverity_forceint = 0x7FFFFFFF
} Discord_LoggingSeverity;

typedef struct Discord_ActivityInvite Discord_ActivityInvite;
typedef struct Discord_ActivityAssets Discord_ActivityAssets;
typedef struct Discord_ActivityTimestamps Discord_ActivityTimestamps;
typedef struct Discord_ActivityParty Discord_ActivityParty;
typedef struct Discord_ActivitySecrets Discord_ActivitySecrets;
typedef struct Discord_Activity Discord_Activity;
typedef struct Discord_ClientResult Discord_ClientResult;
typedef struct Discord_AuthorizationCodeChallenge Discord_AuthorizationCodeChallenge;
typedef struct Discord_AuthorizationCodeVerifier Discord_AuthorizationCodeVerifier;
typedef struct Discord_AuthorizationArgs Discord_AuthorizationArgs;
typedef struct Discord_DeviceAuthorizationArgs Discord_DeviceAuthorizationArgs;
typedef struct Discord_VoiceStateHandle Discord_VoiceStateHandle;
typedef struct Discord_VADThresholdSettings Discord_VADThresholdSettings;
typedef struct Discord_Call Discord_Call;
typedef struct Discord_ChannelHandle Discord_ChannelHandle;
typedef struct Discord_GuildMinimal Discord_GuildMinimal;
typedef struct Discord_GuildChannel Discord_GuildChannel;
typedef struct Discord_LinkedLobby Discord_LinkedLobby;
typedef struct Discord_LinkedChannel Discord_LinkedChannel;
typedef struct Discord_RelationshipHandle Discord_RelationshipHandle;
typedef struct Discord_UserHandle Discord_UserHandle;
typedef struct Discord_LobbyMemberHandle Discord_LobbyMemberHandle;
typedef struct Discord_LobbyHandle Discord_LobbyHandle;
typedef struct Discord_AdditionalContent Discord_AdditionalContent;
typedef struct Discord_MessageHandle Discord_MessageHandle;
typedef struct Discord_AudioDevice Discord_AudioDevice;
typedef struct Discord_Client Discord_Client;
typedef struct Discord_CallInfoHandle Discord_CallInfoHandle;
typedef struct Discord_String {
    uint8_t* ptr;
    size_t size;
} Discord_String;
typedef struct Discord_UInt64Span {
    uint64_t* ptr;
    size_t size;
} Discord_UInt64Span;
typedef struct Discord_LobbyMemberHandleSpan {
    Discord_LobbyMemberHandle* ptr;
    size_t size;
} Discord_LobbyMemberHandleSpan;
typedef struct Discord_CallSpan {
    Discord_Call* ptr;
    size_t size;
} Discord_CallSpan;
typedef struct Discord_AudioDeviceSpan {
    Discord_AudioDevice* ptr;
    size_t size;
} Discord_AudioDeviceSpan;
typedef struct Discord_GuildChannelSpan {
    Discord_GuildChannel* ptr;
    size_t size;
} Discord_GuildChannelSpan;
typedef struct Discord_GuildMinimalSpan {
    Discord_GuildMinimal* ptr;
    size_t size;
} Discord_GuildMinimalSpan;
typedef struct Discord_RelationshipHandleSpan {
    Discord_RelationshipHandle* ptr;
    size_t size;
} Discord_RelationshipHandleSpan;
typedef struct Discord_UserHandleSpan {
    Discord_UserHandle* ptr;
    size_t size;
} Discord_UserHandleSpan;
typedef void (*Discord_Call_OnVoiceStateChanged)(uint64_t userId, void* userData);
typedef void (*Discord_Call_OnParticipantChanged)(uint64_t userId, bool added, void* userData);
typedef void (*Discord_Call_OnSpeakingStatusChanged)(uint64_t userId,
                                                     bool isPlayingSound,
                                                     void* userData);
typedef void (*Discord_Call_OnStatusChanged)(Discord_Call_Status status,
                                             Discord_Call_Error error,
                                             int32_t errorDetail,
                                             void* userData);
typedef void (*Discord_Client_EndCallCallback)(void* userData);
typedef void (*Discord_Client_EndCallsCallback)(void* userData);
typedef void (*Discord_Client_GetCurrentInputDeviceCallback)(Discord_AudioDevice const* device,
                                                             void* userData);
typedef void (*Discord_Client_GetCurrentOutputDeviceCallback)(Discord_AudioDevice const* device,
                                                              void* userData);
typedef void (*Discord_Client_GetInputDevicesCallback)(Discord_AudioDeviceSpan devices,
                                                       void* userData);
typedef void (*Discord_Client_GetOutputDevicesCallback)(Discord_AudioDeviceSpan devices,
                                                        void* userData);
typedef void (*Discord_Client_DeviceChangeCallback)(Discord_AudioDeviceSpan inputDevices,
                                                    Discord_AudioDeviceSpan outputDevices,
                                                    void* userData);
typedef void (*Discord_Client_SetInputDeviceCallback)(Discord_ClientResult* result, void* userData);
typedef void (*Discord_Client_NoAudioInputCallback)(bool inputDetected, void* userData);
typedef void (*Discord_Client_SetOutputDeviceCallback)(Discord_ClientResult* result,
                                                       void* userData);
typedef void (*Discord_Client_VoiceParticipantChangedCallback)(uint64_t lobbyId,
                                                               uint64_t memberId,
                                                               bool added,
                                                               void* userData);
typedef void (*Discord_Client_UserAudioReceivedCallback)(uint64_t userId,
                                                         int16_t* data,
                                                         uint64_t samplesPerChannel,
                                                         int32_t sampleRate,
                                                         uint64_t channels,
                                                         bool* outShouldMute,
                                                         void* userData);
typedef void (*Discord_Client_UserAudioCapturedCallback)(int16_t* data,
                                                         uint64_t samplesPerChannel,
                                                         int32_t sampleRate,
                                                         uint64_t channels,
                                                         void* userData);
typedef void (*Discord_Client_AuthorizationCallback)(Discord_ClientResult* result,
                                                     Discord_String code,
                                                     Discord_String redirectUri,
                                                     void* userData);
typedef void (*Discord_Client_FetchCurrentUserCallback)(Discord_ClientResult* result,
                                                        uint64_t id,
                                                        Discord_String name,
                                                        void* userData);
typedef void (*Discord_Client_TokenExchangeCallback)(Discord_ClientResult* result,
                                                     Discord_String accessToken,
                                                     Discord_String refreshToken,
                                                     Discord_AuthorizationTokenType tokenType,
                                                     int32_t expiresIn,
                                                     Discord_String scopes,
                                                     void* userData);
typedef void (*Discord_Client_AuthorizeDeviceScreenClosedCallback)(void* userData);
typedef void (*Discord_Client_TokenExpirationCallback)(void* userData);
typedef void (*Discord_Client_UpdateProvisionalAccountDisplayNameCallback)(
  Discord_ClientResult* result,
  void* userData);
typedef void (*Discord_Client_UpdateTokenCallback)(Discord_ClientResult* result, void* userData);
typedef void (*Discord_Client_DeleteUserMessageCallback)(Discord_ClientResult* result,
                                                         void* userData);
typedef void (*Discord_Client_EditUserMessageCallback)(Discord_ClientResult* result,
                                                       void* userData);
typedef void (*Discord_Client_ProvisionalUserMergeRequiredCallback)(void* userData);
typedef void (*Discord_Client_OpenMessageInDiscordCallback)(Discord_ClientResult* result,
                                                            void* userData);
typedef void (*Discord_Client_SendUserMessageCallback)(Discord_ClientResult* result,
                                                       uint64_t messageId,
                                                       void* userData);
typedef void (*Discord_Client_MessageCreatedCallback)(uint64_t messageId, void* userData);
typedef void (*Discord_Client_MessageDeletedCallback)(uint64_t messageId,
                                                      uint64_t channelId,
                                                      void* userData);
typedef void (*Discord_Client_MessageUpdatedCallback)(uint64_t messageId, void* userData);
typedef void (*Discord_Client_LogCallback)(Discord_String message,
                                           Discord_LoggingSeverity severity,
                                           void* userData);
typedef void (*Discord_Client_OnStatusChanged)(Discord_Client_Status status,
                                               Discord_Client_Error error,
                                               int32_t errorDetail,
                                               void* userData);
typedef void (*Discord_Client_CreateOrJoinLobbyCallback)(Discord_ClientResult* result,
                                                         uint64_t lobbyId,
                                                         void* userData);
typedef void (*Discord_Client_GetGuildChannelsCallback)(Discord_ClientResult* result,
                                                        Discord_GuildChannelSpan guildChannels,
                                                        void* userData);
typedef void (*Discord_Client_GetUserGuildsCallback)(Discord_ClientResult* result,
                                                     Discord_GuildMinimalSpan guilds,
                                                     void* userData);
typedef void (*Discord_Client_LeaveLobbyCallback)(Discord_ClientResult* result, void* userData);
typedef void (*Discord_Client_LinkOrUnlinkChannelCallback)(Discord_ClientResult* result,
                                                           void* userData);
typedef void (*Discord_Client_LobbyCreatedCallback)(uint64_t lobbyId, void* userData);
typedef void (*Discord_Client_LobbyDeletedCallback)(uint64_t lobbyId, void* userData);
typedef void (*Discord_Client_LobbyMemberAddedCallback)(uint64_t lobbyId,
                                                        uint64_t memberId,
                                                        void* userData);
typedef void (*Discord_Client_LobbyMemberRemovedCallback)(uint64_t lobbyId,
                                                          uint64_t memberId,
                                                          void* userData);
typedef void (*Discord_Client_LobbyMemberUpdatedCallback)(uint64_t lobbyId,
                                                          uint64_t memberId,
                                                          void* userData);
typedef void (*Discord_Client_LobbyUpdatedCallback)(uint64_t lobbyId, void* userData);
typedef void (*Discord_Client_AcceptActivityInviteCallback)(Discord_ClientResult* result,
                                                            Discord_String joinSecret,
                                                            void* userData);
typedef void (*Discord_Client_SendActivityInviteCallback)(Discord_ClientResult* result,
                                                          void* userData);
typedef void (*Discord_Client_ActivityInviteCallback)(Discord_ActivityInvite* invite,
                                                      void* userData);
typedef void (*Discord_Client_ActivityJoinCallback)(Discord_String joinSecret, void* userData);
typedef void (*Discord_Client_UpdateStatusCallback)(Discord_ClientResult* result, void* userData);
typedef void (*Discord_Client_UpdateRichPresenceCallback)(Discord_ClientResult* result,
                                                          void* userData);
typedef void (*Discord_Client_UpdateRelationshipCallback)(Discord_ClientResult* result,
                                                          void* userData);
typedef void (*Discord_Client_SendFriendRequestCallback)(Discord_ClientResult* result,
                                                         void* userData);
typedef void (*Discord_Client_RelationshipCreatedCallback)(uint64_t userId,
                                                           bool isDiscordRelationshipUpdate,
                                                           void* userData);
typedef void (*Discord_Client_RelationshipDeletedCallback)(uint64_t userId,
                                                           bool isDiscordRelationshipUpdate,
                                                           void* userData);
typedef void (*Discord_Client_GetDiscordClientConnectedUserCallback)(Discord_ClientResult* result,
                                                                     Discord_UserHandle* user,
                                                                     void* userData);
typedef void (*Discord_Client_UserUpdatedCallback)(uint64_t userId, void* userData);
struct Discord_ActivityInvite {
    void* opaque;
};

void DISCORD_API Discord_ActivityInvite_Init(Discord_ActivityInvite* self);
void DISCORD_API Discord_ActivityInvite_Drop(Discord_ActivityInvite* self);
void DISCORD_API Discord_ActivityInvite_Clone(Discord_ActivityInvite* self,
                                              Discord_ActivityInvite const* rhs);
void DISCORD_API Discord_ActivityInvite_SetSenderId(Discord_ActivityInvite* self, uint64_t value);
uint64_t DISCORD_API Discord_ActivityInvite_SenderId(Discord_ActivityInvite* self);
void DISCORD_API Discord_ActivityInvite_SetChannelId(Discord_ActivityInvite* self, uint64_t value);
uint64_t DISCORD_API Discord_ActivityInvite_ChannelId(Discord_ActivityInvite* self);
void DISCORD_API Discord_ActivityInvite_SetMessageId(Discord_ActivityInvite* self, uint64_t value);
uint64_t DISCORD_API Discord_ActivityInvite_MessageId(Discord_ActivityInvite* self);
void DISCORD_API Discord_ActivityInvite_SetType(Discord_ActivityInvite* self,
                                                Discord_ActivityActionTypes value);
Discord_ActivityActionTypes DISCORD_API Discord_ActivityInvite_Type(Discord_ActivityInvite* self);
void DISCORD_API Discord_ActivityInvite_SetApplicationId(Discord_ActivityInvite* self,
                                                         uint64_t value);
uint64_t DISCORD_API Discord_ActivityInvite_ApplicationId(Discord_ActivityInvite* self);
void DISCORD_API Discord_ActivityInvite_SetPartyId(Discord_ActivityInvite* self,
                                                   Discord_String value);
void DISCORD_API Discord_ActivityInvite_PartyId(Discord_ActivityInvite* self,
                                                Discord_String* returnValue);
void DISCORD_API Discord_ActivityInvite_SetSessionId(Discord_ActivityInvite* self,
                                                     Discord_String value);
void DISCORD_API Discord_ActivityInvite_SessionId(Discord_ActivityInvite* self,
                                                  Discord_String* returnValue);
void DISCORD_API Discord_ActivityInvite_SetIsValid(Discord_ActivityInvite* self, bool value);
bool DISCORD_API Discord_ActivityInvite_IsValid(Discord_ActivityInvite* self);
struct Discord_ActivityAssets {
    void* opaque;
};

void DISCORD_API Discord_ActivityAssets_Init(Discord_ActivityAssets* self);
void DISCORD_API Discord_ActivityAssets_Drop(Discord_ActivityAssets* self);
void DISCORD_API Discord_ActivityAssets_Clone(Discord_ActivityAssets* self,
                                              Discord_ActivityAssets const* arg0);
void DISCORD_API Discord_ActivityAssets_SetLargeImage(Discord_ActivityAssets* self,
                                                      Discord_String* value);
bool DISCORD_API Discord_ActivityAssets_LargeImage(Discord_ActivityAssets* self,
                                                   Discord_String* returnValue);
void DISCORD_API Discord_ActivityAssets_SetLargeText(Discord_ActivityAssets* self,
                                                     Discord_String* value);
bool DISCORD_API Discord_ActivityAssets_LargeText(Discord_ActivityAssets* self,
                                                  Discord_String* returnValue);
void DISCORD_API Discord_ActivityAssets_SetSmallImage(Discord_ActivityAssets* self,
                                                      Discord_String* value);
bool DISCORD_API Discord_ActivityAssets_SmallImage(Discord_ActivityAssets* self,
                                                   Discord_String* returnValue);
void DISCORD_API Discord_ActivityAssets_SetSmallText(Discord_ActivityAssets* self,
                                                     Discord_String* value);
bool DISCORD_API Discord_ActivityAssets_SmallText(Discord_ActivityAssets* self,
                                                  Discord_String* returnValue);
struct Discord_ActivityTimestamps {
    void* opaque;
};

void DISCORD_API Discord_ActivityTimestamps_Init(Discord_ActivityTimestamps* self);
void DISCORD_API Discord_ActivityTimestamps_Drop(Discord_ActivityTimestamps* self);
void DISCORD_API Discord_ActivityTimestamps_Clone(Discord_ActivityTimestamps* self,
                                                  Discord_ActivityTimestamps const* arg0);
void DISCORD_API Discord_ActivityTimestamps_SetStart(Discord_ActivityTimestamps* self,
                                                     uint64_t value);
uint64_t DISCORD_API Discord_ActivityTimestamps_Start(Discord_ActivityTimestamps* self);
void DISCORD_API Discord_ActivityTimestamps_SetEnd(Discord_ActivityTimestamps* self,
                                                   uint64_t value);
uint64_t DISCORD_API Discord_ActivityTimestamps_End(Discord_ActivityTimestamps* self);
struct Discord_ActivityParty {
    void* opaque;
};

void DISCORD_API Discord_ActivityParty_Init(Discord_ActivityParty* self);
void DISCORD_API Discord_ActivityParty_Drop(Discord_ActivityParty* self);
void DISCORD_API Discord_ActivityParty_Clone(Discord_ActivityParty* self,
                                             Discord_ActivityParty const* arg0);
void DISCORD_API Discord_ActivityParty_SetId(Discord_ActivityParty* self, Discord_String value);
void DISCORD_API Discord_ActivityParty_Id(Discord_ActivityParty* self, Discord_String* returnValue);
void DISCORD_API Discord_ActivityParty_SetCurrentSize(Discord_ActivityParty* self, int32_t value);
int32_t DISCORD_API Discord_ActivityParty_CurrentSize(Discord_ActivityParty* self);
void DISCORD_API Discord_ActivityParty_SetMaxSize(Discord_ActivityParty* self, int32_t value);
int32_t DISCORD_API Discord_ActivityParty_MaxSize(Discord_ActivityParty* self);
void DISCORD_API Discord_ActivityParty_SetPrivacy(Discord_ActivityParty* self,
                                                  Discord_ActivityPartyPrivacy value);
Discord_ActivityPartyPrivacy DISCORD_API Discord_ActivityParty_Privacy(Discord_ActivityParty* self);
struct Discord_ActivitySecrets {
    void* opaque;
};

void DISCORD_API Discord_ActivitySecrets_Init(Discord_ActivitySecrets* self);
void DISCORD_API Discord_ActivitySecrets_Drop(Discord_ActivitySecrets* self);
void DISCORD_API Discord_ActivitySecrets_Clone(Discord_ActivitySecrets* self,
                                               Discord_ActivitySecrets const* arg0);
void DISCORD_API Discord_ActivitySecrets_SetJoin(Discord_ActivitySecrets* self,
                                                 Discord_String value);
void DISCORD_API Discord_ActivitySecrets_Join(Discord_ActivitySecrets* self,
                                              Discord_String* returnValue);
struct Discord_Activity {
    void* opaque;
};

void DISCORD_API Discord_Activity_Init(Discord_Activity* self);
void DISCORD_API Discord_Activity_Drop(Discord_Activity* self);
void DISCORD_API Discord_Activity_Clone(Discord_Activity* self, Discord_Activity const* arg0);
bool DISCORD_API Discord_Activity_Equals(Discord_Activity* self, Discord_Activity const* other);
void DISCORD_API Discord_Activity_SetName(Discord_Activity* self, Discord_String value);
void DISCORD_API Discord_Activity_Name(Discord_Activity* self, Discord_String* returnValue);
void DISCORD_API Discord_Activity_SetType(Discord_Activity* self, Discord_ActivityTypes value);
Discord_ActivityTypes DISCORD_API Discord_Activity_Type(Discord_Activity* self);
void DISCORD_API Discord_Activity_SetState(Discord_Activity* self, Discord_String* value);
bool DISCORD_API Discord_Activity_State(Discord_Activity* self, Discord_String* returnValue);
void DISCORD_API Discord_Activity_SetDetails(Discord_Activity* self, Discord_String* value);
bool DISCORD_API Discord_Activity_Details(Discord_Activity* self, Discord_String* returnValue);
void DISCORD_API Discord_Activity_SetApplicationId(Discord_Activity* self, uint64_t* value);
bool DISCORD_API Discord_Activity_ApplicationId(Discord_Activity* self, uint64_t* returnValue);
void DISCORD_API Discord_Activity_SetAssets(Discord_Activity* self, Discord_ActivityAssets* value);
bool DISCORD_API Discord_Activity_Assets(Discord_Activity* self,
                                         Discord_ActivityAssets* returnValue);
void DISCORD_API Discord_Activity_SetTimestamps(Discord_Activity* self,
                                                Discord_ActivityTimestamps* value);
bool DISCORD_API Discord_Activity_Timestamps(Discord_Activity* self,
                                             Discord_ActivityTimestamps* returnValue);
void DISCORD_API Discord_Activity_SetParty(Discord_Activity* self, Discord_ActivityParty* value);
bool DISCORD_API Discord_Activity_Party(Discord_Activity* self, Discord_ActivityParty* returnValue);
void DISCORD_API Discord_Activity_SetSecrets(Discord_Activity* self,
                                             Discord_ActivitySecrets* value);
bool DISCORD_API Discord_Activity_Secrets(Discord_Activity* self,
                                          Discord_ActivitySecrets* returnValue);
void DISCORD_API Discord_Activity_SetSupportedPlatforms(Discord_Activity* self,
                                                        Discord_ActivityGamePlatforms value);
Discord_ActivityGamePlatforms DISCORD_API
Discord_Activity_SupportedPlatforms(Discord_Activity* self);
struct Discord_ClientResult {
    void* opaque;
};

void DISCORD_API Discord_ClientResult_Drop(Discord_ClientResult* self);
void DISCORD_API Discord_ClientResult_Clone(Discord_ClientResult* self,
                                            Discord_ClientResult const* arg0);
void DISCORD_API Discord_ClientResult_ToString(Discord_ClientResult* self,
                                               Discord_String* returnValue);
void DISCORD_API Discord_ClientResult_SetType(Discord_ClientResult* self, Discord_ErrorType value);
Discord_ErrorType DISCORD_API Discord_ClientResult_Type(Discord_ClientResult* self);
void DISCORD_API Discord_ClientResult_SetError(Discord_ClientResult* self, Discord_String value);
void DISCORD_API Discord_ClientResult_Error(Discord_ClientResult* self,
                                            Discord_String* returnValue);
void DISCORD_API Discord_ClientResult_SetErrorCode(Discord_ClientResult* self, int32_t value);
int32_t DISCORD_API Discord_ClientResult_ErrorCode(Discord_ClientResult* self);
void DISCORD_API Discord_ClientResult_SetStatus(Discord_ClientResult* self,
                                                Discord_HttpStatusCode value);
Discord_HttpStatusCode DISCORD_API Discord_ClientResult_Status(Discord_ClientResult* self);
void DISCORD_API Discord_ClientResult_SetResponseBody(Discord_ClientResult* self,
                                                      Discord_String value);
void DISCORD_API Discord_ClientResult_ResponseBody(Discord_ClientResult* self,
                                                   Discord_String* returnValue);
void DISCORD_API Discord_ClientResult_SetSuccessful(Discord_ClientResult* self, bool value);
bool DISCORD_API Discord_ClientResult_Successful(Discord_ClientResult* self);
void DISCORD_API Discord_ClientResult_SetRetryable(Discord_ClientResult* self, bool value);
bool DISCORD_API Discord_ClientResult_Retryable(Discord_ClientResult* self);
void DISCORD_API Discord_ClientResult_SetRetryAfter(Discord_ClientResult* self, float value);
float DISCORD_API Discord_ClientResult_RetryAfter(Discord_ClientResult* self);
struct Discord_AuthorizationCodeChallenge {
    void* opaque;
};

void DISCORD_API Discord_AuthorizationCodeChallenge_Init(Discord_AuthorizationCodeChallenge* self);
void DISCORD_API Discord_AuthorizationCodeChallenge_Drop(Discord_AuthorizationCodeChallenge* self);
void DISCORD_API
Discord_AuthorizationCodeChallenge_Clone(Discord_AuthorizationCodeChallenge* self,
                                         Discord_AuthorizationCodeChallenge const* arg0);
void DISCORD_API
Discord_AuthorizationCodeChallenge_SetMethod(Discord_AuthorizationCodeChallenge* self,
                                             Discord_AuthenticationCodeChallengeMethod value);
Discord_AuthenticationCodeChallengeMethod DISCORD_API
Discord_AuthorizationCodeChallenge_Method(Discord_AuthorizationCodeChallenge* self);
void DISCORD_API
Discord_AuthorizationCodeChallenge_SetChallenge(Discord_AuthorizationCodeChallenge* self,
                                                Discord_String value);
void DISCORD_API
Discord_AuthorizationCodeChallenge_Challenge(Discord_AuthorizationCodeChallenge* self,
                                             Discord_String* returnValue);
struct Discord_AuthorizationCodeVerifier {
    void* opaque;
};

void DISCORD_API Discord_AuthorizationCodeVerifier_Drop(Discord_AuthorizationCodeVerifier* self);
void DISCORD_API
Discord_AuthorizationCodeVerifier_Clone(Discord_AuthorizationCodeVerifier* self,
                                        Discord_AuthorizationCodeVerifier const* arg0);
void DISCORD_API
Discord_AuthorizationCodeVerifier_SetChallenge(Discord_AuthorizationCodeVerifier* self,
                                               Discord_AuthorizationCodeChallenge* value);
void DISCORD_API
Discord_AuthorizationCodeVerifier_Challenge(Discord_AuthorizationCodeVerifier* self,
                                            Discord_AuthorizationCodeChallenge* returnValue);
void DISCORD_API
Discord_AuthorizationCodeVerifier_SetVerifier(Discord_AuthorizationCodeVerifier* self,
                                              Discord_String value);
void DISCORD_API Discord_AuthorizationCodeVerifier_Verifier(Discord_AuthorizationCodeVerifier* self,
                                                            Discord_String* returnValue);
struct Discord_AuthorizationArgs {
    void* opaque;
};

void DISCORD_API Discord_AuthorizationArgs_Init(Discord_AuthorizationArgs* self);
void DISCORD_API Discord_AuthorizationArgs_Drop(Discord_AuthorizationArgs* self);
void DISCORD_API Discord_AuthorizationArgs_Clone(Discord_AuthorizationArgs* self,
                                                 Discord_AuthorizationArgs const* arg0);
void DISCORD_API Discord_AuthorizationArgs_SetClientId(Discord_AuthorizationArgs* self,
                                                       uint64_t value);
uint64_t DISCORD_API Discord_AuthorizationArgs_ClientId(Discord_AuthorizationArgs* self);
void DISCORD_API Discord_AuthorizationArgs_SetScopes(Discord_AuthorizationArgs* self,
                                                     Discord_String value);
void DISCORD_API Discord_AuthorizationArgs_Scopes(Discord_AuthorizationArgs* self,
                                                  Discord_String* returnValue);
void DISCORD_API Discord_AuthorizationArgs_SetState(Discord_AuthorizationArgs* self,
                                                    Discord_String* value);
bool DISCORD_API Discord_AuthorizationArgs_State(Discord_AuthorizationArgs* self,
                                                 Discord_String* returnValue);
void DISCORD_API Discord_AuthorizationArgs_SetNonce(Discord_AuthorizationArgs* self,
                                                    Discord_String* value);
bool DISCORD_API Discord_AuthorizationArgs_Nonce(Discord_AuthorizationArgs* self,
                                                 Discord_String* returnValue);
void DISCORD_API
Discord_AuthorizationArgs_SetCodeChallenge(Discord_AuthorizationArgs* self,
                                           Discord_AuthorizationCodeChallenge* value);
bool DISCORD_API
Discord_AuthorizationArgs_CodeChallenge(Discord_AuthorizationArgs* self,
                                        Discord_AuthorizationCodeChallenge* returnValue);
struct Discord_DeviceAuthorizationArgs {
    void* opaque;
};

void DISCORD_API Discord_DeviceAuthorizationArgs_Init(Discord_DeviceAuthorizationArgs* self);
void DISCORD_API Discord_DeviceAuthorizationArgs_Drop(Discord_DeviceAuthorizationArgs* self);
void DISCORD_API Discord_DeviceAuthorizationArgs_Clone(Discord_DeviceAuthorizationArgs* self,
                                                       Discord_DeviceAuthorizationArgs const* arg0);
void DISCORD_API Discord_DeviceAuthorizationArgs_SetClientId(Discord_DeviceAuthorizationArgs* self,
                                                             uint64_t value);
uint64_t DISCORD_API
Discord_DeviceAuthorizationArgs_ClientId(Discord_DeviceAuthorizationArgs* self);
void DISCORD_API Discord_DeviceAuthorizationArgs_SetScopes(Discord_DeviceAuthorizationArgs* self,
                                                           Discord_String value);
void DISCORD_API Discord_DeviceAuthorizationArgs_Scopes(Discord_DeviceAuthorizationArgs* self,
                                                        Discord_String* returnValue);
struct Discord_VoiceStateHandle {
    void* opaque;
};

void DISCORD_API Discord_VoiceStateHandle_Drop(Discord_VoiceStateHandle* self);
void DISCORD_API Discord_VoiceStateHandle_Clone(Discord_VoiceStateHandle* self,
                                                Discord_VoiceStateHandle const* other);
bool DISCORD_API Discord_VoiceStateHandle_SelfDeaf(Discord_VoiceStateHandle* self);
bool DISCORD_API Discord_VoiceStateHandle_SelfMute(Discord_VoiceStateHandle* self);
struct Discord_VADThresholdSettings {
    void* opaque;
};

void DISCORD_API Discord_VADThresholdSettings_Drop(Discord_VADThresholdSettings* self);
void DISCORD_API Discord_VADThresholdSettings_SetVadThreshold(Discord_VADThresholdSettings* self,
                                                              float value);
float DISCORD_API Discord_VADThresholdSettings_VadThreshold(Discord_VADThresholdSettings* self);
void DISCORD_API Discord_VADThresholdSettings_SetAutomatic(Discord_VADThresholdSettings* self,
                                                           bool value);
bool DISCORD_API Discord_VADThresholdSettings_Automatic(Discord_VADThresholdSettings* self);
struct Discord_Call {
    void* opaque[3];
};

void DISCORD_API Discord_Call_Drop(Discord_Call* self);
void DISCORD_API Discord_Call_Clone(Discord_Call* self, Discord_Call* other);
void DISCORD_API Discord_Call_ErrorToString(Discord_Call_Error type, Discord_String* returnValue);
Discord_AudioModeType DISCORD_API Discord_Call_GetAudioMode(Discord_Call* self);
uint64_t DISCORD_API Discord_Call_GetChannelId(Discord_Call* self);
uint64_t DISCORD_API Discord_Call_GetGuildId(Discord_Call* self);
bool DISCORD_API Discord_Call_GetLocalMute(Discord_Call* self, uint64_t userId);
void DISCORD_API Discord_Call_GetParticipants(Discord_Call* self, Discord_UInt64Span* returnValue);
float DISCORD_API Discord_Call_GetParticipantVolume(Discord_Call* self, uint64_t userId);
bool DISCORD_API Discord_Call_GetPTTActive(Discord_Call* self);
uint32_t DISCORD_API Discord_Call_GetPTTReleaseDelay(Discord_Call* self);
bool DISCORD_API Discord_Call_GetSelfDeaf(Discord_Call* self);
bool DISCORD_API Discord_Call_GetSelfMute(Discord_Call* self);
Discord_Call_Status DISCORD_API Discord_Call_GetStatus(Discord_Call* self);
void DISCORD_API Discord_Call_GetVADThreshold(Discord_Call* self,
                                              Discord_VADThresholdSettings* returnValue);
bool DISCORD_API Discord_Call_GetVoiceStateHandle(Discord_Call* self,
                                                  uint64_t userId,
                                                  Discord_VoiceStateHandle* returnValue);
void DISCORD_API Discord_Call_SetAudioMode(Discord_Call* self, Discord_AudioModeType audioMode);
void DISCORD_API Discord_Call_SetLocalMute(Discord_Call* self, uint64_t userId, bool mute);
void DISCORD_API Discord_Call_SetOnVoiceStateChangedCallback(Discord_Call* self,
                                                             Discord_Call_OnVoiceStateChanged cb,
                                                             Discord_FreeFn cb__userDataFree,
                                                             void* cb__userData);
void DISCORD_API Discord_Call_SetParticipantChangedCallback(Discord_Call* self,
                                                            Discord_Call_OnParticipantChanged cb,
                                                            Discord_FreeFn cb__userDataFree,
                                                            void* cb__userData);
void DISCORD_API Discord_Call_SetParticipantVolume(Discord_Call* self,
                                                   uint64_t userId,
                                                   float volume);
void DISCORD_API Discord_Call_SetPTTActive(Discord_Call* self, bool active);
void DISCORD_API Discord_Call_SetPTTReleaseDelay(Discord_Call* self, uint32_t releaseDelayMs);
void DISCORD_API Discord_Call_SetSelfDeaf(Discord_Call* self, bool deaf);
void DISCORD_API Discord_Call_SetSelfMute(Discord_Call* self, bool mute);
void DISCORD_API
Discord_Call_SetSpeakingStatusChangedCallback(Discord_Call* self,
                                              Discord_Call_OnSpeakingStatusChanged cb,
                                              Discord_FreeFn cb__userDataFree,
                                              void* cb__userData);
void DISCORD_API Discord_Call_SetStatusChangedCallback(Discord_Call* self,
                                                       Discord_Call_OnStatusChanged cb,
                                                       Discord_FreeFn cb__userDataFree,
                                                       void* cb__userData);
void DISCORD_API Discord_Call_SetVADThreshold(Discord_Call* self, bool automatic, float threshold);
void DISCORD_API Discord_Call_StatusToString(Discord_Call_Status type, Discord_String* returnValue);
struct Discord_ChannelHandle {
    void* opaque;
};

void DISCORD_API Discord_ChannelHandle_Drop(Discord_ChannelHandle* self);
void DISCORD_API Discord_ChannelHandle_Clone(Discord_ChannelHandle* self,
                                             Discord_ChannelHandle const* other);
uint64_t DISCORD_API Discord_ChannelHandle_Id(Discord_ChannelHandle* self);
void DISCORD_API Discord_ChannelHandle_Name(Discord_ChannelHandle* self,
                                            Discord_String* returnValue);
void DISCORD_API Discord_ChannelHandle_Recipients(Discord_ChannelHandle* self,
                                                  Discord_UInt64Span* returnValue);
Discord_ChannelType DISCORD_API Discord_ChannelHandle_Type(Discord_ChannelHandle* self);
struct Discord_GuildMinimal {
    void* opaque;
};

void DISCORD_API Discord_GuildMinimal_Drop(Discord_GuildMinimal* self);
void DISCORD_API Discord_GuildMinimal_Clone(Discord_GuildMinimal* self,
                                            Discord_GuildMinimal const* arg0);
void DISCORD_API Discord_GuildMinimal_SetId(Discord_GuildMinimal* self, uint64_t value);
uint64_t DISCORD_API Discord_GuildMinimal_Id(Discord_GuildMinimal* self);
void DISCORD_API Discord_GuildMinimal_SetName(Discord_GuildMinimal* self, Discord_String value);
void DISCORD_API Discord_GuildMinimal_Name(Discord_GuildMinimal* self, Discord_String* returnValue);
struct Discord_GuildChannel {
    void* opaque;
};

void DISCORD_API Discord_GuildChannel_Drop(Discord_GuildChannel* self);
void DISCORD_API Discord_GuildChannel_Clone(Discord_GuildChannel* self,
                                            Discord_GuildChannel const* arg0);
void DISCORD_API Discord_GuildChannel_SetId(Discord_GuildChannel* self, uint64_t value);
uint64_t DISCORD_API Discord_GuildChannel_Id(Discord_GuildChannel* self);
void DISCORD_API Discord_GuildChannel_SetName(Discord_GuildChannel* self, Discord_String value);
void DISCORD_API Discord_GuildChannel_Name(Discord_GuildChannel* self, Discord_String* returnValue);
void DISCORD_API Discord_GuildChannel_SetIsLinkable(Discord_GuildChannel* self, bool value);
bool DISCORD_API Discord_GuildChannel_IsLinkable(Discord_GuildChannel* self);
void DISCORD_API
Discord_GuildChannel_SetIsViewableAndWriteableByAllMembers(Discord_GuildChannel* self, bool value);
bool DISCORD_API
Discord_GuildChannel_IsViewableAndWriteableByAllMembers(Discord_GuildChannel* self);
void DISCORD_API Discord_GuildChannel_SetLinkedLobby(Discord_GuildChannel* self,
                                                     Discord_LinkedLobby* value);
bool DISCORD_API Discord_GuildChannel_LinkedLobby(Discord_GuildChannel* self,
                                                  Discord_LinkedLobby* returnValue);
struct Discord_LinkedLobby {
    void* opaque;
};

void DISCORD_API Discord_LinkedLobby_Init(Discord_LinkedLobby* self);
void DISCORD_API Discord_LinkedLobby_Drop(Discord_LinkedLobby* self);
void DISCORD_API Discord_LinkedLobby_Clone(Discord_LinkedLobby* self,
                                           Discord_LinkedLobby const* arg0);
void DISCORD_API Discord_LinkedLobby_SetApplicationId(Discord_LinkedLobby* self, uint64_t value);
uint64_t DISCORD_API Discord_LinkedLobby_ApplicationId(Discord_LinkedLobby* self);
void DISCORD_API Discord_LinkedLobby_SetLobbyId(Discord_LinkedLobby* self, uint64_t value);
uint64_t DISCORD_API Discord_LinkedLobby_LobbyId(Discord_LinkedLobby* self);
struct Discord_LinkedChannel {
    void* opaque;
};

void DISCORD_API Discord_LinkedChannel_Drop(Discord_LinkedChannel* self);
void DISCORD_API Discord_LinkedChannel_Clone(Discord_LinkedChannel* self,
                                             Discord_LinkedChannel const* arg0);
void DISCORD_API Discord_LinkedChannel_SetId(Discord_LinkedChannel* self, uint64_t value);
uint64_t DISCORD_API Discord_LinkedChannel_Id(Discord_LinkedChannel* self);
void DISCORD_API Discord_LinkedChannel_SetName(Discord_LinkedChannel* self, Discord_String value);
void DISCORD_API Discord_LinkedChannel_Name(Discord_LinkedChannel* self,
                                            Discord_String* returnValue);
void DISCORD_API Discord_LinkedChannel_SetGuildId(Discord_LinkedChannel* self, uint64_t value);
uint64_t DISCORD_API Discord_LinkedChannel_GuildId(Discord_LinkedChannel* self);
struct Discord_RelationshipHandle {
    void* opaque;
};

void DISCORD_API Discord_RelationshipHandle_Drop(Discord_RelationshipHandle* self);
void DISCORD_API Discord_RelationshipHandle_Clone(Discord_RelationshipHandle* self,
                                                  Discord_RelationshipHandle const* other);
Discord_RelationshipType DISCORD_API
Discord_RelationshipHandle_DiscordRelationshipType(Discord_RelationshipHandle* self);
Discord_RelationshipType DISCORD_API
Discord_RelationshipHandle_GameRelationshipType(Discord_RelationshipHandle* self);
uint64_t DISCORD_API Discord_RelationshipHandle_Id(Discord_RelationshipHandle* self);
bool DISCORD_API Discord_RelationshipHandle_User(Discord_RelationshipHandle* self,
                                                 Discord_UserHandle* returnValue);
struct Discord_UserHandle {
    void* opaque;
};

void DISCORD_API Discord_UserHandle_Drop(Discord_UserHandle* self);
void DISCORD_API Discord_UserHandle_Clone(Discord_UserHandle* self, Discord_UserHandle const* arg0);
bool DISCORD_API Discord_UserHandle_Avatar(Discord_UserHandle* self, Discord_String* returnValue);
void DISCORD_API Discord_UserHandle_AvatarTypeToString(Discord_UserHandle_AvatarType type,
                                                       Discord_String* returnValue);
void DISCORD_API Discord_UserHandle_AvatarUrl(Discord_UserHandle* self,
                                              Discord_UserHandle_AvatarType animatedType,
                                              Discord_UserHandle_AvatarType staticType,
                                              Discord_String* returnValue);
void DISCORD_API Discord_UserHandle_DisplayName(Discord_UserHandle* self,
                                                Discord_String* returnValue);
bool DISCORD_API Discord_UserHandle_GameActivity(Discord_UserHandle* self,
                                                 Discord_Activity* returnValue);
bool DISCORD_API Discord_UserHandle_GlobalName(Discord_UserHandle* self,
                                               Discord_String* returnValue);
uint64_t DISCORD_API Discord_UserHandle_Id(Discord_UserHandle* self);
bool DISCORD_API Discord_UserHandle_IsProvisional(Discord_UserHandle* self);
void DISCORD_API Discord_UserHandle_Relationship(Discord_UserHandle* self,
                                                 Discord_RelationshipHandle* returnValue);
Discord_StatusType DISCORD_API Discord_UserHandle_Status(Discord_UserHandle* self);
void DISCORD_API Discord_UserHandle_Username(Discord_UserHandle* self, Discord_String* returnValue);
struct Discord_LobbyMemberHandle {
    void* opaque;
};

void DISCORD_API Discord_LobbyMemberHandle_Drop(Discord_LobbyMemberHandle* self);
void DISCORD_API Discord_LobbyMemberHandle_Clone(Discord_LobbyMemberHandle* self,
                                                 Discord_LobbyMemberHandle const* other);
bool DISCORD_API Discord_LobbyMemberHandle_CanLinkLobby(Discord_LobbyMemberHandle* self);
bool DISCORD_API Discord_LobbyMemberHandle_Connected(Discord_LobbyMemberHandle* self);
uint64_t DISCORD_API Discord_LobbyMemberHandle_Id(Discord_LobbyMemberHandle* self);
void DISCORD_API Discord_LobbyMemberHandle_Metadata(Discord_LobbyMemberHandle* self,
                                                    Discord_Properties* returnValue);
bool DISCORD_API Discord_LobbyMemberHandle_User(Discord_LobbyMemberHandle* self,
                                                Discord_UserHandle* returnValue);
struct Discord_LobbyHandle {
    void* opaque;
};

void DISCORD_API Discord_LobbyHandle_Drop(Discord_LobbyHandle* self);
void DISCORD_API Discord_LobbyHandle_Clone(Discord_LobbyHandle* self,
                                           Discord_LobbyHandle const* other);
bool DISCORD_API Discord_LobbyHandle_GetCallInfoHandle(Discord_LobbyHandle* self,
                                                       Discord_CallInfoHandle* returnValue);
bool DISCORD_API Discord_LobbyHandle_GetLobbyMemberHandle(Discord_LobbyHandle* self,
                                                          uint64_t memberId,
                                                          Discord_LobbyMemberHandle* returnValue);
uint64_t DISCORD_API Discord_LobbyHandle_Id(Discord_LobbyHandle* self);
bool DISCORD_API Discord_LobbyHandle_LinkedChannel(Discord_LobbyHandle* self,
                                                   Discord_LinkedChannel* returnValue);
void DISCORD_API Discord_LobbyHandle_LobbyMemberIds(Discord_LobbyHandle* self,
                                                    Discord_UInt64Span* returnValue);
void DISCORD_API Discord_LobbyHandle_LobbyMembers(Discord_LobbyHandle* self,
                                                  Discord_LobbyMemberHandleSpan* returnValue);
void DISCORD_API Discord_LobbyHandle_Metadata(Discord_LobbyHandle* self,
                                              Discord_Properties* returnValue);
struct Discord_AdditionalContent {
    void* opaque;
};

void DISCORD_API Discord_AdditionalContent_Init(Discord_AdditionalContent* self);
void DISCORD_API Discord_AdditionalContent_Drop(Discord_AdditionalContent* self);
void DISCORD_API Discord_AdditionalContent_Clone(Discord_AdditionalContent* self,
                                                 Discord_AdditionalContent const* arg0);
bool DISCORD_API Discord_AdditionalContent_Equals(Discord_AdditionalContent* self,
                                                  Discord_AdditionalContent const* rhs);
void DISCORD_API Discord_AdditionalContent_TypeToString(Discord_AdditionalContentType type,
                                                        Discord_String* returnValue);
void DISCORD_API Discord_AdditionalContent_SetType(Discord_AdditionalContent* self,
                                                   Discord_AdditionalContentType value);
Discord_AdditionalContentType DISCORD_API
Discord_AdditionalContent_Type(Discord_AdditionalContent* self);
void DISCORD_API Discord_AdditionalContent_SetTitle(Discord_AdditionalContent* self,
                                                    Discord_String* value);
bool DISCORD_API Discord_AdditionalContent_Title(Discord_AdditionalContent* self,
                                                 Discord_String* returnValue);
void DISCORD_API Discord_AdditionalContent_SetCount(Discord_AdditionalContent* self, uint8_t value);
uint8_t DISCORD_API Discord_AdditionalContent_Count(Discord_AdditionalContent* self);
struct Discord_MessageHandle {
    void* opaque;
};

void DISCORD_API Discord_MessageHandle_Drop(Discord_MessageHandle* self);
void DISCORD_API Discord_MessageHandle_Clone(Discord_MessageHandle* self,
                                             Discord_MessageHandle const* other);
bool DISCORD_API Discord_MessageHandle_AdditionalContent(Discord_MessageHandle* self,
                                                         Discord_AdditionalContent* returnValue);
bool DISCORD_API Discord_MessageHandle_Author(Discord_MessageHandle* self,
                                              Discord_UserHandle* returnValue);
uint64_t DISCORD_API Discord_MessageHandle_AuthorId(Discord_MessageHandle* self);
bool DISCORD_API Discord_MessageHandle_Channel(Discord_MessageHandle* self,
                                               Discord_ChannelHandle* returnValue);
uint64_t DISCORD_API Discord_MessageHandle_ChannelId(Discord_MessageHandle* self);
void DISCORD_API Discord_MessageHandle_Content(Discord_MessageHandle* self,
                                               Discord_String* returnValue);
bool DISCORD_API Discord_MessageHandle_DisclosureType(Discord_MessageHandle* self,
                                                      Discord_DisclosureTypes* returnValue);
uint64_t DISCORD_API Discord_MessageHandle_EditedTimestamp(Discord_MessageHandle* self);
uint64_t DISCORD_API Discord_MessageHandle_Id(Discord_MessageHandle* self);
bool DISCORD_API Discord_MessageHandle_Lobby(Discord_MessageHandle* self,
                                             Discord_LobbyHandle* returnValue);
void DISCORD_API Discord_MessageHandle_Metadata(Discord_MessageHandle* self,
                                                Discord_Properties* returnValue);
void DISCORD_API Discord_MessageHandle_RawContent(Discord_MessageHandle* self,
                                                  Discord_String* returnValue);
bool DISCORD_API Discord_MessageHandle_Recipient(Discord_MessageHandle* self,
                                                 Discord_UserHandle* returnValue);
uint64_t DISCORD_API Discord_MessageHandle_RecipientId(Discord_MessageHandle* self);
bool DISCORD_API Discord_MessageHandle_SentFromGame(Discord_MessageHandle* self);
uint64_t DISCORD_API Discord_MessageHandle_SentTimestamp(Discord_MessageHandle* self);
struct Discord_AudioDevice {
    void* opaque;
};

void DISCORD_API Discord_AudioDevice_Drop(Discord_AudioDevice* self);
void DISCORD_API Discord_AudioDevice_Clone(Discord_AudioDevice* self,
                                           Discord_AudioDevice const* arg0);
bool DISCORD_API Discord_AudioDevice_Equals(Discord_AudioDevice* self,
                                            Discord_AudioDevice const* rhs);
void DISCORD_API Discord_AudioDevice_SetId(Discord_AudioDevice* self, Discord_String value);
void DISCORD_API Discord_AudioDevice_Id(Discord_AudioDevice* self, Discord_String* returnValue);
void DISCORD_API Discord_AudioDevice_SetName(Discord_AudioDevice* self, Discord_String value);
void DISCORD_API Discord_AudioDevice_Name(Discord_AudioDevice* self, Discord_String* returnValue);
void DISCORD_API Discord_AudioDevice_SetIsDefault(Discord_AudioDevice* self, bool value);
bool DISCORD_API Discord_AudioDevice_IsDefault(Discord_AudioDevice* self);
struct Discord_Client {
    void* opaque;
};

void DISCORD_API Discord_Client_Init(Discord_Client* self);
void DISCORD_API Discord_Client_InitWithBases(Discord_Client* self,
                                              Discord_String apiBase,
                                              Discord_String webBase);
void DISCORD_API Discord_Client_Drop(Discord_Client* self);
void DISCORD_API Discord_Client_ErrorToString(Discord_Client_Error type,
                                              Discord_String* returnValue);
uint64_t DISCORD_API Discord_Client_GetApplicationId(Discord_Client* self);
void DISCORD_API Discord_Client_GetDefaultAudioDeviceId(Discord_String* returnValue);
void DISCORD_API Discord_Client_GetDefaultCommunicationScopes(Discord_String* returnValue);
void DISCORD_API Discord_Client_GetDefaultPresenceScopes(Discord_String* returnValue);
void DISCORD_API Discord_Client_GetVersionHash(Discord_String* returnValue);
int32_t DISCORD_API Discord_Client_GetVersionMajor();
int32_t DISCORD_API Discord_Client_GetVersionMinor();
int32_t DISCORD_API Discord_Client_GetVersionPatch();
void DISCORD_API Discord_Client_StatusToString(Discord_Client_Status type,
                                               Discord_String* returnValue);
void DISCORD_API Discord_Client_ThreadToString(Discord_Client_Thread type,
                                               Discord_String* returnValue);
void DISCORD_API Discord_Client_EndCall(Discord_Client* self,
                                        uint64_t channelId,
                                        Discord_Client_EndCallCallback callback,
                                        Discord_FreeFn callback__userDataFree,
                                        void* callback__userData);
void DISCORD_API Discord_Client_EndCalls(Discord_Client* self,
                                         Discord_Client_EndCallsCallback callback,
                                         Discord_FreeFn callback__userDataFree,
                                         void* callback__userData);
bool DISCORD_API Discord_Client_GetCall(Discord_Client* self,
                                        uint64_t channelId,
                                        Discord_Call* returnValue);
void DISCORD_API Discord_Client_GetCalls(Discord_Client* self, Discord_CallSpan* returnValue);
void DISCORD_API
Discord_Client_GetCurrentInputDevice(Discord_Client* self,
                                     Discord_Client_GetCurrentInputDeviceCallback cb,
                                     Discord_FreeFn cb__userDataFree,
                                     void* cb__userData);
void DISCORD_API
Discord_Client_GetCurrentOutputDevice(Discord_Client* self,
                                      Discord_Client_GetCurrentOutputDeviceCallback cb,
                                      Discord_FreeFn cb__userDataFree,
                                      void* cb__userData);
void DISCORD_API Discord_Client_GetInputDevices(Discord_Client* self,
                                                Discord_Client_GetInputDevicesCallback cb,
                                                Discord_FreeFn cb__userDataFree,
                                                void* cb__userData);
float DISCORD_API Discord_Client_GetInputVolume(Discord_Client* self);
void DISCORD_API Discord_Client_GetOutputDevices(Discord_Client* self,
                                                 Discord_Client_GetOutputDevicesCallback cb,
                                                 Discord_FreeFn cb__userDataFree,
                                                 void* cb__userData);
float DISCORD_API Discord_Client_GetOutputVolume(Discord_Client* self);
bool DISCORD_API Discord_Client_GetSelfDeafAll(Discord_Client* self);
bool DISCORD_API Discord_Client_GetSelfMuteAll(Discord_Client* self);
void DISCORD_API Discord_Client_SetAutomaticGainControl(Discord_Client* self, bool on);
void DISCORD_API
Discord_Client_SetDeviceChangeCallback(Discord_Client* self,
                                       Discord_Client_DeviceChangeCallback callback,
                                       Discord_FreeFn callback__userDataFree,
                                       void* callback__userData);
void DISCORD_API Discord_Client_SetEchoCancellation(Discord_Client* self, bool on);
void DISCORD_API Discord_Client_SetInputDevice(Discord_Client* self,
                                               Discord_String deviceId,
                                               Discord_Client_SetInputDeviceCallback cb,
                                               Discord_FreeFn cb__userDataFree,
                                               void* cb__userData);
void DISCORD_API Discord_Client_SetInputVolume(Discord_Client* self, float inputVolume);
void DISCORD_API
Discord_Client_SetNoAudioInputCallback(Discord_Client* self,
                                       Discord_Client_NoAudioInputCallback callback,
                                       Discord_FreeFn callback__userDataFree,
                                       void* callback__userData);
void DISCORD_API Discord_Client_SetNoAudioInputThreshold(Discord_Client* self, float dBFSThreshold);
void DISCORD_API Discord_Client_SetNoiseSuppression(Discord_Client* self, bool on);
void DISCORD_API Discord_Client_SetOpusHardwareCoding(Discord_Client* self,
                                                      bool encode,
                                                      bool decode);
void DISCORD_API Discord_Client_SetOutputDevice(Discord_Client* self,
                                                Discord_String deviceId,
                                                Discord_Client_SetOutputDeviceCallback cb,
                                                Discord_FreeFn cb__userDataFree,
                                                void* cb__userData);
void DISCORD_API Discord_Client_SetOutputVolume(Discord_Client* self, float outputVolume);
void DISCORD_API Discord_Client_SetSelfDeafAll(Discord_Client* self, bool deaf);
void DISCORD_API Discord_Client_SetSelfMuteAll(Discord_Client* self, bool mute);
bool DISCORD_API Discord_Client_SetSpeakerMode(Discord_Client* self, bool speakerMode);
void DISCORD_API Discord_Client_SetThreadPriority(Discord_Client* self,
                                                  Discord_Client_Thread thread,
                                                  int32_t priority);
void DISCORD_API
Discord_Client_SetVoiceParticipantChangedCallback(Discord_Client* self,
                                                  Discord_Client_VoiceParticipantChangedCallback cb,
                                                  Discord_FreeFn cb__userDataFree,
                                                  void* cb__userData);
bool DISCORD_API Discord_Client_ShowAudioRoutePicker(Discord_Client* self);
bool DISCORD_API Discord_Client_StartCall(Discord_Client* self,
                                          uint64_t channelId,
                                          Discord_Call* returnValue);
bool DISCORD_API
Discord_Client_StartCallWithAudioCallbacks(Discord_Client* self,
                                           uint64_t lobbyId,
                                           Discord_Client_UserAudioReceivedCallback receivedCb,
                                           Discord_FreeFn receivedCb__userDataFree,
                                           void* receivedCb__userData,
                                           Discord_Client_UserAudioCapturedCallback capturedCb,
                                           Discord_FreeFn capturedCb__userDataFree,
                                           void* capturedCb__userData,
                                           Discord_Call* returnValue);
void DISCORD_API Discord_Client_AbortAuthorize(Discord_Client* self);
void DISCORD_API Discord_Client_AbortGetTokenFromDevice(Discord_Client* self);
void DISCORD_API Discord_Client_Authorize(Discord_Client* self,
                                          Discord_AuthorizationArgs* args,
                                          Discord_Client_AuthorizationCallback callback,
                                          Discord_FreeFn callback__userDataFree,
                                          void* callback__userData);
void DISCORD_API Discord_Client_CloseAuthorizeDeviceScreen(Discord_Client* self);
void DISCORD_API
Discord_Client_CreateAuthorizationCodeVerifier(Discord_Client* self,
                                               Discord_AuthorizationCodeVerifier* returnValue);
void DISCORD_API Discord_Client_FetchCurrentUser(Discord_Client* self,
                                                 Discord_AuthorizationTokenType tokenType,
                                                 Discord_String token,
                                                 Discord_Client_FetchCurrentUserCallback callback,
                                                 Discord_FreeFn callback__userDataFree,
                                                 void* callback__userData);
void DISCORD_API
Discord_Client_GetProvisionalToken(Discord_Client* self,
                                   uint64_t applicationId,
                                   Discord_AuthenticationExternalAuthType externalAuthType,
                                   Discord_String externalAuthToken,
                                   Discord_Client_TokenExchangeCallback callback,
                                   Discord_FreeFn callback__userDataFree,
                                   void* callback__userData);
void DISCORD_API Discord_Client_GetToken(Discord_Client* self,
                                         uint64_t applicationId,
                                         Discord_String code,
                                         Discord_String codeVerifier,
                                         Discord_String redirectUri,
                                         Discord_Client_TokenExchangeCallback callback,
                                         Discord_FreeFn callback__userDataFree,
                                         void* callback__userData);
void DISCORD_API Discord_Client_GetTokenFromDevice(Discord_Client* self,
                                                   Discord_DeviceAuthorizationArgs* args,
                                                   Discord_Client_TokenExchangeCallback callback,
                                                   Discord_FreeFn callback__userDataFree,
                                                   void* callback__userData);
void DISCORD_API Discord_Client_GetTokenFromDeviceProvisionalMerge(
  Discord_Client* self,
  Discord_DeviceAuthorizationArgs* args,
  Discord_AuthenticationExternalAuthType externalAuthType,
  Discord_String externalAuthToken,
  Discord_Client_TokenExchangeCallback callback,
  Discord_FreeFn callback__userDataFree,
  void* callback__userData);
void DISCORD_API
Discord_Client_GetTokenFromProvisionalMerge(Discord_Client* self,
                                            uint64_t applicationId,
                                            Discord_String code,
                                            Discord_String codeVerifier,
                                            Discord_String redirectUri,
                                            Discord_AuthenticationExternalAuthType externalAuthType,
                                            Discord_String externalAuthToken,
                                            Discord_Client_TokenExchangeCallback callback,
                                            Discord_FreeFn callback__userDataFree,
                                            void* callback__userData);
bool DISCORD_API Discord_Client_IsAuthenticated(Discord_Client* self);
void DISCORD_API Discord_Client_OpenAuthorizeDeviceScreen(Discord_Client* self,
                                                          uint64_t clientId,
                                                          Discord_String userCode);
void DISCORD_API Discord_Client_ProvisionalUserMergeCompleted(Discord_Client* self, bool success);
void DISCORD_API Discord_Client_RefreshToken(Discord_Client* self,
                                             uint64_t applicationId,
                                             Discord_String refreshToken,
                                             Discord_Client_TokenExchangeCallback callback,
                                             Discord_FreeFn callback__userDataFree,
                                             void* callback__userData);
void DISCORD_API Discord_Client_SetAuthorizeDeviceScreenClosedCallback(
  Discord_Client* self,
  Discord_Client_AuthorizeDeviceScreenClosedCallback cb,
  Discord_FreeFn cb__userDataFree,
  void* cb__userData);
void DISCORD_API Discord_Client_SetGameWindowPid(Discord_Client* self, int32_t pid);
void DISCORD_API
Discord_Client_SetTokenExpirationCallback(Discord_Client* self,
                                          Discord_Client_TokenExpirationCallback callback,
                                          Discord_FreeFn callback__userDataFree,
                                          void* callback__userData);
void DISCORD_API Discord_Client_UpdateProvisionalAccountDisplayName(
  Discord_Client* self,
  Discord_String name,
  Discord_Client_UpdateProvisionalAccountDisplayNameCallback callback,
  Discord_FreeFn callback__userDataFree,
  void* callback__userData);
void DISCORD_API Discord_Client_UpdateToken(Discord_Client* self,
                                            Discord_AuthorizationTokenType tokenType,
                                            Discord_String token,
                                            Discord_Client_UpdateTokenCallback callback,
                                            Discord_FreeFn callback__userDataFree,
                                            void* callback__userData);
bool DISCORD_API Discord_Client_CanOpenMessageInDiscord(Discord_Client* self, uint64_t messageId);
void DISCORD_API Discord_Client_DeleteUserMessage(Discord_Client* self,
                                                  uint64_t recipientId,
                                                  uint64_t messageId,
                                                  Discord_Client_DeleteUserMessageCallback cb,
                                                  Discord_FreeFn cb__userDataFree,
                                                  void* cb__userData);
void DISCORD_API Discord_Client_EditUserMessage(Discord_Client* self,
                                                uint64_t recipientId,
                                                uint64_t messageId,
                                                Discord_String content,
                                                Discord_Client_EditUserMessageCallback cb,
                                                Discord_FreeFn cb__userDataFree,
                                                void* cb__userData);
bool DISCORD_API Discord_Client_GetChannelHandle(Discord_Client* self,
                                                 uint64_t channelId,
                                                 Discord_ChannelHandle* returnValue);
bool DISCORD_API Discord_Client_GetMessageHandle(Discord_Client* self,
                                                 uint64_t messageId,
                                                 Discord_MessageHandle* returnValue);
void DISCORD_API Discord_Client_OpenMessageInDiscord(
  Discord_Client* self,
  uint64_t messageId,
  Discord_Client_ProvisionalUserMergeRequiredCallback provisionalUserMergeRequiredCallback,
  Discord_FreeFn provisionalUserMergeRequiredCallback__userDataFree,
  void* provisionalUserMergeRequiredCallback__userData,
  Discord_Client_OpenMessageInDiscordCallback callback,
  Discord_FreeFn callback__userDataFree,
  void* callback__userData);
void DISCORD_API Discord_Client_SendLobbyMessage(Discord_Client* self,
                                                 uint64_t lobbyId,
                                                 Discord_String content,
                                                 Discord_Client_SendUserMessageCallback cb,
                                                 Discord_FreeFn cb__userDataFree,
                                                 void* cb__userData);
void DISCORD_API
Discord_Client_SendLobbyMessageWithMetadata(Discord_Client* self,
                                            uint64_t lobbyId,
                                            Discord_String content,
                                            Discord_Properties metadata,
                                            Discord_Client_SendUserMessageCallback cb,
                                            Discord_FreeFn cb__userDataFree,
                                            void* cb__userData);
void DISCORD_API Discord_Client_SendUserMessage(Discord_Client* self,
                                                uint64_t recipientId,
                                                Discord_String content,
                                                Discord_Client_SendUserMessageCallback cb,
                                                Discord_FreeFn cb__userDataFree,
                                                void* cb__userData);
void DISCORD_API
Discord_Client_SendUserMessageWithMetadata(Discord_Client* self,
                                           uint64_t recipientId,
                                           Discord_String content,
                                           Discord_Properties metadata,
                                           Discord_Client_SendUserMessageCallback cb,
                                           Discord_FreeFn cb__userDataFree,
                                           void* cb__userData);
void DISCORD_API Discord_Client_SetMessageCreatedCallback(Discord_Client* self,
                                                          Discord_Client_MessageCreatedCallback cb,
                                                          Discord_FreeFn cb__userDataFree,
                                                          void* cb__userData);
void DISCORD_API Discord_Client_SetMessageDeletedCallback(Discord_Client* self,
                                                          Discord_Client_MessageDeletedCallback cb,
                                                          Discord_FreeFn cb__userDataFree,
                                                          void* cb__userData);
void DISCORD_API Discord_Client_SetMessageUpdatedCallback(Discord_Client* self,
                                                          Discord_Client_MessageUpdatedCallback cb,
                                                          Discord_FreeFn cb__userDataFree,
                                                          void* cb__userData);
void DISCORD_API Discord_Client_SetShowingChat(Discord_Client* self, bool showingChat);
void DISCORD_API Discord_Client_AddLogCallback(Discord_Client* self,
                                               Discord_Client_LogCallback callback,
                                               Discord_FreeFn callback__userDataFree,
                                               void* callback__userData,
                                               Discord_LoggingSeverity minSeverity);
void DISCORD_API Discord_Client_AddVoiceLogCallback(Discord_Client* self,
                                                    Discord_Client_LogCallback callback,
                                                    Discord_FreeFn callback__userDataFree,
                                                    void* callback__userData,
                                                    Discord_LoggingSeverity minSeverity);
void DISCORD_API Discord_Client_Connect(Discord_Client* self);
void DISCORD_API Discord_Client_Disconnect(Discord_Client* self);
Discord_Client_Status DISCORD_API Discord_Client_GetStatus(Discord_Client* self);
void DISCORD_API Discord_Client_SetApplicationId(Discord_Client* self, uint64_t applicationId);
bool DISCORD_API Discord_Client_SetLogDir(Discord_Client* self,
                                          Discord_String path,
                                          Discord_LoggingSeverity minSeverity);
void DISCORD_API Discord_Client_SetStatusChangedCallback(Discord_Client* self,
                                                         Discord_Client_OnStatusChanged cb,
                                                         Discord_FreeFn cb__userDataFree,
                                                         void* cb__userData);
void DISCORD_API Discord_Client_SetVoiceLogDir(Discord_Client* self,
                                               Discord_String path,
                                               Discord_LoggingSeverity minSeverity);
void DISCORD_API Discord_Client_CreateOrJoinLobby(Discord_Client* self,
                                                  Discord_String secret,
                                                  Discord_Client_CreateOrJoinLobbyCallback callback,
                                                  Discord_FreeFn callback__userDataFree,
                                                  void* callback__userData);
void DISCORD_API
Discord_Client_CreateOrJoinLobbyWithMetadata(Discord_Client* self,
                                             Discord_String secret,
                                             Discord_Properties lobbyMetadata,
                                             Discord_Properties memberMetadata,
                                             Discord_Client_CreateOrJoinLobbyCallback callback,
                                             Discord_FreeFn callback__userDataFree,
                                             void* callback__userData);
void DISCORD_API Discord_Client_GetGuildChannels(Discord_Client* self,
                                                 uint64_t guildId,
                                                 Discord_Client_GetGuildChannelsCallback cb,
                                                 Discord_FreeFn cb__userDataFree,
                                                 void* cb__userData);
bool DISCORD_API Discord_Client_GetLobbyHandle(Discord_Client* self,
                                               uint64_t lobbyId,
                                               Discord_LobbyHandle* returnValue);
void DISCORD_API Discord_Client_GetLobbyIds(Discord_Client* self, Discord_UInt64Span* returnValue);
void DISCORD_API Discord_Client_GetUserGuilds(Discord_Client* self,
                                              Discord_Client_GetUserGuildsCallback cb,
                                              Discord_FreeFn cb__userDataFree,
                                              void* cb__userData);
void DISCORD_API Discord_Client_LeaveLobby(Discord_Client* self,
                                           uint64_t lobbyId,
                                           Discord_Client_LeaveLobbyCallback callback,
                                           Discord_FreeFn callback__userDataFree,
                                           void* callback__userData);
void DISCORD_API
Discord_Client_LinkChannelToLobby(Discord_Client* self,
                                  uint64_t lobbyId,
                                  uint64_t channelId,
                                  Discord_Client_LinkOrUnlinkChannelCallback callback,
                                  Discord_FreeFn callback__userDataFree,
                                  void* callback__userData);
void DISCORD_API Discord_Client_SetLobbyCreatedCallback(Discord_Client* self,
                                                        Discord_Client_LobbyCreatedCallback cb,
                                                        Discord_FreeFn cb__userDataFree,
                                                        void* cb__userData);
void DISCORD_API Discord_Client_SetLobbyDeletedCallback(Discord_Client* self,
                                                        Discord_Client_LobbyDeletedCallback cb,
                                                        Discord_FreeFn cb__userDataFree,
                                                        void* cb__userData);
void DISCORD_API
Discord_Client_SetLobbyMemberAddedCallback(Discord_Client* self,
                                           Discord_Client_LobbyMemberAddedCallback cb,
                                           Discord_FreeFn cb__userDataFree,
                                           void* cb__userData);
void DISCORD_API
Discord_Client_SetLobbyMemberRemovedCallback(Discord_Client* self,
                                             Discord_Client_LobbyMemberRemovedCallback cb,
                                             Discord_FreeFn cb__userDataFree,
                                             void* cb__userData);
void DISCORD_API
Discord_Client_SetLobbyMemberUpdatedCallback(Discord_Client* self,
                                             Discord_Client_LobbyMemberUpdatedCallback cb,
                                             Discord_FreeFn cb__userDataFree,
                                             void* cb__userData);
void DISCORD_API Discord_Client_SetLobbyUpdatedCallback(Discord_Client* self,
                                                        Discord_Client_LobbyUpdatedCallback cb,
                                                        Discord_FreeFn cb__userDataFree,
                                                        void* cb__userData);
void DISCORD_API
Discord_Client_UnlinkChannelFromLobby(Discord_Client* self,
                                      uint64_t lobbyId,
                                      Discord_Client_LinkOrUnlinkChannelCallback callback,
                                      Discord_FreeFn callback__userDataFree,
                                      void* callback__userData);
void DISCORD_API Discord_Client_AcceptActivityInvite(Discord_Client* self,
                                                     Discord_ActivityInvite* invite,
                                                     Discord_Client_AcceptActivityInviteCallback cb,
                                                     Discord_FreeFn cb__userDataFree,
                                                     void* cb__userData);
void DISCORD_API Discord_Client_ClearRichPresence(Discord_Client* self);
bool DISCORD_API Discord_Client_RegisterLaunchCommand(Discord_Client* self,
                                                      uint64_t applicationId,
                                                      Discord_String command);
bool DISCORD_API Discord_Client_RegisterLaunchSteamApplication(Discord_Client* self,
                                                               uint64_t applicationId,
                                                               uint32_t steamAppId);
void DISCORD_API Discord_Client_SendActivityInvite(Discord_Client* self,
                                                   uint64_t userId,
                                                   Discord_String content,
                                                   Discord_Client_SendActivityInviteCallback cb,
                                                   Discord_FreeFn cb__userDataFree,
                                                   void* cb__userData);
void DISCORD_API
Discord_Client_SendActivityJoinRequest(Discord_Client* self,
                                       uint64_t userId,
                                       Discord_Client_SendActivityInviteCallback cb,
                                       Discord_FreeFn cb__userDataFree,
                                       void* cb__userData);
void DISCORD_API
Discord_Client_SendActivityJoinRequestReply(Discord_Client* self,
                                            Discord_ActivityInvite* invite,
                                            Discord_Client_SendActivityInviteCallback cb,
                                            Discord_FreeFn cb__userDataFree,
                                            void* cb__userData);
void DISCORD_API
Discord_Client_SetActivityInviteCreatedCallback(Discord_Client* self,
                                                Discord_Client_ActivityInviteCallback cb,
                                                Discord_FreeFn cb__userDataFree,
                                                void* cb__userData);
void DISCORD_API
Discord_Client_SetActivityInviteUpdatedCallback(Discord_Client* self,
                                                Discord_Client_ActivityInviteCallback cb,
                                                Discord_FreeFn cb__userDataFree,
                                                void* cb__userData);
void DISCORD_API Discord_Client_SetActivityJoinCallback(Discord_Client* self,
                                                        Discord_Client_ActivityJoinCallback cb,
                                                        Discord_FreeFn cb__userDataFree,
                                                        void* cb__userData);
void DISCORD_API Discord_Client_SetOnlineStatus(Discord_Client* self,
                                                Discord_StatusType status,
                                                Discord_Client_UpdateStatusCallback callback,
                                                Discord_FreeFn callback__userDataFree,
                                                void* callback__userData);
void DISCORD_API Discord_Client_UpdateRichPresence(Discord_Client* self,
                                                   Discord_Activity* activity,
                                                   Discord_Client_UpdateRichPresenceCallback cb,
                                                   Discord_FreeFn cb__userDataFree,
                                                   void* cb__userData);
void DISCORD_API
Discord_Client_AcceptDiscordFriendRequest(Discord_Client* self,
                                          uint64_t userId,
                                          Discord_Client_UpdateRelationshipCallback cb,
                                          Discord_FreeFn cb__userDataFree,
                                          void* cb__userData);
void DISCORD_API
Discord_Client_AcceptGameFriendRequest(Discord_Client* self,
                                       uint64_t userId,
                                       Discord_Client_UpdateRelationshipCallback cb,
                                       Discord_FreeFn cb__userDataFree,
                                       void* cb__userData);
void DISCORD_API Discord_Client_BlockUser(Discord_Client* self,
                                          uint64_t userId,
                                          Discord_Client_UpdateRelationshipCallback cb,
                                          Discord_FreeFn cb__userDataFree,
                                          void* cb__userData);
void DISCORD_API
Discord_Client_CancelDiscordFriendRequest(Discord_Client* self,
                                          uint64_t userId,
                                          Discord_Client_UpdateRelationshipCallback cb,
                                          Discord_FreeFn cb__userDataFree,
                                          void* cb__userData);
void DISCORD_API
Discord_Client_CancelGameFriendRequest(Discord_Client* self,
                                       uint64_t userId,
                                       Discord_Client_UpdateRelationshipCallback cb,
                                       Discord_FreeFn cb__userDataFree,
                                       void* cb__userData);
void DISCORD_API Discord_Client_GetRelationshipHandle(Discord_Client* self,
                                                      uint64_t userId,
                                                      Discord_RelationshipHandle* returnValue);
void DISCORD_API Discord_Client_GetRelationships(Discord_Client* self,
                                                 Discord_RelationshipHandleSpan* returnValue);
void DISCORD_API
Discord_Client_RejectDiscordFriendRequest(Discord_Client* self,
                                          uint64_t userId,
                                          Discord_Client_UpdateRelationshipCallback cb,
                                          Discord_FreeFn cb__userDataFree,
                                          void* cb__userData);
void DISCORD_API
Discord_Client_RejectGameFriendRequest(Discord_Client* self,
                                       uint64_t userId,
                                       Discord_Client_UpdateRelationshipCallback cb,
                                       Discord_FreeFn cb__userDataFree,
                                       void* cb__userData);
void DISCORD_API
Discord_Client_RemoveDiscordAndGameFriend(Discord_Client* self,
                                          uint64_t userId,
                                          Discord_Client_UpdateRelationshipCallback cb,
                                          Discord_FreeFn cb__userDataFree,
                                          void* cb__userData);
void DISCORD_API Discord_Client_RemoveGameFriend(Discord_Client* self,
                                                 uint64_t userId,
                                                 Discord_Client_UpdateRelationshipCallback cb,
                                                 Discord_FreeFn cb__userDataFree,
                                                 void* cb__userData);
void DISCORD_API Discord_Client_SearchFriendsByUsername(Discord_Client* self,
                                                        Discord_String searchStr,
                                                        Discord_UserHandleSpan* returnValue);
void DISCORD_API
Discord_Client_SendDiscordFriendRequest(Discord_Client* self,
                                        Discord_String username,
                                        Discord_Client_SendFriendRequestCallback cb,
                                        Discord_FreeFn cb__userDataFree,
                                        void* cb__userData);
void DISCORD_API
Discord_Client_SendDiscordFriendRequestById(Discord_Client* self,
                                            uint64_t userId,
                                            Discord_Client_UpdateRelationshipCallback cb,
                                            Discord_FreeFn cb__userDataFree,
                                            void* cb__userData);
void DISCORD_API Discord_Client_SendGameFriendRequest(Discord_Client* self,
                                                      Discord_String username,
                                                      Discord_Client_SendFriendRequestCallback cb,
                                                      Discord_FreeFn cb__userDataFree,
                                                      void* cb__userData);
void DISCORD_API
Discord_Client_SendGameFriendRequestById(Discord_Client* self,
                                         uint64_t userId,
                                         Discord_Client_UpdateRelationshipCallback cb,
                                         Discord_FreeFn cb__userDataFree,
                                         void* cb__userData);
void DISCORD_API
Discord_Client_SetRelationshipCreatedCallback(Discord_Client* self,
                                              Discord_Client_RelationshipCreatedCallback cb,
                                              Discord_FreeFn cb__userDataFree,
                                              void* cb__userData);
void DISCORD_API
Discord_Client_SetRelationshipDeletedCallback(Discord_Client* self,
                                              Discord_Client_RelationshipDeletedCallback cb,
                                              Discord_FreeFn cb__userDataFree,
                                              void* cb__userData);
void DISCORD_API Discord_Client_UnblockUser(Discord_Client* self,
                                            uint64_t userId,
                                            Discord_Client_UpdateRelationshipCallback cb,
                                            Discord_FreeFn cb__userDataFree,
                                            void* cb__userData);
void DISCORD_API Discord_Client_GetCurrentUser(Discord_Client* self,
                                               Discord_UserHandle* returnValue);
void DISCORD_API Discord_Client_GetDiscordClientConnectedUser(
  Discord_Client* self,
  uint64_t applicationId,
  Discord_Client_GetDiscordClientConnectedUserCallback callback,
  Discord_FreeFn callback__userDataFree,
  void* callback__userData);
bool DISCORD_API Discord_Client_GetUser(Discord_Client* self,
                                        uint64_t userId,
                                        Discord_UserHandle* returnValue);
void DISCORD_API Discord_Client_SetUserUpdatedCallback(Discord_Client* self,
                                                       Discord_Client_UserUpdatedCallback cb,
                                                       Discord_FreeFn cb__userDataFree,
                                                       void* cb__userData);
struct Discord_CallInfoHandle {
    void* opaque;
};

void DISCORD_API Discord_CallInfoHandle_Drop(Discord_CallInfoHandle* self);
void DISCORD_API Discord_CallInfoHandle_Clone(Discord_CallInfoHandle* self,
                                              Discord_CallInfoHandle const* other);
uint64_t DISCORD_API Discord_CallInfoHandle_ChannelId(Discord_CallInfoHandle* self);
void DISCORD_API Discord_CallInfoHandle_GetParticipants(Discord_CallInfoHandle* self,
                                                        Discord_UInt64Span* returnValue);
bool DISCORD_API Discord_CallInfoHandle_GetVoiceStateHandle(Discord_CallInfoHandle* self,
                                                            uint64_t userId,
                                                            Discord_VoiceStateHandle* returnValue);
uint64_t DISCORD_API Discord_CallInfoHandle_GuildId(Discord_CallInfoHandle* self);

#ifdef __cplusplus
}
#endif

#endif /* DISCORD_HEADER_CDISCORD_H_ */
