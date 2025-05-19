// Generated with <3 by Discord.Sdk.Derive
#ifndef DISCORD_HEADER_DISCORDPP_H_
#define DISCORD_HEADER_DISCORDPP_H_
#include "cdiscord.h"
#include <atomic>
#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

/// The namespace for the generated Discord SDK bindings.
namespace discordpp {

/// Represents the memory state of a Discord object.
enum class DiscordObjectState {
    /// The object has been freed
    Invalid,
    /// The object is owned by the C++ wrapper and methods can be called on it
    Owned,
};

/// Runs pending callbacks from the Discord SDK.
///
/// You should call this function periodically to process callbacks, e.g. once per frame.
inline void RunCallbacks()
{
    Discord_RunCallbacks();
}

/// \brief ActivityActionTypes represents the type of invite being sent to a user.
///
/// There are essentially two types of invites:
/// 1: A user with an existing activity party can invite another user to join that existing party
/// 2: A user can request to join the existing activity party of another user
///
/// See https://discord.com/developers/docs/rich-presence/overview for more information.
enum class ActivityActionTypes {

    /// \brief Join
    Join = 1,

    /// \brief JoinRequest
    JoinRequest = 5,
};

/// \brief Allows your game to control the privacy of the party the user is in.
enum class ActivityPartyPrivacy {

    /// \brief The party is private (or unknown), which means that the user is in a party but it is
    /// not
    /// joinable without sending a request to join the party.
    ///
    /// This is the default value. You will also receive this value when receiving other users'
    /// activities as the party privacy for other users is not exposed.
    Private = 0,

    /// \brief The party is public, which means that the user is in a party which *could* be
    /// joinable by
    /// either friends or mutual voice participants without sending a request to join the party.
    /// This depends on a user's desired Discord account privacy settings.
    Public = 1,
};

/// \brief Discord RichPresence supports multiple types of activities that a user can be doing.
///
/// For the SDK, the only activity type that is really relevant is `Playing`.
/// The others are provided for completeness.
///
/// See https://discord.com/developers/docs/rich-presence/overview for more information.
enum class ActivityTypes {

    /// \brief Playing
    Playing = 0,

    /// \brief Streaming
    Streaming = 1,

    /// \brief Listening
    Listening = 2,

    /// \brief Watching
    Watching = 3,

    /// \brief CustomStatus
    CustomStatus = 4,

    /// \brief Competing
    Competing = 5,

    /// \brief HangStatus
    HangStatus = 6,
};

/// \brief Represents the type of platforms that an activity invite can be accepted on.
enum class ActivityGamePlatforms {

    /// \brief Desktop
    Desktop = 1,

    /// \brief Xbox
    Xbox = 2,

    /// \brief Samsung
    Samsung = 4,

    /// \brief IOS
    IOS = 8,

    /// \brief Android
    Android = 16,

    /// \brief Embedded
    Embedded = 32,

    /// \brief PS4
    PS4 = 64,

    /// \brief PS5
    PS5 = 128,
};

/// \brief Enum representing various types of errors the SDK returns.
enum class ErrorType {

    /// \brief No error, the operation was successful.
    None = 0,

    /// \brief The user is offline or there was some network issue that prevented an underlying
    /// HTTP call from succeeding.
    NetworkError = 1,

    /// \brief An HTTP call was made to Discord's servers but a non success HTTP status code was
    /// returned.
    /// In some cases this may be retryable, and if so ClientResult::Retryable will be true.
    /// In most cases though the failure is due to a validation or permissions error, and the
    /// request is not retryable. ClientResult::Error and ClientResult::ErrorCode will have more
    /// information.
    HTTPError = 2,

    /// \brief An operation such as sending a friend request or joining a lobby was attempted but
    /// the
    /// Client is not yet ready. Wait for Client::Status to change to Client::Status::Ready before
    /// trying again.
    ///
    /// Also be sure to call Client::Connect to begin the process of connecting to Discord's
    /// servers, otherwise
    /// the Client will never become ready.
    ClientNotReady = 3,

    /// \brief An operation was temporarily disabled for stability reasons.
    Disabled = 4,

    /// \brief The Client has been destroyed and so this operation cannot complete.
    ClientDestroyed = 5,

    /// \brief Used when an SDK method is called but the inputs don't pass local validation. For
    /// example
    /// if one attempts to accept a friend request when there is no pending friend request for that
    /// user,
    /// this ErrorType would be used.
    ///
    /// The specific validation error will be included in the `error` field, and no other
    /// ClientResult fields will be set.
    ValidationError = 6,

    /// \brief The user or developer aborted an operation, such as an authorization flow.
    Aborted = 7,

    /// \brief An authorization function failed, but not necessarily as the result of an HTTP call
    /// that
    /// returned an error.
    AuthorizationFailed = 8,

    /// \brief An RPC call was made to Discord's desktop application, but it returned a non-success
    /// result.
    /// The error and errorCode fields should both be set with more information.
    RPCError = 9,
};

/// \brief Enum that represents the various HTTP status codes that can be returned.
///
/// You can read more about these at: https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
/// For convenience, we have defined a couple of enum values that are non-standard HTTP codes to
/// represent certain types of errors.
enum class HttpStatusCode {

    /// \brief None
    None = 0,

    /// \brief Continue
    Continue = 100,

    /// \brief SwitchingProtocols
    SwitchingProtocols = 101,

    /// \brief Processing
    Processing = 102,

    /// \brief EarlyHints
    EarlyHints = 103,

    /// \brief Ok
    Ok = 200,

    /// \brief Created
    Created = 201,

    /// \brief Accepted
    Accepted = 202,

    /// \brief NonAuthoritativeInfo
    NonAuthoritativeInfo = 203,

    /// \brief NoContent
    NoContent = 204,

    /// \brief ResetContent
    ResetContent = 205,

    /// \brief PartialContent
    PartialContent = 206,

    /// \brief MultiStatus
    MultiStatus = 207,

    /// \brief AlreadyReported
    AlreadyReported = 208,

    /// \brief ImUsed
    ImUsed = 209,

    /// \brief MultipleChoices
    MultipleChoices = 300,

    /// \brief MovedPermanently
    MovedPermanently = 301,

    /// \brief Found
    Found = 302,

    /// \brief SeeOther
    SeeOther = 303,

    /// \brief NotModified
    NotModified = 304,

    /// \brief TemporaryRedirect
    TemporaryRedirect = 307,

    /// \brief PermanentRedirect
    PermanentRedirect = 308,

    /// \brief BadRequest
    BadRequest = 400,

    /// \brief Unauthorized
    Unauthorized = 401,

    /// \brief PaymentRequired
    PaymentRequired = 402,

    /// \brief Forbidden
    Forbidden = 403,

    /// \brief NotFound
    NotFound = 404,

    /// \brief MethodNotAllowed
    MethodNotAllowed = 405,

    /// \brief NotAcceptable
    NotAcceptable = 406,

    /// \brief ProxyAuthRequired
    ProxyAuthRequired = 407,

    /// \brief RequestTimeout
    RequestTimeout = 408,

    /// \brief Conflict
    Conflict = 409,

    /// \brief Gone
    Gone = 410,

    /// \brief LengthRequired
    LengthRequired = 411,

    /// \brief PreconditionFailed
    PreconditionFailed = 412,

    /// \brief PayloadTooLarge
    PayloadTooLarge = 413,

    /// \brief UriTooLong
    UriTooLong = 414,

    /// \brief UnsupportedMediaType
    UnsupportedMediaType = 415,

    /// \brief RangeNotSatisfiable
    RangeNotSatisfiable = 416,

    /// \brief ExpectationFailed
    ExpectationFailed = 417,

    /// \brief MisdirectedRequest
    MisdirectedRequest = 421,

    /// \brief UnprocessableEntity
    UnprocessableEntity = 422,

    /// \brief Locked
    Locked = 423,

    /// \brief FailedDependency
    FailedDependency = 424,

    /// \brief TooEarly
    TooEarly = 425,

    /// \brief UpgradeRequired
    UpgradeRequired = 426,

    /// \brief PreconditionRequired
    PreconditionRequired = 428,

    /// \brief TooManyRequests
    TooManyRequests = 429,

    /// \brief RequestHeaderFieldsTooLarge
    RequestHeaderFieldsTooLarge = 431,

    /// \brief InternalServerError
    InternalServerError = 500,

    /// \brief NotImplemented
    NotImplemented = 501,

    /// \brief BadGateway
    BadGateway = 502,

    /// \brief ServiceUnavailable
    ServiceUnavailable = 503,

    /// \brief GatewayTimeout
    GatewayTimeout = 504,

    /// \brief HttpVersionNotSupported
    HttpVersionNotSupported = 505,

    /// \brief VariantAlsoNegotiates
    VariantAlsoNegotiates = 506,

    /// \brief InsufficientStorage
    InsufficientStorage = 507,

    /// \brief LoopDetected
    LoopDetected = 508,

    /// \brief NotExtended
    NotExtended = 510,

    /// \brief NetworkAuthorizationRequired
    NetworkAuthorizationRequired = 511,
};

/// \brief Represents the crypto method used to generate a code challenge.
///
/// The only method used by the SDK is sha256.
enum class AuthenticationCodeChallengeMethod {

    /// \brief S256
    S256 = 0,
};

/// \brief Represents the type of additional content contained in a message.
enum class AdditionalContentType {

    /// \brief Other
    Other = 0,

    /// \brief Attachment
    Attachment = 1,

    /// \brief Poll
    Poll = 2,

    /// \brief VoiceMessage
    VoiceMessage = 3,

    /// \brief Thread
    Thread = 4,

    /// \brief Embed
    Embed = 5,

    /// \brief Sticker
    Sticker = 6,
};

/// \brief Represents whether a voice call is using push to talk or auto voice detection
enum class AudioModeType {

    /// \brief MODE_UNINIT
    MODE_UNINIT = 0,

    /// \brief MODE_VAD
    MODE_VAD = 1,

    /// \brief MODE_PTT
    MODE_PTT = 2,
};

/// \brief Enum that represents the various channel types on Discord.
///
/// For more information see: https://discord.com/developers/docs/resources/channel
enum class ChannelType {

    /// \brief GuildText
    GuildText = 0,

    /// \brief Dm
    Dm = 1,

    /// \brief GuildVoice
    GuildVoice = 2,

    /// \brief GroupDm
    GroupDm = 3,

    /// \brief GuildCategory
    GuildCategory = 4,

    /// \brief GuildNews
    GuildNews = 5,

    /// \brief GuildStore
    GuildStore = 6,

    /// \brief GuildNewsThread
    GuildNewsThread = 10,

    /// \brief GuildPublicThread
    GuildPublicThread = 11,

    /// \brief GuildPrivateThread
    GuildPrivateThread = 12,

    /// \brief GuildStageVoice
    GuildStageVoice = 13,

    /// \brief GuildDirectory
    GuildDirectory = 14,

    /// \brief GuildForum
    GuildForum = 15,

    /// \brief GuildMedia
    GuildMedia = 16,

    /// \brief Lobby
    Lobby = 17,

    /// \brief EphemeralDm
    EphemeralDm = 18,
};

/// \brief Enum that represents the possible types of relationships that can exist between two users
enum class RelationshipType {

    /// \brief The user has no relationship with the other user.
    None = 0,

    /// \brief The user is friends with the other user.
    Friend = 1,

    /// \brief The current user has blocked the target user, and so certain actions such as sending
    /// messages between these users will not work.
    Blocked = 2,

    /// \brief The current user has received a friend request from the target user, but it is not
    /// yet
    /// accepted.
    PendingIncoming = 3,

    /// \brief The current user has sent a friend request to the target user, but it is not yet
    /// accepted.
    PendingOutgoing = 4,

    /// \brief The Implicit type is documented for visibility, but should be unused in the SDK.
    Implicit = 5,

    /// \brief The Suggestion type is documented for visibility, but should be unused in the SDK.
    Suggestion = 6,
};

/// \brief Enum that specifies the various online statuses for a user.
///
/// Generally a user is online or offline, but in Discord users are able to further customize their
/// status such as turning on "Do not Disturb" mode or "Dnd" to silence notifications.
enum class StatusType {

    /// \brief The user is online and recently active.
    Online = 0,

    /// \brief The user is offline and not connected to Discord.
    Offline = 1,

    /// \brief Blocked
    Blocked = 2,

    /// \brief The user is online, but has not been active for a while and may be away from their
    /// computer.
    Idle = 3,

    /// \brief The user is online, but wishes to suppress notifications for the time being.
    Dnd = 4,

    /// \brief The user is online, but wishes to appear as if they are offline to other users.
    Invisible = 5,

    /// \brief The user is online and is actively streaming content.
    Streaming = 6,

    /// \brief Unknown
    Unknown = 7,
};

/// \brief Enum that represents various informational disclosures that Discord may make to users, so
/// that the game can identity them and customize their rendering as desired.
///
/// See MessageHandle for more details.
enum class DisclosureTypes {

    /// \brief This disclosure type happens the first time a user sends a message in game, and that
    /// message
    /// will be able to be viewed on Discord, so the user knows their content is being copied out of
    /// the game.
    MessageDataVisibleOnDiscord = 3,
};

/// \brief Represents the type of auth token used by the SDK, either the normal tokens produced by
/// the Discord desktop app, or an oauth2 bearer token. Only the latter can be used by the SDK.
enum class AuthorizationTokenType {

    /// \brief User
    User = 0,

    /// \brief Bearer
    Bearer = 1,
};

/// \brief Represents the various identity providers that can be used to authenticate a provisional
/// account user for public clients.
enum class AuthenticationExternalAuthType {

    /// \brief OIDC
    OIDC = 0,

    /// \brief EpicOnlineServicesAccessToken
    EpicOnlineServicesAccessToken = 1,

    /// \brief EpicOnlineServicesIdToken
    EpicOnlineServicesIdToken = 2,

    /// \brief SteamSessionTicket
    SteamSessionTicket = 3,

    /// \brief UnityServicesIdToken
    UnityServicesIdToken = 4,
};

/// \brief Enum that represents the various log levels supported by the SDK.
enum class LoggingSeverity {

    /// \brief Verbose
    Verbose = 1,

    /// \brief Info
    Info = 2,

    /// \brief Warning
    Warning = 3,

    /// \brief Error
    Error = 4,

    /// \brief None
    None = 5,
};
class ActivityInvite;
class ActivityAssets;
class ActivityTimestamps;
class ActivityParty;
class ActivitySecrets;
class Activity;
class ClientResult;
class AuthorizationCodeChallenge;
class AuthorizationCodeVerifier;
class AuthorizationArgs;
class DeviceAuthorizationArgs;
class VoiceStateHandle;
class VADThresholdSettings;
class Call;
class ChannelHandle;
class GuildMinimal;
class GuildChannel;
class LinkedLobby;
class LinkedChannel;
class RelationshipHandle;
class UserHandle;
class LobbyMemberHandle;
class LobbyHandle;
class AdditionalContent;
class MessageHandle;
class AudioDevice;
class Client;
class CallInfoHandle;

/// \brief When one user invites another to join their game on Discord, it will send a message to
/// that user. The SDK will parse those messages for you automatically, and this struct contains all
/// of the relevant invite information which is needed to later accept that invite.
class ActivityInvite {
    /// \cond
    mutable Discord_ActivityInvite instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_ActivityInvite* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit ActivityInvite(Discord_ActivityInvite instance, DiscordObjectState state);
    ~ActivityInvite();
    /// \endcond
    /// Move constructor for ActivityInvite
    ActivityInvite(ActivityInvite&& other) noexcept;
    /// Move assignment operator for ActivityInvite
    ActivityInvite& operator=(ActivityInvite&& other) noexcept;
    /// Uninitialized instance of ActivityInvite
    static const ActivityInvite nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for ActivityInvite
    ActivityInvite(const ActivityInvite& rhs);
    /// Copy assignment operator for ActivityInvite
    ActivityInvite& operator=(const ActivityInvite& rhs);

    explicit ActivityInvite();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The user id of the user who sent the invite.
    uint64_t SenderId() const;
    /// Setter for ActivityInvite::SenderId.
    void SetSenderId(uint64_t SenderId);

    /// \brief The id of the Discord channel in which the invite was sent.
    uint64_t ChannelId() const;
    /// Setter for ActivityInvite::ChannelId.
    void SetChannelId(uint64_t ChannelId);

    /// \brief The id of the Discord message that contains the invite.
    uint64_t MessageId() const;
    /// Setter for ActivityInvite::MessageId.
    void SetMessageId(uint64_t MessageId);

    /// \brief The type of invite that was sent.
    discordpp::ActivityActionTypes Type() const;
    /// Setter for ActivityInvite::Type.
    void SetType(discordpp::ActivityActionTypes Type);

    /// \brief The target application of the invite.
    uint64_t ApplicationId() const;
    /// Setter for ActivityInvite::ApplicationId.
    void SetApplicationId(uint64_t ApplicationId);

    /// \brief The id of the party the invite was sent for.
    std::string PartyId() const;
    /// Setter for ActivityInvite::PartyId.
    void SetPartyId(std::string PartyId);

    /// \brief The session id of the user who sent the invite.
    std::string SessionId() const;
    /// Setter for ActivityInvite::SessionId.
    void SetSessionId(std::string SessionId);

    /// \brief Whether or not this invite is currently joinable. An invite becomes invalid if it was
    /// sent more than 6 hours ago or if the sender is no longer playing the game the invite is for.
    bool IsValid() const;
    /// Setter for ActivityInvite::IsValid.
    void SetIsValid(bool IsValid);
};

/// \brief Struct which controls what your rich presence looks like in
/// the Discord client. If you don't specify any values, the icon
/// and name of your application will be used as defaults.
///
/// Image assets can be either the unique identifier for an image
/// you uploaded to your application via the `Rich Presence` page in
/// the Developer portal, or they can be an external image URL.
///
/// As an example, if I uploaded an asset and name it `goofy-icon`,
/// I could set either image field to the string `goofy-icon`. Alternatively,
/// if my icon was hosted at `http://my-site.com/goofy.jpg`, I could
/// pass that URL into either image field.
///
/// See https://discord.com/developers/docs/rich-presence/overview#adding-custom-art-assets
/// for more information on using custom art assets, as well as for visual
/// examples of what each field does.
class ActivityAssets {
    /// \cond
    mutable Discord_ActivityAssets instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_ActivityAssets* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit ActivityAssets(Discord_ActivityAssets instance, DiscordObjectState state);
    ~ActivityAssets();
    /// \endcond
    /// Move constructor for ActivityAssets
    ActivityAssets(ActivityAssets&& other) noexcept;
    /// Move assignment operator for ActivityAssets
    ActivityAssets& operator=(ActivityAssets&& other) noexcept;
    /// Uninitialized instance of ActivityAssets
    static const ActivityAssets nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for ActivityAssets
    ActivityAssets(const ActivityAssets& arg0);
    /// Copy assignment operator for ActivityAssets
    ActivityAssets& operator=(const ActivityAssets& arg0);

    explicit ActivityAssets();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The primary image identifier or URL, rendered as a large square icon on a user's rich
    /// presence.
    ///
    /// If specified, must be a string between 1 and 256 characters.
    std::optional<std::string> LargeImage() const;
    /// Setter for ActivityAssets::LargeImage.
    void SetLargeImage(std::optional<std::string> LargeImage);

    /// \brief A tooltip string that is shown when the user hovers over the large image.
    ///
    /// If specified, must be a string between 2 and 128 characters.
    std::optional<std::string> LargeText() const;
    /// Setter for ActivityAssets::LargeText.
    void SetLargeText(std::optional<std::string> LargeText);

    /// \brief The secondary image, rendered as a small circle over the `largeImage`.
    ///
    /// If specified, must be a string between 1 and 256 characters.
    std::optional<std::string> SmallImage() const;
    /// Setter for ActivityAssets::SmallImage.
    void SetSmallImage(std::optional<std::string> SmallImage);

    /// \brief A tooltip string that is shown when the user hovers over the small image.
    ///
    /// If specified, must be a string between 2 and 128 characters.
    std::optional<std::string> SmallText() const;
    /// Setter for ActivityAssets::SmallText.
    void SetSmallText(std::optional<std::string> SmallText);
};

/// \brief \see Activity
class ActivityTimestamps {
    /// \cond
    mutable Discord_ActivityTimestamps instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_ActivityTimestamps* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit ActivityTimestamps(Discord_ActivityTimestamps instance, DiscordObjectState state);
    ~ActivityTimestamps();
    /// \endcond
    /// Move constructor for ActivityTimestamps
    ActivityTimestamps(ActivityTimestamps&& other) noexcept;
    /// Move assignment operator for ActivityTimestamps
    ActivityTimestamps& operator=(ActivityTimestamps&& other) noexcept;
    /// Uninitialized instance of ActivityTimestamps
    static const ActivityTimestamps nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for ActivityTimestamps
    ActivityTimestamps(const ActivityTimestamps& arg0);
    /// Copy assignment operator for ActivityTimestamps
    ActivityTimestamps& operator=(const ActivityTimestamps& arg0);

    explicit ActivityTimestamps();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The time the activity started, in milliseconds since Unix epoch.
    ///
    /// The SDK will try to convert seconds to milliseconds if a small-ish value is passed in.
    /// If specified, the Discord client will render a count up timer showing how long the user has
    /// been playing this activity.
    uint64_t Start() const;
    /// Setter for ActivityTimestamps::Start.
    void SetStart(uint64_t Start);

    /// \brief The time the activity will end at, in milliseconds since Unix epoch.
    ///
    /// The SDK will try to convert seconds to milliseconds if a small-ish value is passed in.
    /// If specified, the Discord client will render a countdown timer showing how long until the
    /// activity ends.
    uint64_t End() const;
    /// Setter for ActivityTimestamps::End.
    void SetEnd(uint64_t End);
};

/// \brief \see Activity
class ActivityParty {
    /// \cond
    mutable Discord_ActivityParty instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_ActivityParty* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit ActivityParty(Discord_ActivityParty instance, DiscordObjectState state);
    ~ActivityParty();
    /// \endcond
    /// Move constructor for ActivityParty
    ActivityParty(ActivityParty&& other) noexcept;
    /// Move assignment operator for ActivityParty
    ActivityParty& operator=(ActivityParty&& other) noexcept;
    /// Uninitialized instance of ActivityParty
    static const ActivityParty nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for ActivityParty
    ActivityParty(const ActivityParty& arg0);
    /// Copy assignment operator for ActivityParty
    ActivityParty& operator=(const ActivityParty& arg0);

    explicit ActivityParty();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Specifies the id of the party. "Party" is used colloquially to refer to a group of
    /// players in a shared context. This could be a lobby id, server id, team id, etc.
    ///
    /// All party members should specify a RichPresence update using
    /// the same party id so that the Discord client knows how to group them together. If specified,
    /// must be a string between 2 and 128 characters.
    std::string Id() const;
    /// Setter for ActivityParty::Id.
    void SetId(std::string Id);

    /// \brief The number of people currently in the party, must be at least 1.
    int32_t CurrentSize() const;
    /// Setter for ActivityParty::CurrentSize.
    void SetCurrentSize(int32_t CurrentSize);

    /// \brief The maximum number of people that can be in the party, must be at least 0. When 0,
    /// the UI will not display a maximum.
    int32_t MaxSize() const;
    /// Setter for ActivityParty::MaxSize.
    void SetMaxSize(int32_t MaxSize);

    /// \brief The privacy of the party.
    discordpp::ActivityPartyPrivacy Privacy() const;
    /// Setter for ActivityParty::Privacy.
    void SetPrivacy(discordpp::ActivityPartyPrivacy Privacy);
};

/// \brief \see Activity
class ActivitySecrets {
    /// \cond
    mutable Discord_ActivitySecrets instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_ActivitySecrets* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit ActivitySecrets(Discord_ActivitySecrets instance, DiscordObjectState state);
    ~ActivitySecrets();
    /// \endcond
    /// Move constructor for ActivitySecrets
    ActivitySecrets(ActivitySecrets&& other) noexcept;
    /// Move assignment operator for ActivitySecrets
    ActivitySecrets& operator=(ActivitySecrets&& other) noexcept;
    /// Uninitialized instance of ActivitySecrets
    static const ActivitySecrets nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for ActivitySecrets
    ActivitySecrets(const ActivitySecrets& arg0);
    /// Copy assignment operator for ActivitySecrets
    ActivitySecrets& operator=(const ActivitySecrets& arg0);

    explicit ActivitySecrets();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief A secret string that is shared with users who are accepted into the party so the game
    /// knows how to join the user to the party. For example you might specify an internal game
    /// server ID or a Discord lobby ID or secret. If specified, must be a string between 2 and 128
    /// characters.
    std::string Join() const;
    /// Setter for ActivitySecrets::Join.
    void SetJoin(std::string Join);
};

/// \brief An Activity represents one "thing" a user is doing on Discord and is part of their rich
/// presence.
///
/// Additional information is located on the Discord Developer Portal:
/// - https://discord.com/developers/docs/rich-presence/overview
/// - https://discord.com/developers/docs/developer-tools/game-sdk#activities
/// - https://discord.com/developers/docs/rich-presence/best-practices
///
/// While RichPresence supports multiple types of activities, the only activity type that is really
/// relevant for the SDK is ActivityTypes::Playing. Additionally, the SDK will only expose
/// Activities that are associated with the current game (or application). So for example, a field
/// like `name` below, will always be set to the current game's name from the view of the SDK.
///
/// ## Customization
/// When an activity shows up on Discord, it will look something like this:
/// 1. Playing "game name"
/// 2. Capture the flag | 2 - 1
/// 3. In a group (2 of 3)
///
/// You can control how lines 2 and 3 are rendered in Discord, here's the breakdown:
/// - Line 1, `Playing "game name"` is powered by the name of your game (or application) on Discord.
/// - Line 2, `Capture the flag | 2 - 1` is powered by the `details` field in the activity, and this
/// should generally try to describe what the _player_ is currently doing. You can even include
/// dynamic data such as a match score here.
/// - Line 3, `In a group (2 of 3)` describes the _party_ the player is in. "Party" is used to refer
/// to a group of players in a shared context, such as a lobby, server, team, etc. The first half,
/// `In a group` is powered by the `state` field in the activity, and the second half, `(2 of 3)` is
/// powered by the `party` field in the activity and describes how many people are in the current
/// party and how big the party can get.
///
/// This diagram visually shows the field mapping:
///
///
/// \image html "rich_presence.png" "Rich presence field diagram" width=1070px
///
/// ## Invites / Joinable Activities
/// Other users can be invited to join the current player's activity (or request to join it too),
/// but that does require certain fields to be set:
/// 1. ActivityParty must be set and have a non-empty ActivityParty::Id field. All users in the
/// party should set the same id field too!
/// 2. ActivityParty must specify the size of the group, and there must be room in the group for
/// another person to join.
/// 3. ActivitySecrets::Join must be set to a non-empty string. The join secret is only shared with
/// users who are accepted into the party by an existing member, so it is truly a secret. You can
/// use this so that when the user is accepted your game knows how to join them to the party. For
/// example it could be an internal game ID, or a Discord lobby ID/secret that the client could
/// join.
///
/// There is additional information about game invites here:
/// https://support.discord.com/hc/en-us/articles/115001557452-Game-Invites
///
/// ### Mobile Invites
/// Activity invites are handled via a deep link. To enable users to join your game via an invite in
/// the Discord client, you must do two things:
/// 1. Set your deep link URL in the Discord developer portal. This will be available on the General
/// tab of your application once Social Layer integration is enabled for your app.
/// 2. Set the desired supported platforms when reporting the activity info in your rich presence,
/// e.g.:
///
///
/// \code
///     activity.SetSupportedPlatforms(
///         ActivityGamePlatforms.Desktop |
///         ActivityGamePlatforms.IOS |
///         ActivityGamePlatforms.Android);
/// \endcode
///
///
/// When the user accepts the invite, the Discord client will open:
/// `[your url]/_discord/join?secret=[the join secret you set]`
///
/// ### Example Invites Flow
/// If you are using Discord lobbies for your game, a neat flow would look like this:
/// - When a user starts playing the game, they create a lobby with a random secret string, using
/// Client::CreateOrJoinLobby
/// - That user publishes their RichPresence with the join secret set to the lobby secret, along
/// with party size information
/// - Another use can then see that RichPresence on Discord and join off of it
/// - Once accepted the new user receives the join secret and their client can call
/// CreateOrJoinLobby(joinSecret) to join the lobby
/// - Finally the original user can notice that the lobby membership has changed and so they publish
/// a new RichPresence update containing the updating party size information.
///
/// ### Invites Code Example
///
/// \code
/// // User A
/// // 1. Create a lobby with secret
/// std::string lobbySecret = "foo";
/// client->CreateOrJoinLobby(lobbySecret, [=](discordpp::ClientResult result, uint64_t lobbyId) {
///     // 2. Update rich presence with join secret
///     discordpp::Activity activity{};
///     // set name, state, party size ...
///     discordpp::ActivitySecrets secrets{};
///     secrets.SetJoin(lobbySecret);
///     activity.SetSecrets(secrets);
///     client->UpdateRichPresence(std::move(activity), [](discordpp::ClientResult result) {});
/// });
/// // 3. Some time later, send an invite
/// client->SendActivityInvite(USER_B_ID, "come play with me", [](auto result) {});
///
/// // User B
/// // 4. Monitor for new invites
/// client->SetActivityInviteCallback([client](auto invite) {
///     // 5. When an invite is received, ask the user if they want to accept it.
///     // If they choose to do so then go ahead and invoke AcceptActivityInvite
///     client->AcceptActivityInvite(invite,
///         [client](discordpp::ClientResult result, std::string secret) {
///         if (result.Successful()) {
///             // 5. Join the lobby using the joinSecret
///             client->CreateOrJoinLobby(secret, [](discordpp::ClientResult result, uint64_t
///             lobbyId) {
///                 // Successfully joined lobby!
///             });
///         }
///     });
/// });
/// \endcode
///
///
/// ### Join Requests Code Example
/// Users can also request to join each others parties. This code snippet shows how that flow might
/// look:
///
/// \code
/// // User A
/// // 1. Create a lobby with secret
/// std::string lobbySecret = "foo";
/// client->CreateOrJoinLobby(lobbySecret, [=](discordpp::ClientResult result, uint64_t lobbyId) {
///     // 2. Update rich presence with join secret
///     discordpp::Activity activity{};
///     // set name, state, party size ...
///     discordpp::ActivitySecrets secrets{};
///     secrets.SetJoin(lobbySecret);
///     activity.SetSecrets(secrets);
///     client->UpdateRichPresence(std::move(activity), [](discordpp::ClientResult result) {});
/// });
///
/// // User B
/// // 3. Request to join User A's party
/// client->SendActivityJoinRequest(USER_A_ID, [](auto result) {});
///
/// // User A
/// // Monitor for new invites:
/// client->SetActivityInviteCreatedCallback([client](auto invite) {
///     // 5. The game can now show that User A has received a request to join their party
///     // If User A is ok with that, they can reply back:
///     // Note: invite.type will be ActivityActionTypes::JoinRequest in this example
///     client->SendActivityJoinRequestReply(invite, [](auto result) {});
/// });
///
/// // User B
/// // 6. Same as before, user B can monitor for invites
/// client->SetActivityInviteCreatedCallback([client](auto invite) {
///     // 7. When an invite is received, ask the user if they want to accept it.
///     // If they choose to do so then go ahead and invoke AcceptActivityInvite
///     client->AcceptActivityInvite(invite,
///         [client](discordpp::ClientResult result, std::string secret) {
///         if (result.Successful()) {
///             // 5. Join the lobby using the joinSecret
///             client->CreateOrJoinLobby(secret, [](auto result, uint64_t lobbyId) {
///                 // Successfully joined lobby!
///             });
///         }
///     });
/// });
/// \endcode
///
class Activity {
    /// \cond
    mutable Discord_Activity instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_Activity* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit Activity(Discord_Activity instance, DiscordObjectState state);
    ~Activity();
    /// \endcond
    /// Move constructor for Activity
    Activity(Activity&& other) noexcept;
    /// Move assignment operator for Activity
    Activity& operator=(Activity&& other) noexcept;
    /// Uninitialized instance of Activity
    static const Activity nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for Activity
    Activity(const Activity& arg0);
    /// Copy assignment operator for Activity
    Activity& operator=(const Activity& arg0);

    explicit Activity();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Compares each field of the Activity struct for equality.
    bool Equals(discordpp::Activity other) const;

    /// \brief The name of the game or application that the activity is associated with.
    ///
    /// This field cannot be set by the SDK, and will always be the name of the current game.
    std::string Name() const;
    /// Setter for Activity::Name.
    void SetName(std::string Name);

    /// \brief The type of activity this is.
    ///
    /// This should almost always be set to `Playing`
    discordpp::ActivityTypes Type() const;
    /// Setter for Activity::Type.
    void SetType(discordpp::ActivityTypes Type);

    /// \brief The state _of the party_ for this activity.
    ///
    /// See the docs on the Activity struct for more info.
    /// If specified, must be a string between 2 and 128 characters.
    std::optional<std::string> State() const;
    /// Setter for Activity::State.
    void SetState(std::optional<std::string> State);

    /// \brief The state _of the what the user is doing_ for this activity.
    ///
    /// See the docs on the Activity struct for more info.
    /// If specified, must be a string between 2 and 128 characters.
    std::optional<std::string> Details() const;
    /// Setter for Activity::Details.
    void SetDetails(std::optional<std::string> Details);

    /// \brief The application ID of the game that the activity is associated with.
    ///
    /// This field cannot be set by the SDK, and will always be the application ID of the current
    /// game.
    std::optional<uint64_t> ApplicationId() const;
    /// Setter for Activity::ApplicationId.
    void SetApplicationId(std::optional<uint64_t> ApplicationId);

    /// \brief Images used to customize how the Activity is displayed in the Discord client.
    std::optional<discordpp::ActivityAssets> Assets() const;
    /// Setter for Activity::Assets.
    void SetAssets(std::optional<discordpp::ActivityAssets> Assets);

    /// \brief The timestamps struct can be used to render either:
    /// - a "time remaining" countdown timer (by specifying the `end` value)
    /// - a "time elapsed" countup timer (by specifying the `start` value)
    std::optional<discordpp::ActivityTimestamps> Timestamps() const;
    /// Setter for Activity::Timestamps.
    void SetTimestamps(std::optional<discordpp::ActivityTimestamps> Timestamps);

    /// \brief The party struct is used to indicate the size and members of the people the current
    /// user is playing with.
    std::optional<discordpp::ActivityParty> Party() const;
    /// Setter for Activity::Party.
    void SetParty(std::optional<discordpp::ActivityParty> Party);

    /// \brief The secrets struct is used in combination with the party struct to make an Activity
    /// joinable.
    std::optional<discordpp::ActivitySecrets> Secrets() const;
    /// Setter for Activity::Secrets.
    void SetSecrets(std::optional<discordpp::ActivitySecrets> Secrets);

    /// \brief If an activity is joinable, but only on certain platforms, this field can be used to
    /// indicate which platforms the activity is joinable on. For example if a game is available on
    /// both PC and Mobile, but PC users cannot join Mobile users and vice versa, this field can be
    /// used so that an activity only shows as joinable on Discord if the user is on the appropriate
    /// platform.
    discordpp::ActivityGamePlatforms SupportedPlatforms() const;
    /// Setter for Activity::SupportedPlatforms.
    void SetSupportedPlatforms(discordpp::ActivityGamePlatforms SupportedPlatforms);
};

/// \brief Struct that stores information about the result of an SDK function call.
///
/// Functions can fail for a few reasons including:
/// - The Client is not yet ready and able to perform the action.
/// - The inputs passed to the function are invalid.
/// - The function makes an API call to Discord's backend which returns an error.
/// - The user is offline.
///
/// The ClientResult::Type field is used to to distinguish between the above types of failures
class ClientResult {
    /// \cond
    mutable Discord_ClientResult instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_ClientResult* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit ClientResult(Discord_ClientResult instance, DiscordObjectState state);
    ~ClientResult();
    /// \endcond
    /// Move constructor for ClientResult
    ClientResult(ClientResult&& other) noexcept;
    /// Move assignment operator for ClientResult
    ClientResult& operator=(ClientResult&& other) noexcept;
    /// Uninitialized instance of ClientResult
    static const ClientResult nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for ClientResult
    ClientResult(const ClientResult& arg0);
    /// Copy assignment operator for ClientResult
    ClientResult& operator=(const ClientResult& arg0);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Returns the error message if any of the ClientResult.
    std::string ToString() const;

    /// \brief The type of error that occurred. See ErrorType for more information.
    discordpp::ErrorType Type() const;
    /// Setter for ClientResult::Type.
    void SetType(discordpp::ErrorType Type);

    /// \brief A description of the error that occurred.
    std::string Error() const;
    /// Setter for ClientResult::Error.
    void SetError(std::string Error);

    /// \brief A more detailed error code for this failure. Currently the only use of this is when
    /// an API request is made to Discord's backend and that fails with a specific error, that error
    /// will be included in this field.
    ///
    /// Many of these error codes are documented at:
    /// https://discord.com/developers/docs/topics/opcodes-and-status-codes#json
    ///
    /// This will only be set if the type of error is ErrorType::HTTPError
    int32_t ErrorCode() const;
    /// Setter for ClientResult::ErrorCode.
    void SetErrorCode(int32_t ErrorCode);

    /// \brief The HTTP status code of the API call.
    ///
    /// This will only be set if the type of error is ErrorType::HTTPError
    discordpp::HttpStatusCode Status() const;
    /// Setter for ClientResult::Status.
    void SetStatus(discordpp::HttpStatusCode Status);

    /// \brief The full HTTP response body, which will usually be a JSON string.
    ///
    /// The error format here is a bit more complicated because Discord's API tries to
    /// make it clear which field from the request is causing the error. Documentation on the format
    /// of these errors is here: https://discord.com/developers/docs/reference#error-messages
    ///
    /// This will only be set if the type of error is ErrorType::HTTPError
    std::string ResponseBody() const;
    /// Setter for ClientResult::ResponseBody.
    void SetResponseBody(std::string ResponseBody);

    /// \brief Equivalent to type == ErrorType::None
    bool Successful() const;
    /// Setter for ClientResult::Successful.
    void SetSuccessful(bool Successful);

    /// \brief Indicates if, although an API request failed, it is safe and recommended to retry it.
    bool Retryable() const;
    /// Setter for ClientResult::Retryable.
    void SetRetryable(bool Retryable);

    /// \brief When a user is being rate limited by Discord (and so status == 429), this field
    /// should be set and is the number of seconds to wait before trying again.
    float RetryAfter() const;
    /// Setter for ClientResult::RetryAfter.
    void SetRetryAfter(float RetryAfter);
};

/// \brief Struct that encapsulates the challenge part of the code verification flow.
class AuthorizationCodeChallenge {
    /// \cond
    mutable Discord_AuthorizationCodeChallenge instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_AuthorizationCodeChallenge* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit AuthorizationCodeChallenge(Discord_AuthorizationCodeChallenge instance,
                                        DiscordObjectState state);
    ~AuthorizationCodeChallenge();
    /// \endcond
    /// Move constructor for AuthorizationCodeChallenge
    AuthorizationCodeChallenge(AuthorizationCodeChallenge&& other) noexcept;
    /// Move assignment operator for AuthorizationCodeChallenge
    AuthorizationCodeChallenge& operator=(AuthorizationCodeChallenge&& other) noexcept;
    /// Uninitialized instance of AuthorizationCodeChallenge
    static const AuthorizationCodeChallenge nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for AuthorizationCodeChallenge
    AuthorizationCodeChallenge(const AuthorizationCodeChallenge& arg0);
    /// Copy assignment operator for AuthorizationCodeChallenge
    AuthorizationCodeChallenge& operator=(const AuthorizationCodeChallenge& arg0);

    explicit AuthorizationCodeChallenge();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The method used to generate the challenge. The only method used by the SDK is sha256.
    discordpp::AuthenticationCodeChallengeMethod Method() const;
    /// Setter for AuthorizationCodeChallenge::Method.
    void SetMethod(discordpp::AuthenticationCodeChallengeMethod Method);

    /// \brief The challenge value
    std::string Challenge() const;
    /// Setter for AuthorizationCodeChallenge::Challenge.
    void SetChallenge(std::string Challenge);
};

/// \brief Struct that encapsulates both parts of the code verification flow.
class AuthorizationCodeVerifier {
    /// \cond
    mutable Discord_AuthorizationCodeVerifier instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_AuthorizationCodeVerifier* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit AuthorizationCodeVerifier(Discord_AuthorizationCodeVerifier instance,
                                       DiscordObjectState state);
    ~AuthorizationCodeVerifier();
    /// \endcond
    /// Move constructor for AuthorizationCodeVerifier
    AuthorizationCodeVerifier(AuthorizationCodeVerifier&& other) noexcept;
    /// Move assignment operator for AuthorizationCodeVerifier
    AuthorizationCodeVerifier& operator=(AuthorizationCodeVerifier&& other) noexcept;
    /// Uninitialized instance of AuthorizationCodeVerifier
    static const AuthorizationCodeVerifier nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for AuthorizationCodeVerifier
    AuthorizationCodeVerifier(const AuthorizationCodeVerifier& arg0);
    /// Copy assignment operator for AuthorizationCodeVerifier
    AuthorizationCodeVerifier& operator=(const AuthorizationCodeVerifier& arg0);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The challenge part of the code verification flow.
    discordpp::AuthorizationCodeChallenge Challenge() const;
    /// Setter for AuthorizationCodeVerifier::Challenge.
    void SetChallenge(discordpp::AuthorizationCodeChallenge Challenge);

    /// \brief The verifier part of the code verification flow.
    std::string Verifier() const;
    /// Setter for AuthorizationCodeVerifier::Verifier.
    void SetVerifier(std::string Verifier);
};

/// \brief Arguments to the Client::Authorize function.
class AuthorizationArgs {
    /// \cond
    mutable Discord_AuthorizationArgs instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_AuthorizationArgs* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit AuthorizationArgs(Discord_AuthorizationArgs instance, DiscordObjectState state);
    ~AuthorizationArgs();
    /// \endcond
    /// Move constructor for AuthorizationArgs
    AuthorizationArgs(AuthorizationArgs&& other) noexcept;
    /// Move assignment operator for AuthorizationArgs
    AuthorizationArgs& operator=(AuthorizationArgs&& other) noexcept;
    /// Uninitialized instance of AuthorizationArgs
    static const AuthorizationArgs nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for AuthorizationArgs
    AuthorizationArgs(const AuthorizationArgs& arg0);
    /// Copy assignment operator for AuthorizationArgs
    AuthorizationArgs& operator=(const AuthorizationArgs& arg0);

    explicit AuthorizationArgs();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Optional. The Discord application ID for your game. Defaults to the value set by
    /// Client::SetApplicationId.
    uint64_t ClientId() const;
    /// Setter for AuthorizationArgs::ClientId.
    void SetClientId(uint64_t ClientId);

    /// \brief Scopes is a space separated string of the oauth scopes your game is requesting.
    ///
    /// Most games should just pass in Client::GetDefaultCommunicationScopes or
    /// Client::GetDefaultPresenceScopes which will include these scopes, respectively:
    /// `openid sdk.social_layer` or `openid sdk.social_layer_presence`
    ///
    /// `sdk.social_layer` and `sdk.social_layer_presence` automatically expand to include all the
    /// necessary scopes for the integration.
    ///
    /// You can pass in additional scopes if you need to, but as a general rule you should only
    /// request the scopes you actually need, and the user will need to grant access to those
    /// additional scopes as well.
    std::string Scopes() const;
    /// Setter for AuthorizationArgs::Scopes.
    void SetScopes(std::string Scopes);

    /// \brief See https://discord.com/developers/docs/topics/oauth2#state-and-security for details
    /// on this field.
    ///
    /// We recommend leaving this unset, and the SDK will automatically generate a secure
    /// random value for you.
    std::optional<std::string> State() const;
    /// Setter for AuthorizationArgs::State.
    void SetState(std::optional<std::string> State);

    /// \brief The nonce field is generally only useful for backend integrations using ID tokens.
    ///
    /// For more information, see:
    /// https://openid.net/specs/openid-connect-core-1_0.html#rfc.section.2~nonce:~:text=auth_time%20response%20parameter.)-,nonce,-String%20value%20used
    std::optional<std::string> Nonce() const;
    /// Setter for AuthorizationArgs::Nonce.
    void SetNonce(std::optional<std::string> Nonce);

    /// \brief If using the Client::GetToken flow, you will need to generate a code challenge and
    /// verifier.
    ///
    /// Use Client::CreateAuthorizationCodeVerifier to generate these values and pass the challenge
    /// property here.
    std::optional<discordpp::AuthorizationCodeChallenge> CodeChallenge() const;
    /// Setter for AuthorizationArgs::CodeChallenge.
    void SetCodeChallenge(std::optional<discordpp::AuthorizationCodeChallenge> CodeChallenge);
};

/// \brief Arguments to the Client::GetTokenFromDevice function.
class DeviceAuthorizationArgs {
    /// \cond
    mutable Discord_DeviceAuthorizationArgs instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_DeviceAuthorizationArgs* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit DeviceAuthorizationArgs(Discord_DeviceAuthorizationArgs instance,
                                     DiscordObjectState state);
    ~DeviceAuthorizationArgs();
    /// \endcond
    /// Move constructor for DeviceAuthorizationArgs
    DeviceAuthorizationArgs(DeviceAuthorizationArgs&& other) noexcept;
    /// Move assignment operator for DeviceAuthorizationArgs
    DeviceAuthorizationArgs& operator=(DeviceAuthorizationArgs&& other) noexcept;
    /// Uninitialized instance of DeviceAuthorizationArgs
    static const DeviceAuthorizationArgs nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for DeviceAuthorizationArgs
    DeviceAuthorizationArgs(const DeviceAuthorizationArgs& arg0);
    /// Copy assignment operator for DeviceAuthorizationArgs
    DeviceAuthorizationArgs& operator=(const DeviceAuthorizationArgs& arg0);

    explicit DeviceAuthorizationArgs();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Optional. The Discord application ID for your game. Defaults to the value set by
    /// Client::SetApplicationId.
    uint64_t ClientId() const;
    /// Setter for DeviceAuthorizationArgs::ClientId.
    void SetClientId(uint64_t ClientId);

    /// \brief Scopes is a space separated string of the oauth scopes your game is requesting.
    ///
    /// Most games should just pass in Client::GetDefaultCommunicationScopes or
    /// Client::GetDefaultPresenceScopes which will include these scopes, respectively:
    /// `openid sdk.social_layer` or `openid sdk.social_layer_presence`
    ///
    /// `sdk.social_layer` and `sdk.social_layer_presence` automatically expand to include all the
    /// necessary scopes for the integration.
    ///
    /// You can pass in additional scopes if you need to, but as a general rule you should only
    /// request the scopes you actually need, and the user will need to grant access to those
    /// additional scopes as well.
    std::string Scopes() const;
    /// Setter for DeviceAuthorizationArgs::Scopes.
    void SetScopes(std::string Scopes);
};

/// \brief A VoiceStateHandle represents the state of a single participant in a Discord voice call.
///
/// The main use case for VoiceStateHandle in the SDK is communicate whether a user has muted or
/// defeaned themselves.
///
/// Handle objects in the SDK hold a reference both to the underlying data, and to the SDK instance.
/// Changes to the underlying data will generally be available on existing handles objects without
/// having to re-create them. If the SDK instance is destroyed, but you still have a reference to a
/// handle object, note that it will return the default value for all method calls (ie an empty
/// string for methods that return a string).
class VoiceStateHandle {
    /// \cond
    mutable Discord_VoiceStateHandle instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_VoiceStateHandle* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit VoiceStateHandle(Discord_VoiceStateHandle instance, DiscordObjectState state);
    ~VoiceStateHandle();
    /// \endcond
    /// Move constructor for VoiceStateHandle
    VoiceStateHandle(VoiceStateHandle&& other) noexcept;
    /// Move assignment operator for VoiceStateHandle
    VoiceStateHandle& operator=(VoiceStateHandle&& other) noexcept;
    /// Uninitialized instance of VoiceStateHandle
    static const VoiceStateHandle nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for VoiceStateHandle
    VoiceStateHandle(const VoiceStateHandle& other);
    /// Copy assignment operator for VoiceStateHandle
    VoiceStateHandle& operator=(const VoiceStateHandle& other);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Returns true if the given user has deafened themselves so that no one else in the
    /// call can hear them and so that they do not hear anyone else in the call either.
    bool SelfDeaf() const;

    /// \brief Returns true if the given user has muted themselves so that no one else in the call
    /// can hear them.
    bool SelfMute() const;
};

/// \brief Settings for the void auto detection threshold for picking up activity from a user's mic.
class VADThresholdSettings {
    /// \cond
    mutable Discord_VADThresholdSettings instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_VADThresholdSettings* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit VADThresholdSettings(Discord_VADThresholdSettings instance, DiscordObjectState state);
    ~VADThresholdSettings();
    /// \endcond
    /// Move constructor for VADThresholdSettings
    VADThresholdSettings(VADThresholdSettings&& other) noexcept;
    /// Move assignment operator for VADThresholdSettings
    VADThresholdSettings& operator=(VADThresholdSettings&& other) noexcept;
    /// Uninitialized instance of VADThresholdSettings
    static const VADThresholdSettings nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    VADThresholdSettings(const VADThresholdSettings&) = delete;
    VADThresholdSettings& operator=(const VADThresholdSettings&) = delete;

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The current void auto detection threshold value, has a range of -100, 0 and defaults
    /// to -60.
    float VadThreshold() const;
    /// Setter for VADThresholdSettings::VadThreshold.
    void SetVadThreshold(float VadThreshold);

    /// \brief Whether or not Discord is currently automatically setting and detecting the
    /// appropriate threshold to use.
    bool Automatic() const;
    /// Setter for VADThresholdSettings::Automatic.
    void SetAutomatic(bool Automatic);
};

/// \brief Class that manages an active voice session in a Lobby.
class Call {
    /// \cond
    mutable Discord_Call instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \brief Enum that represents any network errors with the Call.
    enum class Error {

        /// \brief None
        None = 0,

        /// \brief SignalingConnectionFailed
        SignalingConnectionFailed = 1,

        /// \brief SignalingUnexpectedClose
        SignalingUnexpectedClose = 2,

        /// \brief VoiceConnectionFailed
        VoiceConnectionFailed = 3,

        /// \brief JoinTimeout
        JoinTimeout = 4,

        /// \brief Forbidden
        Forbidden = 5,
    };

    /// \brief Enum that respresents the state of the Call's network connection.
    enum class Status {

        /// \brief Disconnected
        Disconnected = 0,

        /// \brief Joining
        Joining = 1,

        /// \brief Connecting
        Connecting = 2,

        /// \brief SignalingConnected
        SignalingConnected = 3,

        /// \brief Connected
        Connected = 4,

        /// \brief Reconnecting
        Reconnecting = 5,

        /// \brief Disconnecting
        Disconnecting = 6,
    };

    /// \brief Callback function for Call::SetOnVoiceStateChangedCallback.
    using OnVoiceStateChanged = std::function<void(uint64_t userId)>;

    /// \brief Callback function for Call::SetParticipantChangedCallback.
    using OnParticipantChanged = std::function<void(uint64_t userId, bool added)>;

    /// \brief Callback function for Call::SetSpeakingStatusChangedCallback.
    using OnSpeakingStatusChanged = std::function<void(uint64_t userId, bool isPlayingSound)>;

    /// \brief Callback function for Call::SetStatusChangedCallback.
    using OnStatusChanged = std::function<
      void(discordpp::Call::Status status, discordpp::Call::Error error, int32_t errorDetail)>;
    /// \cond
    Discord_Call* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit Call(Discord_Call instance, DiscordObjectState state);
    ~Call();
    /// \endcond
    /// Move constructor for Call
    Call(Call&& other) noexcept;
    /// Move assignment operator for Call
    Call& operator=(Call&& other) noexcept;
    /// Uninitialized instance of Call
    static const Call nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for Call
    Call(const Call& other);
    /// Copy assignment operator for Call
    Call& operator=(const Call& other);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Converts the Error enum to a string.
    static std::string ErrorToString(discordpp::Call::Error type);

    /// \brief Returns whether the call is configured to use voice auto detection or push to talk
    /// for the current user.
    discordpp::AudioModeType GetAudioMode();

    /// \brief Returns the ID of the lobby with which this call is associated.
    uint64_t GetChannelId() const;

    /// \brief Returns the ID of the lobby with which this call is associated.
    uint64_t GetGuildId() const;

    /// \brief Returns whether the current user has locally muted the given userId for themselves.
    bool GetLocalMute(uint64_t userId);

    /// \brief Returns a list of all of the user IDs of the participants in the call.
    std::vector<uint64_t> GetParticipants() const;

    /// \brief Returns the locally set playout volume of the given userId.
    ///
    /// Does not affect the volume of this user for any other connected clients. The range of volume
    /// is [0, 200], where 100 indicate default audio volume of the playback device.
    float GetParticipantVolume(uint64_t userId);

    /// \brief Returns whether push to talk is currently active, meaning the user is currently
    /// pressing their configured push to talk key.
    bool GetPTTActive();

    /// \brief Returns the time that PTT is active after the user releases the PTT key and
    /// SetPTTActive(false) is called.
    uint32_t GetPTTReleaseDelay();

    /// \brief Returns whether the current user is deafened.
    bool GetSelfDeaf();

    /// \brief Returns whether the current user's microphone is muted.
    bool GetSelfMute();

    /// \brief Returns the current call status.
    ///
    /// A call is not ready to be used until the status changes to "Connected".
    discordpp::Call::Status GetStatus() const;

    /// \brief Returns the current configuration for void auto detection thresholds. See the
    /// description of the VADThreshold struct for specifics.
    discordpp::VADThresholdSettings GetVADThreshold() const;

    /// \brief Returns a reference to the VoiceStateHandle for the user ID of the given call
    /// participant.
    ///
    /// The VoiceStateHandle allows other users to know if the target user has muted or deafened
    /// themselves.
    std::optional<discordpp::VoiceStateHandle> GetVoiceStateHandle(uint64_t userId) const;

    /// \brief Sets whether to use voice auto detection or push to talk for the current user on this
    /// call.
    ///
    /// If using push to talk you should call SetPTTActive() whenever the user presses their
    /// confused push to talk key.
    void SetAudioMode(discordpp::AudioModeType audioMode);

    /// \brief Locally mutes the given userId, so that the current user cannot hear them anymore.
    ///
    /// Does not affect whether the given user is muted for any other connected clients.
    void SetLocalMute(uint64_t userId, bool mute);

    /// \brief Sets a callback function to generally be invoked whenever a field on a
    /// VoiceStateHandle object for a user would have changed.
    ///
    /// For example when a user mutes themselves, all other connected clients will invoke the
    /// VoiceStateChanged callback, because the "self mute" field will be true now. The callback is
    /// generally not invoked when users join or leave channels.
    void SetOnVoiceStateChangedCallback(discordpp::Call::OnVoiceStateChanged cb);

    /// \brief Sets a callback function to be invoked whenever some joins or leaves a voice call.
    void SetParticipantChangedCallback(discordpp::Call::OnParticipantChanged cb);

    /// \brief Locally changes the playout volume of the given userId.
    ///
    /// Does not affect the volume of this user for any other connected clients. The range of volume
    /// is [0, 200], where 100 indicate default audio volume of the playback device.
    void SetParticipantVolume(uint64_t userId, float volume);

    /// \brief When push to talk is enabled, this should be called whenever the user pushes or
    /// releases their configured push to talk key. This key must be configured in the game, the SDK
    /// does not handle keybinds itself.
    void SetPTTActive(bool active);

    /// \brief If set, extends the time that PTT is active after the user releases the PTT key and
    /// SetPTTActive(false) is called.
    ///
    /// Defaults to no release delay, but we recommend setting to 20ms, which is what Discord uses.
    void SetPTTReleaseDelay(uint32_t releaseDelayMs);

    /// \brief Mutes all audio from the currently active call for the current user.
    /// They will not be able to hear any other participants,
    /// and no other participants will be able to hear the current user either.
    void SetSelfDeaf(bool deaf);

    /// \brief Mutes the current user's microphone so that no other participant in their active
    /// calls can hear them.
    void SetSelfMute(bool mute);

    /// \brief Sets a callback function to be invoked whenever a user starts or stops speaking and
    /// is passed in the userId and whether they are currently speaking.
    ///
    /// It can be invoked in other cases as well, such as if the priority speaker changes or if the
    /// user plays a soundboard sound.
    void SetSpeakingStatusChangedCallback(discordpp::Call::OnSpeakingStatusChanged cb);

    /// \brief Sets a callback function to be invoked when the call status changes, such as when it
    /// fully connects or starts reconnecting.
    void SetStatusChangedCallback(discordpp::Call::OnStatusChanged cb);

    /// \brief Customizes the void auto detection thresholds for picking up activity from a user's
    /// mic.
    /// - When automatic is set to True, Discord will automatically detect the appropriate threshold
    /// to use.
    /// - When automatic is set to False, the given threshold value will be used. Threshold has a
    /// range of -100, 0, and defaults to -60.
    void SetVADThreshold(bool automatic, float threshold);

    /// \brief Converts the Status enum to a string.
    static std::string StatusToString(discordpp::Call::Status type);
};

/// \brief All messages sent on Discord are done so in a Channel. MessageHandle::ChannelId will
/// contain the ID of the channel a message was sent in, and Client::GetChannelHandle will return an
/// instance of this class.
///
/// Handle objects in the SDK hold a reference both to the underlying data, and to the SDK instance.
/// Changes to the underlying data will generally be available on existing handles objects without
/// having to re-create them. If the SDK instance is destroyed, but you still have a reference to a
/// handle object, note that it will return the default value for all method calls (ie an empty
/// string for methods that return a string).
class ChannelHandle {
    /// \cond
    mutable Discord_ChannelHandle instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_ChannelHandle* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit ChannelHandle(Discord_ChannelHandle instance, DiscordObjectState state);
    ~ChannelHandle();
    /// \endcond
    /// Move constructor for ChannelHandle
    ChannelHandle(ChannelHandle&& other) noexcept;
    /// Move assignment operator for ChannelHandle
    ChannelHandle& operator=(ChannelHandle&& other) noexcept;
    /// Uninitialized instance of ChannelHandle
    static const ChannelHandle nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for ChannelHandle
    ChannelHandle(const ChannelHandle& other);
    /// Copy assignment operator for ChannelHandle
    ChannelHandle& operator=(const ChannelHandle& other);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Returns the ID of the channel.
    uint64_t Id() const;

    /// \brief Returns the name of the channel.
    ///
    /// Generally only channels in servers have names, although Discord may generate a display name
    /// for some channels as well.
    std::string Name() const;

    /// \brief For DMs and GroupDMs, returns the user IDs of the members of the channel.
    /// For all other channels returns an empty list.
    std::vector<uint64_t> Recipients() const;

    /// \brief Returns the type of the channel.
    discordpp::ChannelType Type() const;
};

/// \brief Represents a guild (also knowns as a Discord server), that the current user is a member
/// of, that contains channels that can be linked to a lobby.
class GuildMinimal {
    /// \cond
    mutable Discord_GuildMinimal instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_GuildMinimal* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit GuildMinimal(Discord_GuildMinimal instance, DiscordObjectState state);
    ~GuildMinimal();
    /// \endcond
    /// Move constructor for GuildMinimal
    GuildMinimal(GuildMinimal&& other) noexcept;
    /// Move assignment operator for GuildMinimal
    GuildMinimal& operator=(GuildMinimal&& other) noexcept;
    /// Uninitialized instance of GuildMinimal
    static const GuildMinimal nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for GuildMinimal
    GuildMinimal(const GuildMinimal& arg0);
    /// Copy assignment operator for GuildMinimal
    GuildMinimal& operator=(const GuildMinimal& arg0);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The id of the guild.
    uint64_t Id() const;
    /// Setter for GuildMinimal::Id.
    void SetId(uint64_t Id);

    /// \brief The name of the guild.
    std::string Name() const;
    /// Setter for GuildMinimal::Name.
    void SetName(std::string Name);
};

/// \brief Represents a channel in a guild that the current user is a member of and may be able to
/// be linked to a lobby.
class GuildChannel {
    /// \cond
    mutable Discord_GuildChannel instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_GuildChannel* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit GuildChannel(Discord_GuildChannel instance, DiscordObjectState state);
    ~GuildChannel();
    /// \endcond
    /// Move constructor for GuildChannel
    GuildChannel(GuildChannel&& other) noexcept;
    /// Move assignment operator for GuildChannel
    GuildChannel& operator=(GuildChannel&& other) noexcept;
    /// Uninitialized instance of GuildChannel
    static const GuildChannel nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for GuildChannel
    GuildChannel(const GuildChannel& arg0);
    /// Copy assignment operator for GuildChannel
    GuildChannel& operator=(const GuildChannel& arg0);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The id of the channel.
    uint64_t Id() const;
    /// Setter for GuildChannel::Id.
    void SetId(uint64_t Id);

    /// \brief The name of the channel.
    std::string Name() const;
    /// Setter for GuildChannel::Name.
    void SetName(std::string Name);

    /// \brief Whether the current user is able to link this channel to a lobby.
    ///
    /// For this to be true:
    /// - The channel must be a guild text channel
    /// - The channel may not be marked as NSFW
    /// - The channel must not be currently linked to a different lobby
    /// - The user must have the following permissions in the channel in order to link it:
    ///   - Manage Channels
    ///   - View Channel
    ///   - Send Messages
    bool IsLinkable() const;
    /// Setter for GuildChannel::IsLinkable.
    void SetIsLinkable(bool IsLinkable);

    /// \brief Whether the channel is "fully public" which means every member of the guild is able
    /// to view and send messages in that channel.
    ///
    /// Discord allows lobbies to be linked to private channels
    /// in a server, which enables things like a private admin chat.
    ///
    /// However there is no permission synchronization between the game and Discord, so it is the
    /// responsibility of the game to restrict access to the lobby. Every member of the lobby will
    /// be able to view and send messages in the lobby/channel, regardless of whether that user
    /// would have permission to do so in Discord.
    ///
    /// This may be more complexity than a game wants to take on, so instead you can only allow
    /// linking of channels that are fully public in the server so there is no confusion.
    bool IsViewableAndWriteableByAllMembers() const;
    /// Setter for GuildChannel::IsViewableAndWriteableByAllMembers.
    void SetIsViewableAndWriteableByAllMembers(bool IsViewableAndWriteableByAllMembers);

    /// \brief Information about the currently linked lobby, if any.
    /// Currently Discord enforces that a channel can only be linked to a single lobby.
    std::optional<discordpp::LinkedLobby> LinkedLobby() const;
    /// Setter for GuildChannel::LinkedLobby.
    void SetLinkedLobby(std::optional<discordpp::LinkedLobby> LinkedLobby);
};

/// \brief Struct that stores information about the lobby linked to a channel.
class LinkedLobby {
    /// \cond
    mutable Discord_LinkedLobby instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_LinkedLobby* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit LinkedLobby(Discord_LinkedLobby instance, DiscordObjectState state);
    ~LinkedLobby();
    /// \endcond
    /// Move constructor for LinkedLobby
    LinkedLobby(LinkedLobby&& other) noexcept;
    /// Move assignment operator for LinkedLobby
    LinkedLobby& operator=(LinkedLobby&& other) noexcept;
    /// Uninitialized instance of LinkedLobby
    static const LinkedLobby nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for LinkedLobby
    LinkedLobby(const LinkedLobby& arg0);
    /// Copy assignment operator for LinkedLobby
    LinkedLobby& operator=(const LinkedLobby& arg0);

    explicit LinkedLobby();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The ID of the application that owns the lobby.
    uint64_t ApplicationId() const;
    /// Setter for LinkedLobby::ApplicationId.
    void SetApplicationId(uint64_t ApplicationId);

    /// \brief The ID of the lobby.
    uint64_t LobbyId() const;
    /// Setter for LinkedLobby::LobbyId.
    void SetLobbyId(uint64_t LobbyId);
};

/// \brief Struct that stores information about the channel that a lobby is linked to.
class LinkedChannel {
    /// \cond
    mutable Discord_LinkedChannel instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_LinkedChannel* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit LinkedChannel(Discord_LinkedChannel instance, DiscordObjectState state);
    ~LinkedChannel();
    /// \endcond
    /// Move constructor for LinkedChannel
    LinkedChannel(LinkedChannel&& other) noexcept;
    /// Move assignment operator for LinkedChannel
    LinkedChannel& operator=(LinkedChannel&& other) noexcept;
    /// Uninitialized instance of LinkedChannel
    static const LinkedChannel nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for LinkedChannel
    LinkedChannel(const LinkedChannel& arg0);
    /// Copy assignment operator for LinkedChannel
    LinkedChannel& operator=(const LinkedChannel& arg0);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief The id of the linked channel.
    uint64_t Id() const;
    /// Setter for LinkedChannel::Id.
    void SetId(uint64_t Id);

    /// \brief The name of the linked channel.
    std::string Name() const;
    /// Setter for LinkedChannel::Name.
    void SetName(std::string Name);

    /// \brief The id of the guild (aka server) that owns the linked channel.
    uint64_t GuildId() const;
    /// Setter for LinkedChannel::GuildId.
    void SetGuildId(uint64_t GuildId);
};

/// \brief A RelationshipHandle represents the relationship between the current user and a target
/// user on Discord. Relationships include friends, blocked users, and friend invites.
///
/// The SDK supports two types of relationships:
/// - Discord: These are relationships that persist across games and on the Discord client.
/// Both users will be able to see whether each other is online regardless of whether they are in
/// the same game or not.
/// - Game: These are per-game relationships and do not carry over to other games. The two users
/// will only be able to see if the other is online if they are playing a game in which they are
/// friends.
///
/// If someone is a game friend they can later choose to "upgrade" to a full Discord friend. In this
/// case, the user has two relationships at the same time, which is why there are two different type
/// fields on RelationshipHandle. In this example, their RelationshipHandle::DiscordRelationshipType
/// would be set to RelationshipType::PendingIncoming or RelationshipType::PendingOutgoing (based on
/// whether they are receiving or sending the invite respectively), and their
/// RelationshipHandle::GameRelationshipType would remain as RelationshipType::Friend.
///
/// When a user blocks another user, it is always stored on the
/// RelationshipHandle::DiscordRelationshipType field, and will persist across games. It is not
/// possible to block a user in only one game.
///
/// See the @ref friends documentation for more information.
///
/// Note: While the SDK allows you to manage a user's relationships, you should never take an action
/// without their explicit consent. You should not automatically send or accept friend requests.
/// Only invoke APIs to manage relationships in response to a user action such as clicking a "Send
/// Friend Request" button.
///
/// Handle objects in the SDK hold a reference both to the underlying data, and to the SDK instance.
/// Changes to the underlying data will generally be available on existing handles objects without
/// having to re-create them. If the SDK instance is destroyed, but you still have a reference to a
/// handle object, note that it will return the default value for all method calls (ie an empty
/// string for methods that return a string).
class RelationshipHandle {
    /// \cond
    mutable Discord_RelationshipHandle instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_RelationshipHandle* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit RelationshipHandle(Discord_RelationshipHandle instance, DiscordObjectState state);
    ~RelationshipHandle();
    /// \endcond
    /// Move constructor for RelationshipHandle
    RelationshipHandle(RelationshipHandle&& other) noexcept;
    /// Move assignment operator for RelationshipHandle
    RelationshipHandle& operator=(RelationshipHandle&& other) noexcept;
    /// Uninitialized instance of RelationshipHandle
    static const RelationshipHandle nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for RelationshipHandle
    RelationshipHandle(const RelationshipHandle& other);
    /// Copy assignment operator for RelationshipHandle
    RelationshipHandle& operator=(const RelationshipHandle& other);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Returns the type of the Discord relationship.
    discordpp::RelationshipType DiscordRelationshipType() const;

    /// \brief Returns the type of the Game relationship.
    discordpp::RelationshipType GameRelationshipType() const;

    /// \brief Returns the ID of the target user in this relationship.
    uint64_t Id() const;

    /// \brief Returns a handle to the target user in this relationship, if one is available.
    /// This would be the user with the same ID as the one returned by the Id() method.
    std::optional<discordpp::UserHandle> User() const;
};

/// \brief A UserHandle represents a single user on Discord that the SDK knows about and contains
/// basic account information for them such as id, name, and avatar, as well as their "status"
/// information which includes both whether they are online/offline/etc as well as whether they are
/// playing this game.
///
/// Handle objects in the SDK hold a reference both to the underlying data, and to the SDK instance.
/// Changes to the underlying data will generally be available on existing handles objects without
/// having to re-create them. If the SDK instance is destroyed, but you still have a reference to a
/// handle object, note that it will return the default value for all method calls (ie an empty
/// string for methods that return a string).
class UserHandle {
    /// \cond
    mutable Discord_UserHandle instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \brief The desired type of avatar url to generate for a User.
    enum class AvatarType {

        /// \brief Gif
        Gif = 0,

        /// \brief Webp
        Webp = 1,

        /// \brief Png
        Png = 2,

        /// \brief Jpeg
        Jpeg = 3,
    };
    /// \cond
    Discord_UserHandle* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit UserHandle(Discord_UserHandle instance, DiscordObjectState state);
    ~UserHandle();
    /// \endcond
    /// Move constructor for UserHandle
    UserHandle(UserHandle&& other) noexcept;
    /// Move assignment operator for UserHandle
    UserHandle& operator=(UserHandle&& other) noexcept;
    /// Uninitialized instance of UserHandle
    static const UserHandle nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for UserHandle
    UserHandle(const UserHandle& arg0);
    /// Copy assignment operator for UserHandle
    UserHandle& operator=(const UserHandle& arg0);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Returns the hash of the user's Discord profile avatar, if one is set.
    std::optional<std::string> Avatar() const;

    /// \brief Converts the AvatarType enum to a string.
    static std::string AvatarTypeToString(discordpp::UserHandle::AvatarType type);

    /// \brief Returns a CDN url to the user's Discord profile avatar.
    ///
    /// If the user does not have an avatar set, a url to one of Discord's default avatars is
    /// returned instead.
    std::string AvatarUrl(discordpp::UserHandle::AvatarType animatedType,
                          discordpp::UserHandle::AvatarType staticType) const;

    /// \brief Returns the user's preferred name, if one is set, otherwise returns their unique
    /// username.
    std::string DisplayName() const;

    /// \brief Returns the user's rich presence activity that is associated with the current game,
    /// if one is set.
    ///
    /// On Discord, users can have multiple rich presence activities at once, but the SDK will only
    /// expose the activity that is associated with your game. You can use this to know about the
    /// party the user is in, if any, and what the user is doing in the game.
    ///
    /// For more information see the Activity class and check out
    /// https://discord.com/developers/docs/rich-presence/overview
    std::optional<discordpp::Activity> GameActivity() const;

    /// \brief Returns the preferred display name of this user, if one is set.
    ///
    /// Discord's public API refers to this as a "global name" instead of "display name".
    ///
    /// Discord users can set their preferred name to almost any string.
    ///
    /// For more information about usernames on Discord, see:
    /// https://discord.com/developers/docs/resources/user
    std::optional<std::string> GlobalName() const;

    /// \brief Returns the ID of this user.
    ///
    /// If this returns 0 then the UserHandle is no longer valid.
    uint64_t Id() const;

    /// \brief Returns true if this user is a provisional account.
    bool IsProvisional() const;

    /// \brief Returns a reference to the RelationshipHandle between the currently authenticated
    /// user and this user, if any.
    discordpp::RelationshipHandle Relationship() const;

    /// \brief Returns the user's online/offline/idle status.
    discordpp::StatusType Status() const;

    /// \brief Returns the globally unique username of this user.
    ///
    /// For provisional accounts this is an auto-generated string.
    ///
    /// For more information about usernames on Discord, see:
    /// https://discord.com/developers/docs/resources/user
    std::string Username() const;
};

/// \brief A LobbyMemberHandle represents the state of a single user in a Lobby.
///
/// The SDK separates lobby membership into two concepts:
/// 1. Has the user been added to the lobby by the game developer?
/// If the LobbyMemberHandle exists for a user/lobby pair, then the user has been added to the
/// lobby.
/// 2. Does the user have an active game session that is connected to the lobby and will receive any
/// lobby messages? It is possible for a game developer to add a user to a lobby while they are
/// offline. Also users may temporarily disconnect and rejoin later. So the `Connected` boolean
/// tells you whether the user is actively connected to the lobby.
///
/// Handle objects in the SDK hold a reference both to the underlying data, and to the SDK instance.
/// Changes to the underlying data will generally be available on existing handles objects without
/// having to re-create them. If the SDK instance is destroyed, but you still have a reference to a
/// handle object, note that it will return the default value for all method calls (ie an empty
/// string for methods that return a string).
class LobbyMemberHandle {
    /// \cond
    mutable Discord_LobbyMemberHandle instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_LobbyMemberHandle* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit LobbyMemberHandle(Discord_LobbyMemberHandle instance, DiscordObjectState state);
    ~LobbyMemberHandle();
    /// \endcond
    /// Move constructor for LobbyMemberHandle
    LobbyMemberHandle(LobbyMemberHandle&& other) noexcept;
    /// Move assignment operator for LobbyMemberHandle
    LobbyMemberHandle& operator=(LobbyMemberHandle&& other) noexcept;
    /// Uninitialized instance of LobbyMemberHandle
    static const LobbyMemberHandle nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for LobbyMemberHandle
    LobbyMemberHandle(const LobbyMemberHandle& other);
    /// Copy assignment operator for LobbyMemberHandle
    LobbyMemberHandle& operator=(const LobbyMemberHandle& other);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Returns true if the user is allowed to link a channel to this lobby.
    ///
    /// Under the hood this checks if the LobbyMemberFlags::CanLinkLobby flag is set.
    /// This flag can only be set via the server API, add_lobby_member
    /// The use case for this is for games that want to restrict a lobby so that only the
    /// clan/guild/group leader is allowed to manage the linked channel for the lobby.
    bool CanLinkLobby() const;

    /// \brief Returns true if the user is currently connected to the lobby.
    bool Connected() const;

    /// \brief The user id of the lobby member.
    uint64_t Id() const;

    /// \brief Metadata is a set of string key/value pairs that the game developer can use.
    ///
    /// A common use case may be to store the game's internal user ID for this user so that every
    /// member of a lobby knows the discord user ID and the game's internal user ID mapping for each
    /// user.
    std::unordered_map<std::string, std::string> Metadata() const;

    /// \brief The UserHandle of the lobby member.
    std::optional<discordpp::UserHandle> User() const;
};

/// \brief A LobbyHandle represents a single lobby in the SDK. A lobby can be thought of as
/// just an arbitrary, developer-controlled group of users that can communicate with each other.
///
/// ## Managing Lobbies
/// Lobbies can be managed through a set of @ref server_apis that are documented elsewhere, which
/// allow you to create lobbies, add and remove users from lobbies, and delete them.
///
/// There is also an API to create lobbies without any server side component using the
/// Client::CreateOrJoinLobby function, which accepts a game-generated secret and will join the user
/// to the lobby associated with that secret, creating it if necessary.
///
/// NOTE: When using this API the secret will auto-expire in 30 days, at which point the existing
/// lobby can no longer be joined, but will still exist. We recommend using this for short term
/// lobbies and not permanent lobbies. Use the Server API for more permanent lobbies.
///
/// Members of a lobby are not automatically removed when they close the game or temporarily
/// disconnect. When the SDK connects, it will attempt to re-connect to any lobbies it is currently
/// a member of.
///
/// # Lobby Auto-Deletion
/// Lobbies are generally ephemeral and will be auto-deleted if they have been idle (meaning no
/// users are actively connected to them) for some amount of time. The default is to auto delete
/// after 5 minutes, but this can be customized when creating the lobby. As long as one user is
/// connected to the lobby though it will not be auto-deleted (meaning they have the SDK running and
/// status is set to Ready). Additionally, lobbies that are linked to a channel on Discord will not
/// be auto deleted.
///
/// You can also use the @ref server_apis to customize this timeout, it can be raised to as high as
/// 7 days, meaning the lobby only gets deleted if no one connects to it for an entire week. This
/// should give a good amount of permanence to lobbies when needed, but there may be rare cases
/// where a lobby does need to be "rebuilt" if everyone is offline for an extended period.
///
/// # Membership Limits
/// Lobbies may have a maximum of 1,000 members, and each user may be in a maximum of 100 lobbies
/// per game.
///
/// ## Audio
/// Lobbies support voice calls. Although a lobby is allowed to have 1,000 members, you should not
/// try to start voice calls in lobbies that large. We strongly recommend sticking to around 25
/// members or fewer for voice calls.
///
/// See Client::StartCall for more information on how to start a voice call in a lobby.
///
/// ## Channel Linking
/// Lobbies can be linked to a channel on Discord, which allows messages sent in one place to show
/// up in the other. Any lobby can be linked to a channel, but only lobby members with the
/// LobbyMemberFlags::CanLinkLobby flag are allowed to a link a lobby. This flag must be set using
/// the server APIs, which allows you to ensure that only clan/guild/group leaders can link lobbies
/// to Discord channels.
///
/// To setup a link you'll need to use methods in the Client class to fetch the servers (aka guilds)
/// and channels a user is a member of and setup the link. The Client::GetUserGuilds and
/// Client::GetGuildChannels methods are used to fetch the servers and channels respectively. You
/// can use these to show a UI for the user to pick which server and channel they want to link to.
///
/// Not all channels are linkable. To be linked:
/// - The channel must be a guild text channel
/// - The channel may not be marked as NSFW
/// - The channel must not be currently linked to a different lobby
/// - The user must have the following permissions in the channel in order to link it:
///   - Manage Channels
///   - View Channel
///   - Send Messages
///
/// ### Linking Private Channels
/// Discord is allowing all channels the user has access to in a server to be linked in game, even
/// if that channel is private to other members of the server. This means that a user could choose
/// to link a private "admins chat" channel (assuming they are an admin) in game if they wanted.
///
/// It's not really possible for the game to know which users should have access to that channel or
/// not though. So in this implementation, every member of a lobby will be able to view all messages
/// sent in the linked channel and reply to them. If you are going to allow private channels to be
/// linked in game, you must make sure that users are aware that their private channel will be
/// viewable by everyone in the lobby!
///
/// To help you identify which channels are public or private, we have added a
/// isViewableAndWriteableByAllMembers boolean which is described more in GuildChannel. You can use
/// that to just not allow private channels to be linked, or to know when to show a clear warning,
/// it's up to you!
///
/// ## Misc
/// Handle objects in the SDK hold a reference both to the underlying data, and to the SDK instance.
/// Changes to the underlying data will generally be available on existing handles objects without
/// having to re-create them. If the SDK instance is destroyed, but you still have a reference to a
/// handle object, note that it will return the default value for all method calls (ie an empty
/// string for methods that return a string).
class LobbyHandle {
    /// \cond
    mutable Discord_LobbyHandle instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_LobbyHandle* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit LobbyHandle(Discord_LobbyHandle instance, DiscordObjectState state);
    ~LobbyHandle();
    /// \endcond
    /// Move constructor for LobbyHandle
    LobbyHandle(LobbyHandle&& other) noexcept;
    /// Move assignment operator for LobbyHandle
    LobbyHandle& operator=(LobbyHandle&& other) noexcept;
    /// Uninitialized instance of LobbyHandle
    static const LobbyHandle nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for LobbyHandle
    LobbyHandle(const LobbyHandle& other);
    /// Copy assignment operator for LobbyHandle
    LobbyHandle& operator=(const LobbyHandle& other);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Returns a reference to the CallInfoHandle if there is an active voice call in this
    /// lobby.
    ///
    /// This can allow you to display which lobby members are in voice, even if the current user has
    /// not yet joined the voice call.
    std::optional<discordpp::CallInfoHandle> GetCallInfoHandle() const;

    /// \brief Returns a reference to the LobbyMemberHandle for the given user ID, if they are a
    /// member of this lobby.
    std::optional<discordpp::LobbyMemberHandle> GetLobbyMemberHandle(uint64_t memberId) const;

    /// \brief Returns the id of the lobby.
    uint64_t Id() const;

    /// \brief Returns information about the channel linked to this lobby, if any.
    std::optional<discordpp::LinkedChannel> LinkedChannel() const;

    /// \brief Returns a list of the user IDs that are members of this lobby.
    std::vector<uint64_t> LobbyMemberIds() const;

    /// \brief Returns a list of the LobbyMemberHandle objects for each member of this lobby.
    std::vector<discordpp::LobbyMemberHandle> LobbyMembers() const;

    /// \brief Returns any developer supplied metadata for this lobby.
    ///
    /// Metadata is simple string key/value pairs and is a way to associate internal game
    /// information with the lobby so each lobby member can have easy access to.
    std::unordered_map<std::string, std::string> Metadata() const;
};

/// \brief Contains information about non-text content in a message that likely cannot be rendered
/// in game such as images, videos, embeds, polls, and more.
class AdditionalContent {
    /// \cond
    mutable Discord_AdditionalContent instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_AdditionalContent* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit AdditionalContent(Discord_AdditionalContent instance, DiscordObjectState state);
    ~AdditionalContent();
    /// \endcond
    /// Move constructor for AdditionalContent
    AdditionalContent(AdditionalContent&& other) noexcept;
    /// Move assignment operator for AdditionalContent
    AdditionalContent& operator=(AdditionalContent&& other) noexcept;
    /// Uninitialized instance of AdditionalContent
    static const AdditionalContent nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for AdditionalContent
    AdditionalContent(const AdditionalContent& arg0);
    /// Copy assignment operator for AdditionalContent
    AdditionalContent& operator=(const AdditionalContent& arg0);

    explicit AdditionalContent();

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Compares each field of the AdditionalContent struct for equality.
    bool Equals(discordpp::AdditionalContent rhs) const;

    /// \brief Converts the AdditionalContentType enum to a string.
    static std::string TypeToString(discordpp::AdditionalContentType type);

    /// \brief Represents the type of additional content in the message.
    discordpp::AdditionalContentType Type() const;
    /// Setter for AdditionalContent::Type.
    void SetType(discordpp::AdditionalContentType Type);

    /// \brief When the additional content is a poll or thread, this field will contain the name of
    /// the poll or thread.
    std::optional<std::string> Title() const;
    /// Setter for AdditionalContent::Title.
    void SetTitle(std::optional<std::string> Title);

    /// \brief Represents the number of pieces of additional content so you could for example
    /// renders "2 additional images".
    uint8_t Count() const;
    /// Setter for AdditionalContent::Count.
    void SetCount(uint8_t Count);
};

/// \brief A MessageHandle represents a single message received by the SDK.
///
/// # Chat types
/// The SDK supports two types of chat:
/// 1. 1 on 1 chat between two users
/// 2. Chat within a lobby
///
/// You can determine the context a message was sent in with the MessageHandle::Channel and
/// ChannelHandle::Type methods. The SDK should only be receiving messages in the following channel
/// types:
/// - DM
/// - Ephemeral DM
/// - Lobby
///
/// # Syncing with Discord
/// In some situations messages sent from the SDK will also show up in Discord.
/// In general this will happen for:
/// - 1 on 1 chat when at least one of the users is a full Discord user
/// - Lobby chat when the lobby is linked to a Discord channel
///
/// Additionally the message must have been sent by a user who is not banned on the Discord side.
///
/// # Legal disclosures
/// As a convenience for game developers, the first time a user sends a message in game, and that
/// message will show up on the Discord client, the SDK will inject a "fake" message into the chat,
/// that contains a basic English explanation of what is happening to the user. You can identify
/// these messages with the MessageHandle::DisclosureType method. We encourage you to customize the
/// rendering of these messages, possibly changing the wording, translating them, and making them
/// look more "official". You can choose to avoid rendering these as well.
///
/// # History
/// The SDK keeps the 25 most recent messages in each channel in memory, but it does not have access
/// to any historical messages sent before the SDK was connected. A MessageHandle will keep working
/// though even after the SDK has discarded the message for being too old, you just won't be able to
/// create a new MessageHandle objects for that message.
///
/// # Unrenderable Content
/// Messages sent on Discord can contain content that may not be renderable in game, such as images,
/// videos, embeds, polls, and more. The game isn't expected to render these, but instead show a
/// small notice so the user is aware there is more content and a way to view that content on
/// Discord. The MessageHandle::AdditionalContent method will contain data about the non-text
/// content in this message.
///
/// There is also more information about the struct of messages on Discord here:
/// https://discord.com/developers/docs/resources/message
///
/// Note: While the SDK allows you to send messages on behalf of a user, you must only do so in
/// response to a user action. You should never automatically send messages.
///
/// Handle objects in the SDK hold a reference both to the underlying data, and to the SDK instance.
/// Changes to the underlying data will generally be available on existing handles objects without
/// having to re-create them. If the SDK instance is destroyed, but you still have a reference to a
/// handle object, note that it will return the default value for all method calls (ie an empty
/// string for methods that return a string).
class MessageHandle {
    /// \cond
    mutable Discord_MessageHandle instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_MessageHandle* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit MessageHandle(Discord_MessageHandle instance, DiscordObjectState state);
    ~MessageHandle();
    /// \endcond
    /// Move constructor for MessageHandle
    MessageHandle(MessageHandle&& other) noexcept;
    /// Move assignment operator for MessageHandle
    MessageHandle& operator=(MessageHandle&& other) noexcept;
    /// Uninitialized instance of MessageHandle
    static const MessageHandle nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for MessageHandle
    MessageHandle(const MessageHandle& other);
    /// Copy assignment operator for MessageHandle
    MessageHandle& operator=(const MessageHandle& other);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief If the message contains non-text content, such as images, videos, embeds, polls, etc,
    /// this method will return information about that content.
    std::optional<discordpp::AdditionalContent> AdditionalContent() const;

    /// \brief Returns the UserHandle for the author of this message.
    std::optional<discordpp::UserHandle> Author() const;

    /// \brief Returns the user ID of the user who sent this message.
    uint64_t AuthorId() const;

    /// \brief Returns the ChannelHandle for the channel this message was sent in.
    std::optional<discordpp::ChannelHandle> Channel() const;

    /// \brief Returns the channel ID this message was sent in.
    uint64_t ChannelId() const;

    /// \brief Returns the content of this message, if any.
    ///
    /// A message can be blank if it was sent from Discord but only contains content such as image
    /// attachments. Certain types of markup, such as markup for emojis and mentions, will be auto
    /// replaced with a more human readable form, such as `@username` or `:emoji_name:`.
    std::string Content() const;

    /// \brief If this is an auto-generated message that is explaining some integration behavior to
    /// users, this method will return the type of disclosure so you can customize it.
    std::optional<discordpp::DisclosureTypes> DisclosureType() const;

    /// \brief The timestamp in millis since the epoch when the message was most recently edited.
    ///
    /// Returns 0 if the message has not been edited yet.
    uint64_t EditedTimestamp() const;

    /// \brief Returns the ID of this message.
    uint64_t Id() const;

    /// \brief Returns the LobbyHandle this message was sent in, if it was sent in a lobby.
    std::optional<discordpp::LobbyHandle> Lobby() const;

    /// \brief Returns any metadata the developer included with this message.
    ///
    /// Metadata is just a set of simple string key/value pairs.
    /// An example use case might be to include a character name so you can customize how a message
    /// renders in game.
    std::unordered_map<std::string, std::string> Metadata() const;

    /// \brief Returns the content of this message, if any, but without replacing any markup from
    /// emojis and mentions.
    ///
    /// A message can be blank if it was sent from Discord but only contains content such as image
    /// attachments.
    std::string RawContent() const;

    /// \brief Returns the UserHandle for the other participant in a DM, if this message was sent in
    /// a DM.
    std::optional<discordpp::UserHandle> Recipient() const;

    /// \brief When this message was sent in a DM or Ephemeral DM, this method will return the ID of
    /// the other user in that DM.
    uint64_t RecipientId() const;

    /// \brief Returns true if this message was sent in-game, otherwise false (i.e. from Discord
    /// itself).
    bool SentFromGame() const;

    /// \brief The timestamp in millis since the epoch when the message was sent.
    uint64_t SentTimestamp() const;
};

/// \brief Represents a single input or output audio device available to the user.
///
/// Discord will initialize the audio engine with the system default input and output devices.
/// You can change the device through the Client by passing the id of the desired audio device.
class AudioDevice {
    /// \cond
    mutable Discord_AudioDevice instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_AudioDevice* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit AudioDevice(Discord_AudioDevice instance, DiscordObjectState state);
    ~AudioDevice();
    /// \endcond
    /// Move constructor for AudioDevice
    AudioDevice(AudioDevice&& other) noexcept;
    /// Move assignment operator for AudioDevice
    AudioDevice& operator=(AudioDevice&& other) noexcept;
    /// Uninitialized instance of AudioDevice
    static const AudioDevice nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for AudioDevice
    AudioDevice(const AudioDevice& arg0);
    /// Copy assignment operator for AudioDevice
    AudioDevice& operator=(const AudioDevice& arg0);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Compares the ID of two AudioDevice objects for equality.
    bool Equals(discordpp::AudioDevice rhs);

    /// \brief The ID of the audio device.
    std::string Id() const;
    /// Setter for AudioDevice::Id.
    void SetId(std::string Id);

    /// \brief The display name of the audio device.
    std::string Name() const;
    /// Setter for AudioDevice::Name.
    void SetName(std::string Name);

    /// \brief Whether the audio device is the system default device.
    bool IsDefault() const;
    /// Setter for AudioDevice::IsDefault.
    void SetIsDefault(bool IsDefault);
};

/// \brief The Client class is the main entry point for the Discord SDK. All functionality is
/// exposed through this class.
///
/// See @ref getting_started "Getting Started" for more information on how to use the Client class.
class Client {
    /// \cond
    mutable Discord_Client instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \brief Represents an error state for the socket connection that the Discord SDK maintains
    /// with the Discord backend.
    ///
    /// Generic network failures will use the ConnectionFailed and ConnectionCanceled
    /// enum values. Other errors such as if the user's auth token is invalid or out of
    /// date will be UnexpectedClose and you should look at the other Error fields for the specific
    /// details.
    enum class Error {

        /// \brief None
        None = 0,

        /// \brief ConnectionFailed
        ConnectionFailed = 1,

        /// \brief UnexpectedClose
        UnexpectedClose = 2,

        /// \brief ConnectionCanceled
        ConnectionCanceled = 3,
    };

    /// \brief This enum refers to the status of the internal websocket the SDK uses to communicate
    /// with Discord There are ~2 phases for "launching" the client:
    /// 1. The socket has to connect to Discord and exchange an auth token. This is indicated by the
    /// `Connecting` and `Connected` values.
    /// 2. The socket has to receive an initial payload of data that describes the current user,
    /// what lobbies they are in, who their friends are, etc. This is the `Ready` status.
    /// Many Client functions will not work until the status changes to `Ready`, such as
    /// GetCurrentUser().
    ///
    /// Status::Ready is the one you want to wait for!
    ///
    /// Additionally, sometimes the socket will be disconnected, such as through temporary network
    /// blips. But it will try to automatically reconnect, as indicated by the `Reconnecting`
    /// status.
    enum class Status {

        /// \brief Disconnected
        Disconnected = 0,

        /// \brief Connecting
        Connecting = 1,

        /// \brief Connected
        Connected = 2,

        /// \brief Ready
        Ready = 3,

        /// \brief Reconnecting
        Reconnecting = 4,

        /// \brief Disconnecting
        Disconnecting = 5,

        /// \brief HttpWait
        HttpWait = 6,
    };

    /// \brief Represents the type of thread to control thread priority on.
    enum class Thread {

        /// \brief Client
        Client = 0,

        /// \brief Voice
        Voice = 1,

        /// \brief Network
        Network = 2,
    };

    /// \brief Callback invoked when Client::EndCall completes.
    using EndCallCallback = std::function<void()>;

    /// \brief Callback invoked when Client::EndCalls completes.
    using EndCallsCallback = std::function<void()>;

    /// \brief Callback function for Client::GetCurrentInputDevice.
    using GetCurrentInputDeviceCallback = std::function<void(discordpp::AudioDevice device)>;

    /// \brief Callback function for Client::GetCurrentOutputDevice.
    using GetCurrentOutputDeviceCallback = std::function<void(discordpp::AudioDevice device)>;

    /// \brief Callback function for Client::GetInputDevices.
    using GetInputDevicesCallback =
      std::function<void(std::vector<discordpp::AudioDevice> devices)>;

    /// \brief Callback function for Client::GetOutputDevices.
    using GetOutputDevicesCallback =
      std::function<void(std::vector<discordpp::AudioDevice> devices)>;

    /// \brief Callback function for Client::SetDeviceChangeCallback.
    using DeviceChangeCallback =
      std::function<void(std::vector<discordpp::AudioDevice> inputDevices,
                         std::vector<discordpp::AudioDevice> outputDevices)>;

    /// \brief Callback function for Client::SetInputDevice.
    using SetInputDeviceCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::SetNoAudioInputCallback.
    using NoAudioInputCallback = std::function<void(bool inputDetected)>;

    /// \brief Callback function for Client::SetOutputDevice.
    using SetOutputDeviceCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::SetVoiceParticipantChangedCallback.
    using VoiceParticipantChangedCallback =
      std::function<void(uint64_t lobbyId, uint64_t memberId, bool added)>;

    /// \brief Callback function for Client::StartCallWithAudioCallbacks.
    ///
    /// The audio samples in `data` can be modified in-place to achieve simple DSP effects.
    using UserAudioReceivedCallback = std::function<void(uint64_t userId,
                                                         int16_t* data,
                                                         uint64_t samplesPerChannel,
                                                         int32_t sampleRate,
                                                         uint64_t channels,
                                                         bool& outShouldMute)>;

    /// \brief Callback function for Client::StartCallWithAudioCallbacks.
    ///
    /// The audio samples in `data` can be modified in-place to achieve simple DSP effects.
    using UserAudioCapturedCallback = std::function<
      void(int16_t* data, uint64_t samplesPerChannel, int32_t sampleRate, uint64_t channels)>;

    /// \brief Callback invoked when the Authorize function completes.
    ///
    /// The first arg contains any error message encountered during the authorization flow, such as
    /// if the user cancelled the authorization. The second arg, code, contains an authorization
    /// _code_. This alone cannot be used to authorize with Discord, and instead must be exchanged
    /// for an access token later.
    using AuthorizationCallback = std::function<
      void(discordpp::ClientResult result, std::string code, std::string redirectUri)>;

    /// \brief Callback function for Client::FetchCurrentUser.
    using FetchCurrentUserCallback =
      std::function<void(discordpp::ClientResult result, uint64_t id, std::string name)>;

    /// \brief Callback function for the token exchange APIs such as Client::GetToken
    using TokenExchangeCallback = std::function<void(discordpp::ClientResult result,
                                                     std::string accessToken,
                                                     std::string refreshToken,
                                                     discordpp::AuthorizationTokenType tokenType,
                                                     int32_t expiresIn,
                                                     std::string scopes)>;

    /// \brief Callback function for Client::SetAuthorizeDeviceScreenClosedCallback.
    using AuthorizeDeviceScreenClosedCallback = std::function<void()>;

    /// \brief Callback function for Client::SetTokenExpirationCallback
    using TokenExpirationCallback = std::function<void()>;

    /// \brief Callback function for Client::UpdateProvisionalAccountDisplayName
    using UpdateProvisionalAccountDisplayNameCallback =
      std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback invoked when Client::UpdateToken completes. Once this is done it is safe to
    /// call Client::Connect.
    using UpdateTokenCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::DeleteUserMessage.
    using DeleteUserMessageCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::EditUserMessage.
    using EditUserMessageCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for when Client::ProvisionalUserMergeCompleted completes.
    using ProvisionalUserMergeRequiredCallback = std::function<void()>;

    /// \brief Callback function for when Client::OpenMessageInDiscord completes.
    using OpenMessageInDiscordCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief This is used for all kinds of 'send message' calls despite the name, to make sure
    /// engine bindings use the same delegate type declaration for all of them, which makes things
    /// nicer. `SendMessageCallback` was unavailable because it's a macro on Windows.
    using SendUserMessageCallback =
      std::function<void(discordpp::ClientResult result, uint64_t messageId)>;

    /// \brief Callback function for Client::SetMessageCreatedCallback.
    using MessageCreatedCallback = std::function<void(uint64_t messageId)>;

    /// \brief Callback function for Client::SetMessageDeletedCallback.
    using MessageDeletedCallback = std::function<void(uint64_t messageId, uint64_t channelId)>;

    /// \brief Callback function for Client::SetMessageUpdatedCallback.
    using MessageUpdatedCallback = std::function<void(uint64_t messageId)>;

    /// \brief Callback function invoked when a new log message is generated.
    using LogCallback =
      std::function<void(std::string message, discordpp::LoggingSeverity severity)>;

    /// \brief Callback function for Client::SetStatusChangedCallback.
    ///
    /// errorDetail will usually be one of the error code described here:
    /// https://discord.com/developers/docs/topics/opcodes-and-status-codes#gateway-gateway-close-event-codes
    using OnStatusChanged = std::function<
      void(discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail)>;

    /// \brief Callback function for Client::CreateOrJoinLobby
    using CreateOrJoinLobbyCallback =
      std::function<void(discordpp::ClientResult result, uint64_t lobbyId)>;

    /// \brief Callback function for Client::GetGuildChannels.
    using GetGuildChannelsCallback =
      std::function<void(discordpp::ClientResult result,
                         std::vector<discordpp::GuildChannel> guildChannels)>;

    /// \brief Callback function for Client::GetUserGuilds.
    using GetUserGuildsCallback = std::function<void(discordpp::ClientResult result,
                                                     std::vector<discordpp::GuildMinimal> guilds)>;

    /// \brief Callback function for Client::LeaveLobby.
    using LeaveLobbyCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::LinkChannelToLobby.
    using LinkOrUnlinkChannelCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::SetLobbyCreatedCallback.
    using LobbyCreatedCallback = std::function<void(uint64_t lobbyId)>;

    /// \brief Callback function for Client::SetLobbyDeletedCallback.
    using LobbyDeletedCallback = std::function<void(uint64_t lobbyId)>;

    /// \brief Callback function for Client::SetLobbyMemberAddedCallback.
    using LobbyMemberAddedCallback = std::function<void(uint64_t lobbyId, uint64_t memberId)>;

    /// \brief Callback function for Client::SetLobbyMemberRemovedCallback.
    using LobbyMemberRemovedCallback = std::function<void(uint64_t lobbyId, uint64_t memberId)>;

    /// \brief Callback function for Client::SetLobbyMemberUpdatedCallback.
    using LobbyMemberUpdatedCallback = std::function<void(uint64_t lobbyId, uint64_t memberId)>;

    /// \brief Callback function for Client::SetLobbyUpdatedCallback.
    using LobbyUpdatedCallback = std::function<void(uint64_t lobbyId)>;

    /// \brief Callback function for Client::AcceptActivityInvite.
    using AcceptActivityInviteCallback =
      std::function<void(discordpp::ClientResult result, std::string joinSecret)>;

    /// \brief Callback function for Client::SendActivityInvite, Client::SendActivityJoinRequest,
    /// and Client::SendActivityJoinRequestReply.
    using SendActivityInviteCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::SetActivityInviteCallback.
    using ActivityInviteCallback = std::function<void(discordpp::ActivityInvite invite)>;

    /// \brief Callback function for Client::SetActivityJoinCallback
    using ActivityJoinCallback = std::function<void(std::string joinSecret)>;

    /// \brief Callback function for when Client::SetOnlineStatus completes.
    using UpdateStatusCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for when Client::UpdateRichPresence completes.
    using UpdateRichPresenceCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for most other Relationship functions such as
    /// Client::SendDiscordFriendRequestById.
    using UpdateRelationshipCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::SendDiscordFriendRequest and
    /// Client::SendGameFriendRequest.
    using SendFriendRequestCallback = std::function<void(discordpp::ClientResult result)>;

    /// \brief Callback function for Client::SetRelationshipCreatedCallback, and
    /// Client::SetRelationshipDeletedCallback.
    using RelationshipCreatedCallback =
      std::function<void(uint64_t userId, bool isDiscordRelationshipUpdate)>;

    /// \brief Callback function for Client::SetRelationshipDeletedCallback
    using RelationshipDeletedCallback =
      std::function<void(uint64_t userId, bool isDiscordRelationshipUpdate)>;

    /// \brief Callback function for when Client::GetDiscordClientConnectedUser completes.
    using GetDiscordClientConnectedUserCallback =
      std::function<void(discordpp::ClientResult result,
                         std::optional<discordpp::UserHandle> user)>;

    /// \brief Callback function for Client::SetUserUpdatedCallback.
    using UserUpdatedCallback = std::function<void(uint64_t userId)>;
    /// \cond
    Discord_Client* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit Client(Discord_Client instance, DiscordObjectState state);
    ~Client();
    /// \endcond
    /// Move constructor for Client
    Client(Client&& other) noexcept;
    /// Move assignment operator for Client
    Client& operator=(Client&& other) noexcept;
    /// Uninitialized instance of Client
    static const Client nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    /// \brief Creates a new instance of the Client.
    explicit Client();

    /// \brief Creates a new instance of the Client but allows customizing the Discord URL to use.
    explicit Client(std::string apiBase, std::string webBase);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Converts the Error enum to a string.
    static std::string ErrorToString(discordpp::Client::Error type);

    /// \brief This function is used to get the application ID for the client. This is used to
    /// identify the application to the Discord client. This is used for things like
    /// authentication, rich presence, and activity invites when *not* connected with
    /// Client::Connect. When calling Client::Connect, the application ID is set automatically
    uint64_t GetApplicationId();

    /// \brief Returns the ID of the system default audio device if the user has not explicitly
    /// chosen one.
    static std::string GetDefaultAudioDeviceId();

    /// \brief Returns the default set of OAuth2 scopes that should be used with the Discord SDK
    /// when making use of the full SDK capabilities, including communications-related features
    /// (e.g. user DMs, lobbies, voice chat). If your application does not make use of these
    /// features, you should use Client::GetDefaultPresenceScopes instead.
    ///
    /// Communications-related features are currently in limited access and are not available to
    /// all applications, however, they can be demoed in limited capacity by all applications. If
    /// you are interested in using these features in your game, please reach out to the Discord
    /// team.
    ///
    /// It's ok to further customize your requested oauth2 scopes to add additional scopes if you
    /// have legitimate usages for them. However, we strongly recommend always using
    /// Client::GetDefaultCommunicationScopes or Client::GetDefaultPresenceScopes as a baseline to
    /// enable a better authorization experience for users!
    static std::string GetDefaultCommunicationScopes();

    /// \brief Returns the default set of OAuth2 scopes that should be used with the Discord SDK
    /// when leveraging baseline presence-related features (e.g. friends list, rich presence,
    /// provisional accounts, activity invites). If your application is using
    /// communications-related features, which are currently available in limited access, you should
    /// use Client::GetDefaultCommunicationScopes instead.
    ///
    /// It's ok to further customize your requested oauth2 scopes to add additional scopes if you
    /// have legitimate usages for them. However, we strongly recommend always using
    /// Client::GetDefaultCommunicationScopes or Client::GetDefaultPresenceScopes as a baseline to
    /// enable a better authorization experience for users!
    static std::string GetDefaultPresenceScopes();

    /// \brief Returns the git commit hash this version was built from.
    static std::string GetVersionHash();

    /// \brief Returns the major version of the Discord Social SDK.
    static int32_t GetVersionMajor();

    /// \brief Returns the minor version of the Discord Social SDK.
    static int32_t GetVersionMinor();

    /// \brief Returns the patch version of the Discord Social SDK.
    static int32_t GetVersionPatch();

    /// \brief Converts the Status enum to a string.
    static std::string StatusToString(discordpp::Client::Status type);

    /// \brief Converts the Thread enum to a string.
    static std::string ThreadToString(discordpp::Client::Thread type);

    /// @name Audio
    /// @{

    /// \brief Ends any active call, if any. Any references you have to Call objects are invalid
    /// after they are ended, and can be immediately freed.
    void EndCall(uint64_t channelId, discordpp::Client::EndCallCallback callback);

    /// \brief Ends any active call, if any. Any references you have to Call objects are invalid
    /// after they are ended, and can be immediately freed.
    void EndCalls(discordpp::Client::EndCallsCallback callback);

    /// \brief Returns a reference to the currently active call, if any.
    discordpp::Call GetCall(uint64_t channelId);

    /// \brief Returns a reference to all currently active calls, if any.
    std::vector<discordpp::Call> GetCalls();

    /// \brief Asynchronously fetches the current audio input device in use by the client.
    void GetCurrentInputDevice(discordpp::Client::GetCurrentInputDeviceCallback cb);

    /// \brief Asynchronously fetches the current audio output device in use by the client.
    void GetCurrentOutputDevice(discordpp::Client::GetCurrentOutputDeviceCallback cb);

    /// \brief Asynchronously fetches the list of audio input devices available to the user.
    void GetInputDevices(discordpp::Client::GetInputDevicesCallback cb);

    /// \brief Returns the input volume for the current user's microphone.
    ///
    /// Input volume is specified as a percentage in the range [0, 100] which represents the
    /// perceptual loudness.
    float GetInputVolume();

    /// \brief Asynchronously fetches the list of audio output devices available to the user.
    void GetOutputDevices(discordpp::Client::GetOutputDevicesCallback cb);

    /// \brief Returns the output volume for the current user.
    ///
    /// Output volume specified as a percentage in the range [0, 200] which represents the
    /// perceptual loudness.
    float GetOutputVolume();

    /// \brief Returns whether the current user is deafened in all calls.
    bool GetSelfDeafAll() const;

    /// \brief Returns whether the current user's microphone is muted in all calls.
    bool GetSelfMuteAll() const;

    /// \brief When enabled, automatically adjusts the microphone volume to keep it clear and
    /// consistent.
    ///
    /// Defaults to on.
    ///
    /// Generally this shouldn't need to be used unless you are building a voice settings UI for the
    /// user to control, similar to Discord's voice settings.
    void SetAutomaticGainControl(bool on);

    /// \brief Sets a callback function to be invoked when Discord detects a change in the available
    /// audio devices.
    void SetDeviceChangeCallback(discordpp::Client::DeviceChangeCallback callback);

    /// \brief Enables or disables the basic echo cancellation provided by the WebRTC library.
    ///
    /// Defaults to on.
    ///
    /// Generally this shouldn't need to be used unless you are building a voice settings UI for the
    /// user to control, similar to Discord's voice settings.
    void SetEchoCancellation(bool on);

    /// \brief Asynchronously changes the audio input device in use by the client to the specified
    /// device. You can find the list of device IDs that can be passed in with the
    /// Client::GetInputDevices function.
    void SetInputDevice(std::string deviceId, discordpp::Client::SetInputDeviceCallback cb);

    /// \brief Sets the microphone volume for the current user.
    ///
    /// Input volume is specified as a percentage in the range [0, 100] which represents the
    /// perceptual loudness.
    void SetInputVolume(float inputVolume);

    /// \brief Callback function invoked when the above threshold is set and there is a change in
    /// whether audio is being detected.
    void SetNoAudioInputCallback(discordpp::Client::NoAudioInputCallback callback);

    /// \brief Threshold that can be set to indicate when no audio is being received by the user's
    /// mic.
    ///
    /// An example of when this may be useful: When push to talk is being used and the user pushes
    /// their talk key, but something is configured wrong and no audio is being received, this
    /// threshold and callback can be used to detect that situation and notify the user. The
    /// threshold is specified in DBFS, or decibels relative to full scale, and the range is
    /// [-100.0, 100.0] It defaults to -100.0, so is disabled.
    void SetNoAudioInputThreshold(float dBFSThreshold);

    /// \brief Enables basic background noise suppression.
    ///
    /// Defaults to on.
    ///
    /// Generally this shouldn't need to be used unless you are building a voice settings UI for the
    /// user to control, similar to Discord's voice settings.
    void SetNoiseSuppression(bool on);

    /// \brief Enables or disables hardware encoding and decoding for audio, if it is available.
    ///
    /// Defaults to on.
    ///
    /// This must be called immediately after constructing the Client. If called too late an error
    /// will be logged and the setting will not take effect.
    void SetOpusHardwareCoding(bool encode, bool decode);

    /// \brief Asynchronously changes the audio output device in use by the client to the specified
    /// device. You can find the list of device IDs that can be passed in with the
    /// Client::GetOutputDevices function.
    void SetOutputDevice(std::string deviceId, discordpp::Client::SetOutputDeviceCallback cb);

    /// \brief Sets the speaker volume for the current user.
    ///
    /// Output volume specified as a percentage in the range [0, 200] which represents the
    /// perceptual loudness.
    void SetOutputVolume(float outputVolume);

    /// \brief Mutes all audio from the currently active call for the current user in all calls.
    /// They will not be able to hear any other participants,
    /// and no other participants will be able to hear the current user either.
    /// Note: This overrides the per-call setting.
    void SetSelfDeafAll(bool deaf);

    /// \brief Mutes the current user's microphone so that no other participant in their active
    /// calls can hear them in all calls. Note: This overrides the per-call setting.
    void SetSelfMuteAll(bool mute);

    /// \brief On mobile devices, enable speakerphone mode.
    bool SetSpeakerMode(bool speakerMode);

    /// \brief Allows setting the priority of various SDK threads.
    ///
    /// The threads that can be controlled are:
    /// - Client: This is the main thread for the SDK where most of the data processing happens
    /// - Network: This is the thread that receives voice data from lobby calls
    /// - Voice: This is the thread that the voice engine runs on and processes all audio data
    void SetThreadPriority(discordpp::Client::Thread thread, int32_t priority);

    /// \brief Callback invoked whenever a user in a lobby joins or leaves a voice call.
    ///
    /// The main use case for this is to enable displaying which users are in voice in a lobby
    /// even if the current user is not in voice yet, and thus does not have a Call object to bind
    /// to.
    void SetVoiceParticipantChangedCallback(discordpp::Client::VoiceParticipantChangedCallback cb);

    /// \brief On iOS devices, show the system audio route picker.
    bool ShowAudioRoutePicker();

    /// \brief Starts or joins a call in the lobby specified by channelId (For a lobby, simply
    /// pass in the lobbyId).
    ///
    /// On iOS, your application is responsible for enabling the appropriate background audio mode
    /// in your Info.plist. VoiceBuildPostProcessor in the sample demonstrates how to do this
    /// automatically in your Unity build process.
    ///
    /// On macOS, you should set the NSMicrophoneUsageDescription key in your Info.plist.
    ///
    /// Returns null if the user is already in the given voice channel.
    discordpp::Call StartCall(uint64_t channelId);

    /// \brief Starts or joins a call in the specified lobby.
    ///
    /// The audio received callback is invoked whenever incoming audio is received in a call. If
    /// the developer sets outShouldMute to true during the callback, the audio data will be muted
    /// after the callback is invoked, which is useful if the developer is utilizing the incoming
    /// audio and playing it through their own audio engine or playback. The audio samples
    /// in `data` can be modified in-place for simple DSP effects.
    ///
    /// The audio captured callback is invoked whenever local audio is captured before it is
    /// processed and transmitted which may be useful for voice moderation, etc.
    ///
    /// On iOS, your application is responsible for enabling the appropriate background audio mode
    /// in your Info.plist. VoiceBuildPostProcessor in the sample demonstrates how to do this
    /// automatically in your Unity build process.
    ///
    /// On macOS, you should set the NSMicrophoneUsageDescription key in your Info.plist.
    ///
    /// Returns null if the user is already in the given voice channel.
    discordpp::Call StartCallWithAudioCallbacks(
      uint64_t lobbyId,
      discordpp::Client::UserAudioReceivedCallback receivedCb,
      discordpp::Client::UserAudioCapturedCallback capturedCb);
    /// @}

    /// @name Auth
    /// @{

    /// \brief This will abort the authorize flow if it is in progress and tear down any associated
    /// state.
    ///
    /// NOTE: this *will not* close authorization windows presented to the user.
    void AbortAuthorize();

    /// \brief This function is used to abort/cleanup the device authorization flow.
    void AbortGetTokenFromDevice();

    /// \brief Initiates an OAuth2 flow for a user to "sign in with Discord". This flow is intended
    /// for desktop and mobile devices. If you are implementing for the console, leverage the device
    /// auth flow instead (Client::GetTokenFromDevice or Client::OpenAuthorizeDeviceScreen).
    ///
    /// ## Overview
    /// If you're not familiar with OAuth2, some basic background: At a high level the goal of
    /// OAuth2 is to allow a user to connect two applications together and share data between them.
    /// In this case, allowing a game to access some of their Discord data. The high level flow is:
    /// - This function, Authorize, is invoked to start the OAuth2 process, and the user is sent to
    /// Discord
    /// - On Discord, the user sees a prompt to authorize the connection, and that prompt explains
    /// what data and functionality the game is requesting.
    /// - Once the user approves the connection, they are redirected back to your application with a
    /// secret code.
    /// - You can then exchange that secret code to get back an access token which can be used to
    /// authenticate with the SDK.
    ///
    /// ## Public vs Confidential Clients
    /// Normal OAuth2 requires a backend server to handle exchanging the "code" for a "token" (the
    /// last step mentioned above). Not all games have backend servers or their own identity system
    /// though, and for early testing of the SDK that can take some time to setup.
    ///
    /// If desired, you can instead change your Discord application in the developer portal (on the
    /// OAuth2 tab), to be a "public" client. This will allow you to exchange the code for a token
    /// without a backend server, by using the GetToken function below. You can also change this
    /// setting back once you have a backend in place later too.
    ///
    /// ## Overlay
    /// To streamline the authentication process, the SDK will attempt to use the Discord overlay if
    /// it is enabled. This will allow the user to authenticate without leaving the game, enabling a
    /// more seamless experience.
    ///
    /// You should check to see if the Discord overlay works with your game before shipping. It's
    /// ok if it doesn't though, the SDK will fall back to using a browser window. Once you're ready
    /// to ship, you can work with us to have the overlay enabled by default for your game too.
    ///
    /// If your game's main window is not the same process that the SDK is running in, then you need
    /// to tell the SDK the PID of the window that the overlay should attach to. You can do this by
    /// calling Client::SetGameWindowPid.
    ///
    /// ## Redirects
    /// For the Authorize function to work, you must configure a redirect url in your Discord
    /// application in the developer portal, (it is located on the OAuth2 tab).
    /// - For desktop applications, add `http://127.0.0.1/callback`
    /// - For mobile applications, add `discord-APP_ID:/authorize/callback`
    ///
    /// The SDK will then spin up a local webserver to handle the OAuth2 redirects for you as
    /// well to streamline your integration.
    ///
    /// ## Security
    /// This function accepts an args object, and two of those values are important for security:
    /// - To prevent CSRF attacks during auth, the SDK automatically attaches a state and checks it
    /// for you when performing the authorization. You can override state if you want for your own
    /// flow, but please be mindful to keep it a secure, random value.
    /// - If you are using the Client::GetToken function you will need to provide a "code challenge"
    /// or "code verifier". We'll spare you the boring details of how that works (woo… crypto), as
    /// we've made a simple function to create these for you,
    /// Client::CreateAuthorizationCodeVerifier. That returns a struct with two items, a `challenge`
    /// value to pass into this function and a `verifier` value to pass into Client::GetToken.
    ///
    /// ## Callbacks & Code Exchange
    /// When this flow completes, the given callback function will be invoked with a "code". That
    /// code must be exchanged for an actual authorization token before it can be used. To start,
    /// you can use the Client::GetToken function to perform this exchange. Longer term private apps
    /// will want to move to the server side API for this, since that enables provisional accounts
    /// to "upgrade" to full Discord accounts.
    ///
    /// ## Android
    /// You must add the appropriate intent filter to your `AndroidManifest.xml`.
    /// `AndroidBuildPostProcessor` in the sample demonstrates how to do this automatically.
    ///
    /// If you'd like to manage it yourself, the required entry in your `<application>` looks like
    /// this:
    /// ```xml
    /// <activity android:name="com.discord.socialsdk.AuthenticationActivity"
    /// android:exported="true">
    ///   <intent-filter>
    ///     <action android:name="android.intent.action.VIEW" />
    ///     <category android:name="android.intent.category.DEFAULT" />
    ///     <category android:name="android.intent.category.BROWSABLE" />
    ///     <data android:scheme="discord-1234567890123456789" />
    ///   </intent-filter>
    /// </activity>
    /// ```
    /// Replace the numbers after `discord-` with your Application ID from the Discord developer
    /// portal.
    ///
    /// Android support (specifically the builtin auth flow) requires the androidx.browser library
    /// as a dependency of your app. The sample uses Google External Dependency Manager to add this
    /// to the Gradle build for the project, but you may use any means of your choosing to add this
    /// dependency. We currently depend on version 1.8.0 of this library.
    ///
    /// For more information see: https://discord.com/developers/docs/topics/oauth2
    void Authorize(discordpp::AuthorizationArgs args,
                   discordpp::Client::AuthorizationCallback callback);

    /// \brief This function is used to hide the device authorization screen and is used for the
    /// case where the user is on a limited input device, such as a console or smart TV. This
    /// function should be used in conjunction with a backend server to handle the device
    /// authorization flow. For a public client, you can use Client::AbortGetTokenFromDevice
    /// instead.
    void CloseAuthorizeDeviceScreen();

    /// \brief Helper function that can create a code challenge and verifier for use in the
    /// Client::Authorize + Client::GetToken flow. This returns a struct with two items, a
    /// `challenge` value to pass into Client::Authorize and a `verifier` value to pass into
    /// GetToken.
    discordpp::AuthorizationCodeVerifier CreateAuthorizationCodeVerifier();

    /// \brief Fetches basic information about the user associated with the given auth token.
    ///
    /// This can allow you to check if an auth token is valid or not.
    /// This does not require the client to be connected or to have it's own authentication token,
    /// so it can be called immediately after the client connects.
    void FetchCurrentUser(discordpp::AuthorizationTokenType tokenType,
                          std::string const& token,
                          discordpp::Client::FetchCurrentUserCallback callback);

    /// \brief Provisional accounts are a way for users that have not signed up for Discord to still
    /// access SDK functionality. They are "placeholder" Discord accounts for the user that are
    /// owned and managed by your game. Provisional accounts exist so that your users can engage
    /// with Discord APIs and systems without the friction of creating their own Discord account.
    /// Provisional accounts and their data are unique per Discord application.
    ///
    /// This function generates a Discord access token. You pass in the "identity" of the user, and
    /// it generates a new Discord account that is tied to that identity. There are multiple ways of
    /// specifying that identity, including using Steam/Epic services, or using your own identity
    /// system.
    ///
    /// The callback function will be invoked with an access token that expires in 1 hour. Refresh
    /// tokens are not supported for provisional accounts, so that will be an empty string. You
    /// will need to call this function again to get a new access token when the old one expires.
    ///
    /// NOTE: When the token expires the SDK will still continue to receive updates such as new
    /// messages sent in a lobby, and any voice calls will continue to be active. But any new
    /// actions taken will fail such as sending a messaging or adding a friend. You can get a new
    /// token and pass it to UpdateToken without interrupting the user's experience.
    ///
    /// It is suggested that these provisional tokens are not stored, and instead to just invoke
    /// this function each time the game is launched and when these tokens are about to expire.
    /// However, should you choose to store it, it is recommended to differentiate these provisional
    /// account tokens from "full" Discord account tokens.
    ///
    /// NOTE: This function only works for public clients. Public clients are ones that do not have
    /// a backend server or their own concept of user accounts and simply rely on a separate system
    /// for authentication like Steam/Epic.
    ///
    /// When first testing the SDK, it can be a lot easier to use a public client to get a proof of
    /// concept going, and change it to a confidential client later. You can toggle that setting on
    /// the OAuth2 page for your application in the Discord developer portal,
    /// https://discord.com/developers/applications
    void GetProvisionalToken(uint64_t applicationId,
                             discordpp::AuthenticationExternalAuthType externalAuthType,
                             std::string const& externalAuthToken,
                             discordpp::Client::TokenExchangeCallback callback);

    /// \brief Exchanges an authorization code that was returned from the Client::Authorize function
    /// for an access token which can be used to authenticate with the SDK.
    ///
    /// The callback function will be invoked with two tokens:
    /// - An access token which can be used to authenticate with the SDK, but expires after 7 days.
    /// - A refresh token, which cannot be used to authenticate, but can be used to get a new access
    /// token later. Refresh tokens do not currently expire.
    ///
    /// It will also include when the access token expires in seconds.
    /// You will want to store this value as well and refresh the token when it gets close to
    /// expiring (for example if the user launches the game and the token expires within 24 hours,
    /// it would be good to refresh it).
    ///
    /// For more information see https://discord.com/developers/docs/topics/oauth2
    ///
    /// NOTE: This function only works for public clients. Public clients are ones that do not have
    /// a backend server or their own concept of user accounts and simply rely on a separate system
    /// for authentication like Steam/Epic.
    ///
    /// When first testing the SDK, it can be a lot easier to use a public client to get a proof of
    /// concept going, and change it to a confidential client later. You can toggle that setting on
    /// the OAuth2 page for your application in the Discord developer portal,
    /// https://discord.com/developers/applications
    void GetToken(uint64_t applicationId,
                  std::string const& code,
                  std::string const& codeVerifier,
                  std::string const& redirectUri,
                  discordpp::Client::TokenExchangeCallback callback);

    /// \brief This function is a combination of Client::Authorize and Client::GetToken, but is used
    /// for the case where the user is on a limited input device, such as a console or smart TV.
    ///
    /// The callback function will be invoked with two tokens:
    /// - An access token which can be used to authenticate with the SDK, but expires after 7 days.
    /// - A refresh token, which cannot be used to authenticate, but can be used to get a new access
    /// token later. Refresh tokens do not currently expire.
    ///
    /// It will also include when the access token expires in seconds.
    /// You will want to store this value as well and refresh the token when it gets close to
    /// expiring (for example if the user launches the game and the token expires within 24 hours,
    /// it would be good to refresh it).
    ///
    /// For more information see https://discord.com/developers/docs/topics/oauth2
    ///
    /// NOTE: This function only works for public clients. Public clients are ones that do not have
    /// a backend server or their own concept of user accounts and simply rely on a separate system
    /// for authentication like Steam/Epic. If you have a backend server for auth, you can use
    /// Client::OpenAuthorizeDeviceScreen and Client::CloseAuthorizeDeviceScreen to show/hide the UI
    /// for the device auth flow.
    ///
    /// When first testing the SDK, it can be a lot easier to use a public client to get a proof of
    /// concept going, and change it to a confidential client later. You can toggle that setting on
    /// the OAuth2 page for your application in the Discord developer portal,
    /// https://discord.com/developers/applications
    void GetTokenFromDevice(discordpp::DeviceAuthorizationArgs args,
                            discordpp::Client::TokenExchangeCallback callback);

    /// \brief This function is a combination of Client::Authorize and
    /// Client::GetTokenFromProvisionalMerge, but is used for the case where the user is on a
    /// limited input device, such as a console or smart TV.
    ///
    /// This function should be used whenever a user with a provisional account wants to link to an
    /// existing Discord account or "upgrade" their provisional account into a "full" Discord
    /// account.
    ///
    /// In this case, data from the provisional account should be "migrated" to the Discord
    /// account, a process we call "account merging". Specifically relationships, DMs, and lobby
    /// memberships are transferred to the Discord account.
    ///
    /// The provisional account will be deleted once this merging process completes. If the user
    /// later unlinks, then a new provisional account with a new unique ID is created.
    ///
    /// The account merging process starts the same as the normal login flow, by invoking the
    /// GetTokenFromDevice. But instead of calling GetTokenFromDevice, call this function and pass
    /// in the provisional user's identity as well.
    ///
    /// The Discord backend can then find both the provisional account with that identity and the
    /// new Discord account and merge any data as necessary.
    ///
    /// See the documentation for GetTokenFromDevice for more details on the callback. Note that the
    /// callback will be invoked when the token exchange completes, but the process of merging
    /// accounts happens asynchronously so will not be complete yet.
    ///
    /// NOTE: This function only works for public clients. Public clients are ones that do not have
    /// a backend server or their own concept of user accounts and simply rely on a separate system
    /// for authentication like Steam/Epic. If you have a backend server for auth, you can use
    /// Client::OpenAuthorizeDeviceScreen and Client::CloseAuthorizeDeviceScreen to show/hide the UI
    /// for the device auth flow.
    ///
    /// When first testing the SDK, it can be a lot easier to use a public client to get a proof of
    /// concept going, and change it to a confidential client later. You can toggle that setting on
    /// the OAuth2 page for your application in the Discord developer portal,
    /// https://discord.com/developers/applications
    void GetTokenFromDeviceProvisionalMerge(
      discordpp::DeviceAuthorizationArgs args,
      discordpp::AuthenticationExternalAuthType externalAuthType,
      std::string const& externalAuthToken,
      discordpp::Client::TokenExchangeCallback callback);

    /// \brief This function should be used with the Client::Authorize function whenever a user with
    /// a provisional account wants to link to an existing Discord account or "upgrade" their
    /// provisional account into a "full" Discord account.
    ///
    /// In this case, data from the provisional account should be "migrated" to the Discord
    /// account, a process we call "account merging". Specifically relationships, DMs, and lobby
    /// memberships are transferred to the Discord account.
    ///
    /// The provisional account will be deleted once this merging process completes. If the user
    /// later unlinks, then a new provisional account with a new unique ID is created.
    ///
    /// The account merging process starts the same as the normal login flow, by invoking the
    /// Authorize method to get an authorization code back. But instead of calling GetToken, call
    /// this function and pass in the provisional user's identity as well.
    ///
    /// The Discord backend can then find both the provisional account with that identity and the
    /// new Discord account and merge any data as necessary.
    ///
    /// See the documentation for GetToken for more details on the callback. Note that the callback
    /// will be invoked when the token exchange completes, but the process of merging accounts
    /// happens asynchronously so will not be complete yet.
    ///
    /// NOTE: This function only works for public clients. Public clients are ones that do not have
    /// a backend server or their own concept of user accounts and simply rely on a separate system
    /// for authentication like Steam/Epic.
    ///
    /// When first testing the SDK, it can be a lot easier to use a public client to get a proof of
    /// concept going, and change it to a confidential client later. You can toggle that setting on
    /// the OAuth2 page for your application in the Discord developer portal,
    /// https://discord.com/developers/applications
    void GetTokenFromProvisionalMerge(uint64_t applicationId,
                                      std::string const& code,
                                      std::string const& codeVerifier,
                                      std::string const& redirectUri,
                                      discordpp::AuthenticationExternalAuthType externalAuthType,
                                      std::string const& externalAuthToken,
                                      discordpp::Client::TokenExchangeCallback callback);

    /// \brief Returns true if the SDK has a non-empty OAuth2 token set, regardless of whether that
    /// token is valid or not.
    bool IsAuthenticated();

    /// \brief This function is used to show the device authorization screen and is used for the
    /// case where the user is on a limited input device, such as a console or smart TV. This
    /// function should be used in conjunction with a backend server to handle the device
    /// authorization flow. For a public client, you can use Client::GetTokenFromDevice instead.
    void OpenAuthorizeDeviceScreen(uint64_t clientId, std::string const& userCode);

    /// \brief Some functions don't work for provisional accounts, and require the user
    /// merge their account into a full Discord account before proceeding. This
    /// callback is invoked when an account merge must take place before
    /// proceeding. The developer is responsible for initiating the account merge,
    /// and then calling Client::ProvisionalUserMergeCompleted to signal to the SDK that
    /// the pending operation can continue with the new account.
    void ProvisionalUserMergeCompleted(bool success);

    /// \brief Generates a new access token for the current user from a refresh token.
    ///
    /// Once this is called, the old access and refresh tokens are both invalidated and cannot be
    /// used again. The callback function will be invoked with a new access and refresh token. See
    /// GetToken for more details.
    ///
    /// NOTE: This function only works for public clients. Public clients are ones that do not have
    /// a backend server or their own concept of user accounts and simply rely on a separate system
    /// for authentication like Steam/Epic.
    ///
    /// When first testing the SDK, it can be a lot easier to use a public client to get a proof of
    /// concept going, and change it to a confidential client later. You can toggle that setting on
    /// the OAuth2 page for your application in the Discord developer portal,
    /// https://discord.com/developers/applications
    void RefreshToken(uint64_t applicationId,
                      std::string const& refreshToken,
                      discordpp::Client::TokenExchangeCallback callback);

    /// \brief Sets a callback function to be invoked when the device authorization screen is
    /// closed.
    void SetAuthorizeDeviceScreenClosedCallback(
      discordpp::Client::AuthorizeDeviceScreenClosedCallback cb);

    /// \brief When users are linking their account with Discord, which involves an OAuth2 flow,
    /// the SDK can streamline it by using Discord's overlay so the interaction happens entirely
    /// in-game. If your game's main window is not the same process as the one running the
    /// integration you may need to set the window PID using this method. It defaults to the current
    /// pid.
    void SetGameWindowPid(int32_t pid);

    /// \brief Get a notification when the current token is about to expire or expired.
    ///
    /// This callback is invoked when the SDK detects that the last token passed to
    /// Client::UpdateToken is nearing expiration or has expired. This is a signal to the developer
    /// to refresh the token. The callback is invoked once per token, and will not be invoked again
    /// until a new token is passed to Client::UpdateToken.
    ///
    /// If the token is refreshed before the expiration callback is invoked, call
    /// Client::UpdateToken to pass in the new token and reconfigure the token expiration.
    ///
    /// If your client is disconnected (the token was expired when connecting or was revoked while
    /// connected), the expiration callback will not be invoked.
    void SetTokenExpirationCallback(discordpp::Client::TokenExpirationCallback callback);

    /// \brief Updates the display name of a provisional account to the specified name.
    ///
    /// This should generally be invoked whenever the SDK starts and whenever a provisional account
    /// changes their name, since the auto-generated name for provisional accounts is just a random
    /// string.
    void UpdateProvisionalAccountDisplayName(
      std::string const& name,
      discordpp::Client::UpdateProvisionalAccountDisplayNameCallback callback);

    /// \brief Asynchronously sets a new auth token for this client to use.
    ///
    /// If your client is already connected, this function *may* trigger a reconnect.
    /// If your client is not connected, this function will only update the auth token, and so you
    /// must invoke Client::Connect as well. You should wait for the given callback function to be
    /// invoked though so that the next Client::Connect attempt uses the updated token.
    void UpdateToken(discordpp::AuthorizationTokenType tokenType,
                     std::string token,
                     discordpp::Client::UpdateTokenCallback callback);
    /// @}

    /// @name Chat
    /// @{

    /// \brief Returns true if the given message is able to be viewed in a Discord client.
    ///
    /// Not all chat messages are replicated to Discord. For example lobby chat and some DMs
    /// are ephemeral and not persisted on Discord so cannot be opened. This function checks those
    /// conditions and makes sure the message is viewable in Discord.
    bool CanOpenMessageInDiscord(uint64_t messageId);

    /// \brief Deletes the specified message sent by the current user to the specified recipient.
    void DeleteUserMessage(uint64_t recipientId,
                           uint64_t messageId,
                           discordpp::Client::DeleteUserMessageCallback cb);

    /// \brief Edits the specified message sent by the current user to the specified recipient.
    ///
    /// All of the same restrictions apply as for sending a message, see SendUserMessage for more.
    void EditUserMessage(uint64_t recipientId,
                         uint64_t messageId,
                         std::string const& content,
                         discordpp::Client::EditUserMessageCallback cb);

    /// \brief Returns a reference to the Discord channel object for the given ID.
    ///
    /// All messages in Discord are sent in a channel, so the most common use for this will be
    /// to look up the channel a message was sent in.
    /// For convience this API will also work with lobbies, so the three possible return values
    /// for the SDK are a DM, an Ephemeral DM, and a Lobby.
    std::optional<discordpp::ChannelHandle> GetChannelHandle(uint64_t channelId) const;

    /// \brief Returns a reference to the Discord message object for the given ID.
    ///
    /// The SDK keeps the 25 most recent messages in each channel in memory.
    /// Messages sent before the SDK was started cannot be accessed with this.
    std::optional<discordpp::MessageHandle> GetMessageHandle(uint64_t messageId) const;

    /// \brief Opens the given message in the Discord client.
    ///
    /// This is useful when a message is sent that contains content that cannot be viewed in
    /// Discord. You can call this function in the click handler for any CTA you show to view the
    /// message in Discord.
    void OpenMessageInDiscord(
      uint64_t messageId,
      discordpp::Client::ProvisionalUserMergeRequiredCallback provisionalUserMergeRequiredCallback,
      discordpp::Client::OpenMessageInDiscordCallback callback);

    /// \brief Sends a message in a lobby chat to all members of the lobby.
    ///
    /// The content of the message is restricted to 2,000 characters maximum.
    /// See https://discord.com/developers/docs/resources/message for more details.
    ///
    /// The content of the message can also contain special markup for formatting if desired, see
    /// https://discord.com/developers/docs/reference#message-formatting for more details.
    ///
    /// If the lobby is linked to a channel, the message will also be sent to that channel on
    /// Discord.
    void SendLobbyMessage(uint64_t lobbyId,
                          std::string const& content,
                          discordpp::Client::SendUserMessageCallback cb);

    /// \brief Variant of Client::SendLobbyMessage that also accepts metadata to be sent with the
    /// message.
    ///
    /// Metadata is just simple string key/value pairs.
    /// An example use case for this might be to include the name of the character that sent a
    /// message.
    void SendLobbyMessageWithMetadata(uint64_t lobbyId,
                                      std::string const& content,
                                      std::unordered_map<std::string, std::string> const& metadata,
                                      discordpp::Client::SendUserMessageCallback cb);

    /// \brief Sends a direct message to the specified user.
    ///
    /// The content of the message is restricted to 2,000 characters maximum.
    /// See https://discord.com/developers/docs/resources/message for more details.
    ///
    /// The content of the message can also contain special markup for formatting if desired, see
    /// https://discord.com/developers/docs/reference#message-formatting for more details.
    ///
    /// A message can be sent between two users in the following situations:
    /// - Both users are online and in the game and have not blocked each other
    /// - Both users are friends with each other
    /// - Both users share a mutual Discord server and have previously DM'd each other on Discord
    void SendUserMessage(uint64_t recipientId,
                         std::string const& content,
                         discordpp::Client::SendUserMessageCallback cb);

    /// \brief Variant of Client::SendUserMessage that also accepts metadata to be sent with the
    /// message.
    ///
    /// Metadata is just simple string key/value pairs.
    /// An example use case for this might be to include the name of the character that sent a
    /// message.
    void SendUserMessageWithMetadata(uint64_t recipientId,
                                     std::string const& content,
                                     std::unordered_map<std::string, std::string> const& metadata,
                                     discordpp::Client::SendUserMessageCallback cb);

    /// \brief Sets a callback to be invoked whenever a new message is received in either a lobby or
    /// a DM.
    ///
    /// From the messageId you can fetch the MessageHandle and then the ChannelHandle to determine
    /// the location the message was sent as well.
    ///
    /// If the user has the Discord desktop application open on the same machine as the game, then
    /// they will hear notifications from the Discord application, even though they are able to see
    /// those messages in game. So to avoid double-notifying users, you should call
    /// Client::SetShowingChat whenever the chat is shown or hidden to suppress those duplicate
    /// notifications.
    void SetMessageCreatedCallback(discordpp::Client::MessageCreatedCallback cb);

    /// \brief Sets a callback to be invoked whenever a message is deleted.
    ///
    /// Some messages sent from in game, as well as all messages sent from a connected user's
    /// Discord client can be edited and deleted in the Discord client. So it is valuable to
    /// implement support for this callback so that if a user edits or deletes a message in the
    /// Discord client, it is reflected in game as well.
    void SetMessageDeletedCallback(discordpp::Client::MessageDeletedCallback cb);

    /// \brief Sets a callback to be invoked whenever a message is edited.
    ///
    /// Some messages sent from in game, as well as all messages sent from a connected user's
    /// Discord client can be edited and deleted in the Discord client. So it is valuable to
    /// implement support for this callback so that if a user edits or deletes a message in the
    /// Discord client, it is reflected in game as well.
    void SetMessageUpdatedCallback(discordpp::Client::MessageUpdatedCallback cb);

    /// \brief Sets whether chat messages are currently being shown in the game.
    ///
    /// If the user has the Discord desktop application open on the same machine as the game, then
    /// they will hear notifications from the Discord application, even though they are able to see
    /// those messages in game. So to avoid double-notifying users, you can call this function
    /// whenever the chat is shown or hidden to suppress those duplicate notifications.
    ///
    /// Keep in mind, if the game stops showing chat for a period of time, or the game loses focus
    /// because the user switches to a different app, it is important to call this function again so
    /// that the user's notifications get re-enabled in Discord during this time.
    void SetShowingChat(bool showingChat);
    /// @}

    /// @name Core
    /// @{

    /// \brief Adds a callback function to be invoked for each new log message generated by the SDK.
    ///
    /// This function explicitly excludes most logs for voice and webrtc activity since those are
    /// generally much noisier and you may want to pick a different log level for those. So it will
    /// instead include logs for things such as lobbies, relationships, presence, and
    /// authentication.
    ///
    /// We strongly recommend invoking this function immediately after constructing the Client
    /// object.
    void AddLogCallback(discordpp::Client::LogCallback callback,
                        discordpp::LoggingSeverity minSeverity);

    /// \brief Adds a callback function to be invoked for each new log message generated by the
    /// voice subsystem of the SDK, including the underlying webrtc infrastructure.
    ///
    /// We strongly recommend invoking this function immediately after constructing the Client
    /// object.
    void AddVoiceLogCallback(discordpp::Client::LogCallback callback,
                             discordpp::LoggingSeverity minSeverity);

    /// \brief Asynchronously connects the client to Discord.
    ///
    /// If a client is disconnecting, this will wait for the disconnect before reconnecting.
    /// You should use the Client::SetStatusChangedCallback and Client::GetStatus functions to
    /// receive updates on the client status. The Client is only safe to use once the status changes
    /// to Status::Ready.
    void Connect();

    /// \brief Asynchronously disconnects the client.
    ///
    /// You can leverage Client::SetStatusChangedCallback and Client::GetStatus to receive updates
    /// on the client status. It is fully disconnected when the status changes to
    /// Client::Status::Disconnected.
    void Disconnect();

    /// \brief Returns the current status of the client, see the Status enum for an explanation of
    /// the possible values.
    discordpp::Client::Status GetStatus() const;

    /// \brief This function is used to set the application ID for the client. This is used to
    /// identify the application to the Discord client. This is used for things like
    /// authentication, rich presence, and activity invites when *not* connected with
    /// Client::Connect. When calling Client::Connect, the application ID is set automatically
    void SetApplicationId(uint64_t applicationId);

    /// \brief Causes logs generated by the SDK to be written to disk in the specified directory.
    ///
    /// This function explicitly excludes most logs for voice and webrtc activity since those are
    /// generally much noisier and you may want to pick a different log level for those. So it will
    /// instead include logs for things such as lobbies, relationships, presence, and
    /// authentication. An empty path defaults to logging alongside the client library. A
    /// minSeverity = LoggingSeverity::None disables logging to a file (also the current default).
    /// The logs will be placed into a file called "discord.log" in the specified directory.
    /// Overwrites any existing discord.log file.
    ///
    /// We strongly recommend invoking this function immediately after constructing the Client
    /// object.
    ///
    /// Returns true if the log file was successfully opened, false otherwise.
    bool SetLogDir(std::string const& path, discordpp::LoggingSeverity minSeverity);

    /// \brief Sets a callback function to be invoked whenever the SDKs status changes.
    void SetStatusChangedCallback(discordpp::Client::OnStatusChanged cb);

    /// \brief Causes logs generated by the voice subsystem of the SDK to be written to disk in the
    /// specified directory.
    ///
    /// These logs will be in a file like discord-webrtc_0, and if they grow to big will be rotated
    /// and the number incremented. If the log files already exist the old ones will be renamed to
    /// discord-last-webrtc_0.
    ///
    /// An empty path defaults to logging alongside the client library.
    /// A minSeverity = LoggingSeverity::None disables logging to a file (also the current default).
    ///
    /// WARNING: This function MUST be invoked immediately after constructing the Client object!
    /// It will print out a warning if invoked too late.
    void SetVoiceLogDir(std::string const& path, discordpp::LoggingSeverity minSeverity);
    /// @}

    /// @name Lobbies
    /// @{

    /// \brief Joins the user to the specified lobby, creating one if it does not exist.
    ///
    /// The lobby is specified by the supplied string, which should be a hard to guess secret
    /// generated by the game. All users who join the lobby with the same secret will be placed in
    /// the same lobby.
    ///
    /// For exchanging the secret, we strongly encourage looking into the activity invite and rich
    /// presence systems which provide a way to include a secret string that only accepted party
    /// members are able to see.
    ///
    /// As with server created lobbies, client created lobbies auto-delete once they have been idle
    /// for a few minutes (which currently defaults to 5 minutes). A lobby is idle if no users are
    /// connected to it.
    ///
    /// This function shouldn't be used for long lived lobbies. The "secret" value expires after ~30
    /// days, at which point the existing lobby cannot be joined and a new one would be created
    /// instead.
    void CreateOrJoinLobby(std::string const& secret,
                           discordpp::Client::CreateOrJoinLobbyCallback callback);

    /// \brief Variant of Client::CreateOrJoinLobby that also accepts developer-supplied metadata.
    ///
    /// Metadata is just simple string key/value pairs.
    /// An example use case for this might be to the internal game ID of the user of each lobby so
    /// all members of the lobby can have a mapping of discord IDs to game IDs. Subsequent calls to
    /// CreateOrJoinLobby will overwrite the metadata for the lobby and member.
    void CreateOrJoinLobbyWithMetadata(
      std::string const& secret,
      std::unordered_map<std::string, std::string> const& lobbyMetadata,
      std::unordered_map<std::string, std::string> const& memberMetadata,
      discordpp::Client::CreateOrJoinLobbyCallback callback);

    /// \brief Fetches all of the channels that the current user can access in the given guild.
    ///
    /// The purpose of this is to power the channel linking flow for linking a Discord channel to an
    /// in-game lobby. So this function can be used to power a UI to let the user pick which channel
    /// to link to once they have picked a guild. See the docs on LobbyHandle for more information.
    void GetGuildChannels(uint64_t guildId, discordpp::Client::GetGuildChannelsCallback cb);

    /// \brief Returns a reference to the Discord lobby object for the given ID.
    std::optional<discordpp::LobbyHandle> GetLobbyHandle(uint64_t lobbyId) const;

    /// \brief Returns a list of all the lobbies that the user is a member of and the SDK has
    /// loaded.
    ///
    /// Lobbies are optimistically loaded when the SDK starts but in some cases may not be available
    /// immediately after the SDK status changes to Status::Ready.
    std::vector<uint64_t> GetLobbyIds() const;

    /// \brief Fetches all of the guilds (also known as Discord servers) that the current user is a
    /// member of.
    ///
    /// The purpose of this is to power the channel linking flow for linking a Discord channel
    /// to an in-game lobby. So this function can be used to power a UI to let the user which guild
    /// to link to. See the docs on LobbyHandle for more information.
    void GetUserGuilds(discordpp::Client::GetUserGuildsCallback cb);

    /// \brief Removes the current user from the specified lobby.
    ///
    /// Only lobbies that contain a "secret" can be left.
    /// In other words, only lobbies created with Client::CreateOrJoinLobby can be left.
    /// Lobbies created using the server API may not be manipulated by clients, so you must
    /// use the server API to remove them too.
    void LeaveLobby(uint64_t lobbyId, discordpp::Client::LeaveLobbyCallback callback);

    /// \brief Links the specified channel on Discord to the specified in-game lobby.
    ///
    /// Any message sent in one will be copied over to the other!
    /// See the docs on LobbyHandle for more information.
    void LinkChannelToLobby(uint64_t lobbyId,
                            uint64_t channelId,
                            discordpp::Client::LinkOrUnlinkChannelCallback callback);

    /// \brief Sets a callback to be invoked when a lobby "becomes available" to the client.
    ///
    /// A lobby can become available in a few situations:
    /// - A new lobby is created and the current user is a member of it
    /// - The current user is added to an existing lobby
    /// - A lobby recovers after a backend crash and is available once again
    ///
    /// This means that the LobbyCreated callback can be invoked more than once in a single session!
    /// Generally though it should never be invoked twice in a row. For example if a lobby crashes
    /// or a user is removed from the lobby, you should expect to have the LobbyDeleted callback
    /// invoked first.
    void SetLobbyCreatedCallback(discordpp::Client::LobbyCreatedCallback cb);

    /// \brief Sets a callback to be invoked when a lobby is no longer available.
    ///
    /// This callback can be invoked in a number of situations:
    /// - A lobby the user is a member of is deleted
    /// - The current user is removed from a lobby
    /// - There is a backend crash that causes the lobby to be unavailable for all users
    ///
    /// This means that this callback might be invoked even though the lobby still exists for other
    /// users!
    void SetLobbyDeletedCallback(discordpp::Client::LobbyDeletedCallback cb);

    /// \brief Sets a callback function to be invoked whenever a user is added to a lobby.
    ///
    /// This callback will not be invoked when the current user is added to a lobby, instead the
    /// LobbyCreated callback will be invoked. Additionally, the SDK separates membership in a lobby
    /// from whether a user is connected to a lobby. So a user being added does not necessarily mean
    /// they are online and in the lobby at that moment, just that they have permission to connect
    /// to that lobby.
    void SetLobbyMemberAddedCallback(discordpp::Client::LobbyMemberAddedCallback cb);

    /// \brief Sets a callback function to be invoked whenever a member of a lobby is removed and
    /// can no longer connect to it.
    ///
    /// This callback will not be invoked when the current user is removed from a lobby, instead
    /// LobbyDeleted callback will be invoked. Additionally this is not invoked when a user simply
    /// exits the game. That would cause the LobbyMemberUpdatedCallback to be invoked, and the
    /// LobbyMemberHandle object will indicate they are not connected now.
    void SetLobbyMemberRemovedCallback(discordpp::Client::LobbyMemberRemovedCallback cb);

    /// \brief Sets a callback function to be invoked whenever a member of a lobby is changed.
    ///
    /// This is invoked when:
    /// - The user connects or disconnects
    /// - The metadata of the member is changed
    void SetLobbyMemberUpdatedCallback(discordpp::Client::LobbyMemberUpdatedCallback cb);

    /// \brief Sets a callback to be invoked when a lobby is edited, for example if the lobby's
    /// metadata is changed.
    void SetLobbyUpdatedCallback(discordpp::Client::LobbyUpdatedCallback cb);

    /// \brief Removes any existing channel link from the specified lobby.
    ///
    /// See the docs on LobbyHandle for more information.
    /// A lobby can be unlinked by any user with the LobbyMemberFlags::CanLinkLobby flag, they do
    /// not need to have any permissions on the Discord channel in order to sever the in-game link.
    void UnlinkChannelFromLobby(uint64_t lobbyId,
                                discordpp::Client::LinkOrUnlinkChannelCallback callback);
    /// @}

    /// @name Presence
    /// @{

    /// \brief Accepts an activity invite that the current user has received.
    ///
    /// The given callback will be invoked with the join secret for the activity, which can be used
    /// to join the user to the game's internal party system for example.
    /// This join secret comes from the other user's rich presence activity.
    void AcceptActivityInvite(discordpp::ActivityInvite invite,
                              discordpp::Client::AcceptActivityInviteCallback cb);

    /// \brief Clears the right presence for the current user.
    void ClearRichPresence();

    /// \brief When a user accepts an activity invite for your game within the Discord client,
    /// Discord needs to know how to launch the game for that user. This function allows you to
    /// register a command that Discord will run to launch your game. You should invoke this when
    /// the SDK starts up so that if the user in the future tries to join from Discord the game will
    /// be able to be launched for them. Returns true if the command was successfully registered,
    /// false otherwise.
    ///
    /// On Windows and Linux, this command should be a path to an executable. It also supports any
    /// launch parameters that may be needed, like
    /// "C:\path\to my\game.exe" --full-screen --no-hax
    /// If you pass an empty string in for the command, the SDK will register the current running
    /// executable. To launch the game from a custom protocol like my-awesome-game://, pass that in
    /// as an argument of the executable that should be launched by that protocol. For example,
    /// "C:\path\to my\game.exe" my-awesome-game://
    ///
    /// On macOS, due to the way Discord registers executables, your game needs to be bundled for
    /// this command to work. That means it should be a .app. You can pass a custom protocol like
    /// my-awesome-game:// as the custom command, but *not* a path to an executable. If you pass an
    /// empty string in for the command, the SDK will register the current running bundle, if any.
    bool RegisterLaunchCommand(uint64_t applicationId, std::string command);

    /// \brief When a user accepts an activity invite for your game within the Discord client,
    /// Discord needs to know how to launch the game for that user. For steam games, this function
    /// allows you to indicate to Discord what the steam game ID is. You should invoke this when the
    /// SDK starts up so that if the user in the future tries to join from Discord the game will be
    /// able to be launched for them. Returns true if the command was successfully registered, false
    /// otherwise.
    bool RegisterLaunchSteamApplication(uint64_t applicationId, uint32_t steamAppId);

    /// \brief Sends a Discord activity invite to the specified user.
    ///
    /// The invite is sent as a message on Discord, which means it can be sent in the following
    /// situations:
    /// - Both users are online and in the game and have not blocked each other
    /// - Both users are friends with each other
    /// - Both users share a mutual Discord server and have previously DM'd each other on Discord
    ///
    /// You can optionally include some message content to include in the message containing the
    /// invite, but it's ok to pass an empty string too.
    void SendActivityInvite(uint64_t userId,
                            std::string const& content,
                            discordpp::Client::SendActivityInviteCallback cb);

    /// \brief Requests to join the activity of the specified user.
    ///
    /// This can be called whenever the target user has a rich presence activity for the current
    /// game and that activity has space for another user to join them.
    ///
    /// That user will basically receive an activity invite which they can accept or reject.
    void SendActivityJoinRequest(uint64_t userId, discordpp::Client::SendActivityInviteCallback cb);

    /// \brief When another user requests to join the current user's party, this function is called
    /// to to allow that user to join. Specifically this will send the original user an activity
    /// invite which they then need to accept again.
    void SendActivityJoinRequestReply(discordpp::ActivityInvite invite,
                                      discordpp::Client::SendActivityInviteCallback cb);

    /// \brief Sets a callback function that is invoked when the current user receives an activity
    /// invite from another user.
    ///
    /// These invites are always sent as messages, so the SDK is parsing these
    /// messages to look for invites and invokes this callback instead. The message create callback
    /// will not be invoked for these messages. The invite object contains all the necessary
    /// information to identity the invite, which you can later pass to
    /// Client::AcceptActivityInvite.
    void SetActivityInviteCreatedCallback(discordpp::Client::ActivityInviteCallback cb);

    /// \brief Sets a callback function that is invoked when an existing activity invite changes.
    /// Currently, the only thing that changes on an activity invite is its validity. If the sender
    /// goes offline or exits the party the receiver was invited to, the invite is no longer
    /// joinable. It is possible for an invalid invite to go from invalid to valid if the sender
    /// rejoins the activity.
    void SetActivityInviteUpdatedCallback(discordpp::Client::ActivityInviteCallback cb);

    /// \brief Sets a callback function that is invoked when the current user also has Discord
    /// running on their computer and they accept an activity invite in the Discord client.
    ///
    /// This callback is invoked with the join secret from the activity rich presence, which you can
    /// use to join them to the game's internal party system. See Activity for more information on
    /// invites.
    void SetActivityJoinCallback(discordpp::Client::ActivityJoinCallback cb);

    /// \brief Sets whether a user is online/invisible/idle/dnd on Discord.
    void SetOnlineStatus(discordpp::StatusType status,
                         discordpp::Client::UpdateStatusCallback callback);

    /// \brief Updates the rich presence for the current user.
    ///
    /// You should use rich presence so that other users on Discord know this user is playing a game
    /// and you can include some hints of what they are playing such as a character name or map
    /// name. Rich presence also enables Discord game invites to work too!
    ///
    /// Note: On Desktop, rich presence can be set before calling Client::Connect, but it will be
    /// cleared if the Client connects. When Client is not connected, this sets the rich presence in
    /// the current user's Discord client when available.
    ///
    /// See the docs on the Activity struct for more details.
    ///
    /// Note: The Activity object here is a partial object, fields such as name, and applicationId
    /// cannot be set and will be overwritten by the SDK. See
    /// https://discord.com/developers/docs/rich-presence/using-with-the-game-sdk#partial-activity-struct
    /// for more information.
    void UpdateRichPresence(discordpp::Activity activity,
                            discordpp::Client::UpdateRichPresenceCallback cb);
    /// @}

    /// @name Relationships
    /// @{

    /// \brief Accepts an incoming Discord friend request from the target user.
    ///
    /// Fails if the target user has not sent a Discord friend request to the current user, meaning
    /// that the Discord relationship type between the users must be
    /// RelationshipType::PendingIncoming.
    void AcceptDiscordFriendRequest(uint64_t userId,
                                    discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Accepts an incoming game friend request from the target user.
    ///
    /// Fails if the target user has not sent a game friend request to the current user, meaning
    /// that the game relationship type between the users must be RelationshipType::PendingIncoming.
    void AcceptGameFriendRequest(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Blocks the target user so that they cannot send the user friend or activity invites
    /// and cannot message them anymore.
    ///
    /// Blocking a user will also remove any existing relationship
    /// between the two users, and persists across games, so blocking a user in one game or on
    /// Discord will block them in all other games and on Discord as well.
    void BlockUser(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Cancels an outgoing Discord friend request to the target user.
    ///
    /// Fails if a Discord friend request has not been sent to the target user, meaning
    /// that the Discord relationship type between the users must be
    /// RelationshipType::PendingOutgoing.
    void CancelDiscordFriendRequest(uint64_t userId,
                                    discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Cancels an outgoing game friend request to the target user.
    ///
    /// Fails if a game friend request has not been sent to the target user, meaning
    /// that the game relationship type between the users must be RelationshipType::PendingOutgoing.
    void CancelGameFriendRequest(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Returns the RelationshipHandle that corresponds to the relationship between the
    /// current user and the given user.
    discordpp::RelationshipHandle GetRelationshipHandle(uint64_t userId) const;

    /// \brief Returns a list of all of the relationships the current user has with others,
    /// including all Discord relationships and all Game relationships for the current game.
    std::vector<discordpp::RelationshipHandle> GetRelationships() const;

    /// \brief Declines an incoming Discord friend request from the target user.
    ///
    /// Fails if the target user has not sent a Discord friend request to the current user, meaning
    /// that the Discord relationship type between the users must be
    /// RelationshipType::PendingIncoming.
    void RejectDiscordFriendRequest(uint64_t userId,
                                    discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Declines an incoming game friend request from the target user.
    ///
    /// Fails if the target user has not sent a game friend request to the current user, meaning
    /// that the game relationship type between the users must be RelationshipType::PendingIncoming.
    void RejectGameFriendRequest(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Removes any friendship between the current user and the target user. This function
    /// will remove BOTH any Discord friendship and any game friendship between the users.
    ///
    /// Fails if the target user is not currently a Discord OR game friend with the current user.
    void RemoveDiscordAndGameFriend(uint64_t userId,
                                    discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Removes any game friendship between the current user and the target user.
    ///
    /// Fails if the target user is not currently a game friend with the current user.
    void RemoveGameFriend(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Searches all of your friends by both username and display name, returning
    /// a list of all friends that match the search string.
    ///
    /// Under the hood uses the Levenshtein distance algorithm.
    std::vector<discordpp::UserHandle> SearchFriendsByUsername(std::string searchStr) const;

    /// \brief Sends a Discord friend request to the target user.
    ///
    /// The target user is identified by their Discord unique username (not their DisplayName).
    ///
    /// After the friend request is sent, each user will have a new Discord relationship created.
    /// For the current user the RelationshipHandle::DiscordRelationshipType will be
    /// RelationshipType::PendingOutgoing and for the target user it will be
    /// RelationshipType::PendingIncoming.
    ///
    /// If the current user already has received a Discord friend request from the target user
    /// (meaning RelationshipHandle::DiscordRelationshipType is RelationshipType::PendingIncoming),
    /// then the two users will become Discord friends.
    ///
    /// See RelationshipHandle for more information on the difference between Discord and Game
    /// relationships.
    void SendDiscordFriendRequest(std::string const& username,
                                  discordpp::Client::SendFriendRequestCallback cb);

    /// \brief Sends a Discord friend request to the target user.
    ///
    /// The target user is identified by their Discord ID.
    ///
    /// After the friend request is sent, each user will have a new Discord relationship created.
    /// For the current user the RelationshipHandle::DiscordRelationshipType will be
    /// RelationshipType::PendingOutgoing and for the target user it will be
    /// RelationshipType::PendingIncoming.
    ///
    /// If the current user already has received a Discord friend request from the target user
    /// (meaning RelationshipHandle::DiscordRelationshipType is RelationshipType::PendingIncoming),
    /// then the two users will become Discord friends.
    ///
    /// See RelationshipHandle for more information on the difference between Discord and Game
    /// relationships.
    void SendDiscordFriendRequestById(uint64_t userId,
                                      discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Sends (or accepts) a game friend request to the target user.
    ///
    /// The target user is identified by their Discord unique username (not their DisplayName).
    ///
    /// After the friend request is sent, each user will have a new game relationship created. For
    /// the current user the RelationshipHandle::GameRelationshipType will be
    /// RelationshipType::PendingOutgoing and for the target user it will be
    /// RelationshipType::PendingIncoming.
    ///
    /// If the current user already has received a game friend request from the target user
    /// (meaning RelationshipHandle::GameRelationshipType is RelationshipType::PendingIncoming),
    /// then the two users will become game friends.
    ///
    /// See RelationshipHandle for more information on the difference between Discord and Game
    /// relationships.
    void SendGameFriendRequest(std::string const& username,
                               discordpp::Client::SendFriendRequestCallback cb);

    /// \brief Sends (or accepts) a game friend request to the target user.
    ///
    /// The target user is identified by their Discord ID.
    ///
    /// After the friend request is sent, each user will have a new game relationship created. For
    /// the current user the RelationshipHandle::GameRelationshipType will be
    /// RelationshipType::PendingOutgoing and for the target user it will be
    /// RelationshipType::PendingIncoming.
    ///
    /// If the current user already has received a game friend request from the target user
    /// (meaning RelationshipHandle::GameRelationshipType is RelationshipType::PendingIncoming),
    /// then the two users will become game friends.
    ///
    /// See RelationshipHandle for more information on the difference between Discord and Game
    /// relationships.
    void SendGameFriendRequestById(uint64_t userId,
                                   discordpp::Client::UpdateRelationshipCallback cb);

    /// \brief Sets a callback to be invoked whenever a relationship for this user is established or
    /// changes type.
    ///
    /// This can be invoked when a user sends or accepts a friend invite or blocks a user for
    /// example.
    void SetRelationshipCreatedCallback(discordpp::Client::RelationshipCreatedCallback cb);

    /// \brief Sets a callback to be invoked whenever a relationship for this user is removed,
    /// such as when the user rejects a friend request or removes a friend.
    ///
    /// When a relationship is removed, Client::GetRelationshipHandle will
    /// return a relationship with the type set to RelationshipType::None.
    void SetRelationshipDeletedCallback(discordpp::Client::RelationshipDeletedCallback cb);

    /// \brief Unblocks the target user. Does not restore any old relationship between the users
    /// though.
    ///
    /// Fails if the target user is not currently blocked.
    void UnblockUser(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb);
    /// @}

    /// @name Users
    /// @{

    /// \brief Returns the user associated with the current client.
    ///
    /// Must not be called before the Client::GetStatus has changed to Status::Ready.
    /// If the client has disconnected, or is in the process of reconnecting, it will return the
    /// previous value of the user, even if the auth token has changed since then. Wait for
    /// client.GetStatus() to change to Ready before accessing it again.
    discordpp::UserHandle GetCurrentUser() const;

    /// \brief If the Discord app is running on the user's computer and the SDK establishes a
    /// connection to it, this function will return the user that is currently logged in to the
    /// Discord app.
    void GetDiscordClientConnectedUser(
      uint64_t applicationId,
      discordpp::Client::GetDiscordClientConnectedUserCallback callback) const;

    /// \brief Returns the UserHandle associated with the given user ID.
    ///
    /// It will not fetch a user from Discord's API if it is not available. Generally you can trust
    /// that users will be available for all relationships and for the authors of any messages
    /// received.
    std::optional<discordpp::UserHandle> GetUser(uint64_t userId) const;

    /// \brief The UserUpdatedCallback is invoked whenever *any* user the current session knows
    /// about changes, not just if the current user changes. For example if one of your Discord
    /// friends changes their name or avatar the UserUpdatedCallback will be invoked. It is also
    /// invoked when users come online, go offline, or start playing your game.
    void SetUserUpdatedCallback(discordpp::Client::UserUpdatedCallback cb);
    /// @}
};

/// \brief Convenience class that represents the state of a single Discord call in a lobby.
class CallInfoHandle {
    /// \cond
    mutable Discord_CallInfoHandle instance_{};
    DiscordObjectState state_ = DiscordObjectState::Invalid;
    /// \endcond

public:
    /// \cond
    Discord_CallInfoHandle* instance() const { return &instance_; }
    /// \endcond
    /// \cond
    explicit CallInfoHandle(Discord_CallInfoHandle instance, DiscordObjectState state);
    ~CallInfoHandle();
    /// \endcond
    /// Move constructor for CallInfoHandle
    CallInfoHandle(CallInfoHandle&& other) noexcept;
    /// Move assignment operator for CallInfoHandle
    CallInfoHandle& operator=(CallInfoHandle&& other) noexcept;
    /// Uninitialized instance of CallInfoHandle
    static const CallInfoHandle nullobj;
    /// Returns true if the instance contains a valid object
    operator bool() const { return state_ != DiscordObjectState::Invalid; }

    /// Copy constructor for CallInfoHandle
    CallInfoHandle(const CallInfoHandle& other);
    /// Copy assignment operator for CallInfoHandle
    CallInfoHandle& operator=(const CallInfoHandle& other);

    /// \cond
    void Drop();
    /// \endcond

    /// \brief Returns the lobby ID of the call.
    uint64_t ChannelId() const;

    /// \brief Returns a list of the user IDs of the participants in the call.
    std::vector<uint64_t> GetParticipants() const;

    /// \brief Accesses the voice state for a single user so you can know if they have muted or
    /// deafened themselves.
    std::optional<discordpp::VoiceStateHandle> GetVoiceStateHandle(uint64_t userId) const;

    /// \brief Returns the lobby ID of the call.
    uint64_t GuildId() const;
};
/// Converts a discordpp::ActivityActionTypes to a string.
inline const char* EnumToString(discordpp::ActivityActionTypes value)
{
    switch (value) {
    case discordpp::ActivityActionTypes::Join:
        return "Join";
    case discordpp::ActivityActionTypes::JoinRequest:
        return "JoinRequest";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::ActivityPartyPrivacy to a string.
inline const char* EnumToString(discordpp::ActivityPartyPrivacy value)
{
    switch (value) {
    case discordpp::ActivityPartyPrivacy::Private:
        return "Private";
    case discordpp::ActivityPartyPrivacy::Public:
        return "Public";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::ActivityTypes to a string.
inline const char* EnumToString(discordpp::ActivityTypes value)
{
    switch (value) {
    case discordpp::ActivityTypes::Playing:
        return "Playing";
    case discordpp::ActivityTypes::Streaming:
        return "Streaming";
    case discordpp::ActivityTypes::Listening:
        return "Listening";
    case discordpp::ActivityTypes::Watching:
        return "Watching";
    case discordpp::ActivityTypes::CustomStatus:
        return "CustomStatus";
    case discordpp::ActivityTypes::Competing:
        return "Competing";
    case discordpp::ActivityTypes::HangStatus:
        return "HangStatus";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::ActivityGamePlatforms to a string.
inline const char* EnumToString(discordpp::ActivityGamePlatforms value)
{
    switch (value) {
    case discordpp::ActivityGamePlatforms::Desktop:
        return "Desktop";
    case discordpp::ActivityGamePlatforms::Xbox:
        return "Xbox";
    case discordpp::ActivityGamePlatforms::Samsung:
        return "Samsung";
    case discordpp::ActivityGamePlatforms::IOS:
        return "IOS";
    case discordpp::ActivityGamePlatforms::Android:
        return "Android";
    case discordpp::ActivityGamePlatforms::Embedded:
        return "Embedded";
    case discordpp::ActivityGamePlatforms::PS4:
        return "PS4";
    case discordpp::ActivityGamePlatforms::PS5:
        return "PS5";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::ErrorType to a string.
inline const char* EnumToString(discordpp::ErrorType value)
{
    switch (value) {
    case discordpp::ErrorType::None:
        return "None";
    case discordpp::ErrorType::NetworkError:
        return "NetworkError";
    case discordpp::ErrorType::HTTPError:
        return "HTTPError";
    case discordpp::ErrorType::ClientNotReady:
        return "ClientNotReady";
    case discordpp::ErrorType::Disabled:
        return "Disabled";
    case discordpp::ErrorType::ClientDestroyed:
        return "ClientDestroyed";
    case discordpp::ErrorType::ValidationError:
        return "ValidationError";
    case discordpp::ErrorType::Aborted:
        return "Aborted";
    case discordpp::ErrorType::AuthorizationFailed:
        return "AuthorizationFailed";
    case discordpp::ErrorType::RPCError:
        return "RPCError";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::HttpStatusCode to a string.
inline const char* EnumToString(discordpp::HttpStatusCode value)
{
    switch (value) {
    case discordpp::HttpStatusCode::None:
        return "None";
    case discordpp::HttpStatusCode::Continue:
        return "Continue";
    case discordpp::HttpStatusCode::SwitchingProtocols:
        return "SwitchingProtocols";
    case discordpp::HttpStatusCode::Processing:
        return "Processing";
    case discordpp::HttpStatusCode::EarlyHints:
        return "EarlyHints";
    case discordpp::HttpStatusCode::Ok:
        return "Ok";
    case discordpp::HttpStatusCode::Created:
        return "Created";
    case discordpp::HttpStatusCode::Accepted:
        return "Accepted";
    case discordpp::HttpStatusCode::NonAuthoritativeInfo:
        return "NonAuthoritativeInfo";
    case discordpp::HttpStatusCode::NoContent:
        return "NoContent";
    case discordpp::HttpStatusCode::ResetContent:
        return "ResetContent";
    case discordpp::HttpStatusCode::PartialContent:
        return "PartialContent";
    case discordpp::HttpStatusCode::MultiStatus:
        return "MultiStatus";
    case discordpp::HttpStatusCode::AlreadyReported:
        return "AlreadyReported";
    case discordpp::HttpStatusCode::ImUsed:
        return "ImUsed";
    case discordpp::HttpStatusCode::MultipleChoices:
        return "MultipleChoices";
    case discordpp::HttpStatusCode::MovedPermanently:
        return "MovedPermanently";
    case discordpp::HttpStatusCode::Found:
        return "Found";
    case discordpp::HttpStatusCode::SeeOther:
        return "SeeOther";
    case discordpp::HttpStatusCode::NotModified:
        return "NotModified";
    case discordpp::HttpStatusCode::TemporaryRedirect:
        return "TemporaryRedirect";
    case discordpp::HttpStatusCode::PermanentRedirect:
        return "PermanentRedirect";
    case discordpp::HttpStatusCode::BadRequest:
        return "BadRequest";
    case discordpp::HttpStatusCode::Unauthorized:
        return "Unauthorized";
    case discordpp::HttpStatusCode::PaymentRequired:
        return "PaymentRequired";
    case discordpp::HttpStatusCode::Forbidden:
        return "Forbidden";
    case discordpp::HttpStatusCode::NotFound:
        return "NotFound";
    case discordpp::HttpStatusCode::MethodNotAllowed:
        return "MethodNotAllowed";
    case discordpp::HttpStatusCode::NotAcceptable:
        return "NotAcceptable";
    case discordpp::HttpStatusCode::ProxyAuthRequired:
        return "ProxyAuthRequired";
    case discordpp::HttpStatusCode::RequestTimeout:
        return "RequestTimeout";
    case discordpp::HttpStatusCode::Conflict:
        return "Conflict";
    case discordpp::HttpStatusCode::Gone:
        return "Gone";
    case discordpp::HttpStatusCode::LengthRequired:
        return "LengthRequired";
    case discordpp::HttpStatusCode::PreconditionFailed:
        return "PreconditionFailed";
    case discordpp::HttpStatusCode::PayloadTooLarge:
        return "PayloadTooLarge";
    case discordpp::HttpStatusCode::UriTooLong:
        return "UriTooLong";
    case discordpp::HttpStatusCode::UnsupportedMediaType:
        return "UnsupportedMediaType";
    case discordpp::HttpStatusCode::RangeNotSatisfiable:
        return "RangeNotSatisfiable";
    case discordpp::HttpStatusCode::ExpectationFailed:
        return "ExpectationFailed";
    case discordpp::HttpStatusCode::MisdirectedRequest:
        return "MisdirectedRequest";
    case discordpp::HttpStatusCode::UnprocessableEntity:
        return "UnprocessableEntity";
    case discordpp::HttpStatusCode::Locked:
        return "Locked";
    case discordpp::HttpStatusCode::FailedDependency:
        return "FailedDependency";
    case discordpp::HttpStatusCode::TooEarly:
        return "TooEarly";
    case discordpp::HttpStatusCode::UpgradeRequired:
        return "UpgradeRequired";
    case discordpp::HttpStatusCode::PreconditionRequired:
        return "PreconditionRequired";
    case discordpp::HttpStatusCode::TooManyRequests:
        return "TooManyRequests";
    case discordpp::HttpStatusCode::RequestHeaderFieldsTooLarge:
        return "RequestHeaderFieldsTooLarge";
    case discordpp::HttpStatusCode::InternalServerError:
        return "InternalServerError";
    case discordpp::HttpStatusCode::NotImplemented:
        return "NotImplemented";
    case discordpp::HttpStatusCode::BadGateway:
        return "BadGateway";
    case discordpp::HttpStatusCode::ServiceUnavailable:
        return "ServiceUnavailable";
    case discordpp::HttpStatusCode::GatewayTimeout:
        return "GatewayTimeout";
    case discordpp::HttpStatusCode::HttpVersionNotSupported:
        return "HttpVersionNotSupported";
    case discordpp::HttpStatusCode::VariantAlsoNegotiates:
        return "VariantAlsoNegotiates";
    case discordpp::HttpStatusCode::InsufficientStorage:
        return "InsufficientStorage";
    case discordpp::HttpStatusCode::LoopDetected:
        return "LoopDetected";
    case discordpp::HttpStatusCode::NotExtended:
        return "NotExtended";
    case discordpp::HttpStatusCode::NetworkAuthorizationRequired:
        return "NetworkAuthorizationRequired";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::AuthenticationCodeChallengeMethod to a string.
inline const char* EnumToString(discordpp::AuthenticationCodeChallengeMethod value)
{
    switch (value) {
    case discordpp::AuthenticationCodeChallengeMethod::S256:
        return "S256";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::AdditionalContentType to a string.
inline const char* EnumToString(discordpp::AdditionalContentType value)
{
    switch (value) {
    case discordpp::AdditionalContentType::Other:
        return "Other";
    case discordpp::AdditionalContentType::Attachment:
        return "Attachment";
    case discordpp::AdditionalContentType::Poll:
        return "Poll";
    case discordpp::AdditionalContentType::VoiceMessage:
        return "VoiceMessage";
    case discordpp::AdditionalContentType::Thread:
        return "Thread";
    case discordpp::AdditionalContentType::Embed:
        return "Embed";
    case discordpp::AdditionalContentType::Sticker:
        return "Sticker";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::Call::Error to a string.
inline const char* EnumToString(discordpp::Call::Error value)
{
    switch (value) {
    case discordpp::Call::Error::None:
        return "None";
    case discordpp::Call::Error::SignalingConnectionFailed:
        return "SignalingConnectionFailed";
    case discordpp::Call::Error::SignalingUnexpectedClose:
        return "SignalingUnexpectedClose";
    case discordpp::Call::Error::VoiceConnectionFailed:
        return "VoiceConnectionFailed";
    case discordpp::Call::Error::JoinTimeout:
        return "JoinTimeout";
    case discordpp::Call::Error::Forbidden:
        return "Forbidden";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::AudioModeType to a string.
inline const char* EnumToString(discordpp::AudioModeType value)
{
    switch (value) {
    case discordpp::AudioModeType::MODE_UNINIT:
        return "MODE_UNINIT";
    case discordpp::AudioModeType::MODE_VAD:
        return "MODE_VAD";
    case discordpp::AudioModeType::MODE_PTT:
        return "MODE_PTT";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::Call::Status to a string.
inline const char* EnumToString(discordpp::Call::Status value)
{
    switch (value) {
    case discordpp::Call::Status::Disconnected:
        return "Disconnected";
    case discordpp::Call::Status::Joining:
        return "Joining";
    case discordpp::Call::Status::Connecting:
        return "Connecting";
    case discordpp::Call::Status::SignalingConnected:
        return "SignalingConnected";
    case discordpp::Call::Status::Connected:
        return "Connected";
    case discordpp::Call::Status::Reconnecting:
        return "Reconnecting";
    case discordpp::Call::Status::Disconnecting:
        return "Disconnecting";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::ChannelType to a string.
inline const char* EnumToString(discordpp::ChannelType value)
{
    switch (value) {
    case discordpp::ChannelType::GuildText:
        return "GuildText";
    case discordpp::ChannelType::Dm:
        return "Dm";
    case discordpp::ChannelType::GuildVoice:
        return "GuildVoice";
    case discordpp::ChannelType::GroupDm:
        return "GroupDm";
    case discordpp::ChannelType::GuildCategory:
        return "GuildCategory";
    case discordpp::ChannelType::GuildNews:
        return "GuildNews";
    case discordpp::ChannelType::GuildStore:
        return "GuildStore";
    case discordpp::ChannelType::GuildNewsThread:
        return "GuildNewsThread";
    case discordpp::ChannelType::GuildPublicThread:
        return "GuildPublicThread";
    case discordpp::ChannelType::GuildPrivateThread:
        return "GuildPrivateThread";
    case discordpp::ChannelType::GuildStageVoice:
        return "GuildStageVoice";
    case discordpp::ChannelType::GuildDirectory:
        return "GuildDirectory";
    case discordpp::ChannelType::GuildForum:
        return "GuildForum";
    case discordpp::ChannelType::GuildMedia:
        return "GuildMedia";
    case discordpp::ChannelType::Lobby:
        return "Lobby";
    case discordpp::ChannelType::EphemeralDm:
        return "EphemeralDm";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::RelationshipType to a string.
inline const char* EnumToString(discordpp::RelationshipType value)
{
    switch (value) {
    case discordpp::RelationshipType::None:
        return "None";
    case discordpp::RelationshipType::Friend:
        return "Friend";
    case discordpp::RelationshipType::Blocked:
        return "Blocked";
    case discordpp::RelationshipType::PendingIncoming:
        return "PendingIncoming";
    case discordpp::RelationshipType::PendingOutgoing:
        return "PendingOutgoing";
    case discordpp::RelationshipType::Implicit:
        return "Implicit";
    case discordpp::RelationshipType::Suggestion:
        return "Suggestion";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::UserHandle::AvatarType to a string.
inline const char* EnumToString(discordpp::UserHandle::AvatarType value)
{
    switch (value) {
    case discordpp::UserHandle::AvatarType::Gif:
        return "Gif";
    case discordpp::UserHandle::AvatarType::Webp:
        return "Webp";
    case discordpp::UserHandle::AvatarType::Png:
        return "Png";
    case discordpp::UserHandle::AvatarType::Jpeg:
        return "Jpeg";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::StatusType to a string.
inline const char* EnumToString(discordpp::StatusType value)
{
    switch (value) {
    case discordpp::StatusType::Online:
        return "Online";
    case discordpp::StatusType::Offline:
        return "Offline";
    case discordpp::StatusType::Blocked:
        return "Blocked";
    case discordpp::StatusType::Idle:
        return "Idle";
    case discordpp::StatusType::Dnd:
        return "Dnd";
    case discordpp::StatusType::Invisible:
        return "Invisible";
    case discordpp::StatusType::Streaming:
        return "Streaming";
    case discordpp::StatusType::Unknown:
        return "Unknown";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::DisclosureTypes to a string.
inline const char* EnumToString(discordpp::DisclosureTypes value)
{
    switch (value) {
    case discordpp::DisclosureTypes::MessageDataVisibleOnDiscord:
        return "MessageDataVisibleOnDiscord";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::Client::Error to a string.
inline const char* EnumToString(discordpp::Client::Error value)
{
    switch (value) {
    case discordpp::Client::Error::None:
        return "None";
    case discordpp::Client::Error::ConnectionFailed:
        return "ConnectionFailed";
    case discordpp::Client::Error::UnexpectedClose:
        return "UnexpectedClose";
    case discordpp::Client::Error::ConnectionCanceled:
        return "ConnectionCanceled";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::Client::Status to a string.
inline const char* EnumToString(discordpp::Client::Status value)
{
    switch (value) {
    case discordpp::Client::Status::Disconnected:
        return "Disconnected";
    case discordpp::Client::Status::Connecting:
        return "Connecting";
    case discordpp::Client::Status::Connected:
        return "Connected";
    case discordpp::Client::Status::Ready:
        return "Ready";
    case discordpp::Client::Status::Reconnecting:
        return "Reconnecting";
    case discordpp::Client::Status::Disconnecting:
        return "Disconnecting";
    case discordpp::Client::Status::HttpWait:
        return "HttpWait";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::Client::Thread to a string.
inline const char* EnumToString(discordpp::Client::Thread value)
{
    switch (value) {
    case discordpp::Client::Thread::Client:
        return "Client";
    case discordpp::Client::Thread::Voice:
        return "Voice";
    case discordpp::Client::Thread::Network:
        return "Network";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::AuthorizationTokenType to a string.
inline const char* EnumToString(discordpp::AuthorizationTokenType value)
{
    switch (value) {
    case discordpp::AuthorizationTokenType::User:
        return "User";
    case discordpp::AuthorizationTokenType::Bearer:
        return "Bearer";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::AuthenticationExternalAuthType to a string.
inline const char* EnumToString(discordpp::AuthenticationExternalAuthType value)
{
    switch (value) {
    case discordpp::AuthenticationExternalAuthType::OIDC:
        return "OIDC";
    case discordpp::AuthenticationExternalAuthType::EpicOnlineServicesAccessToken:
        return "EpicOnlineServicesAccessToken";
    case discordpp::AuthenticationExternalAuthType::EpicOnlineServicesIdToken:
        return "EpicOnlineServicesIdToken";
    case discordpp::AuthenticationExternalAuthType::SteamSessionTicket:
        return "SteamSessionTicket";
    case discordpp::AuthenticationExternalAuthType::UnityServicesIdToken:
        return "UnityServicesIdToken";
    default:
        return "unknown";
    }
}
/// Converts a discordpp::LoggingSeverity to a string.
inline const char* EnumToString(discordpp::LoggingSeverity value)
{
    switch (value) {
    case discordpp::LoggingSeverity::Verbose:
        return "Verbose";
    case discordpp::LoggingSeverity::Info:
        return "Info";
    case discordpp::LoggingSeverity::Warning:
        return "Warning";
    case discordpp::LoggingSeverity::Error:
        return "Error";
    case discordpp::LoggingSeverity::None:
        return "None";
    default:
        return "unknown";
    }
}
} // namespace discordpp
#endif // DISCORD_HEADER_DISCORDPP_H_
#ifdef DISCORDPP_IMPLEMENTATION
#undef DISCORDPP_IMPLEMENTATION
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
namespace discordpp {
std::function<void(std::function<void()>)> s_synchronizationContext;

inline bool HasSynchronizationContext()
{
    return !!s_synchronizationContext;
}

inline void PostTask(std::function<void()> task)
{
    assert(s_synchronizationContext);
    s_synchronizationContext(std::move(task));
}

void SetSynchronizationContext(std::function<void(std::function<void()>)> executor)
{
    s_synchronizationContext = std::move(executor);
}

template <typename T>
struct TDelegateUserData {
    T delegate;
    TDelegateUserData(T delegate)
      : delegate{delegate}
    {
    }

    static void Free(void* ptr) { delete reinterpret_cast<TDelegateUserData*>(ptr); }

    static T& Get(void* userData)
    {
        return reinterpret_cast<TDelegateUserData*>(userData)->delegate;
    }
};

struct ConvertedProperties {
    ConvertedProperties(std::unordered_map<std::string, std::string> const& PropertyMap)
    {
        Properties.size = PropertyMap.size();
        Properties.keys = reinterpret_cast<Discord_String*>(
          Discord_Alloc(Properties.size * sizeof(Discord_String)));
        Properties.values = reinterpret_cast<Discord_String*>(
          Discord_Alloc(Properties.size * sizeof(Discord_String)));
        size_t i = 0;
        for (auto& pair : PropertyMap) {
            Properties.keys[i] = AllocateString(pair.first);
            Properties.values[i] = AllocateString(pair.second);
            ++i;
        }
    }
    ~ConvertedProperties() { Discord_FreeProperties(Properties); }
    Discord_Properties Properties{};

private:
    Discord_String AllocateString(std::string const& str)
    {
        Discord_String result;
        result.ptr = reinterpret_cast<uint8_t*>(Discord_Alloc(str.size()));
        result.size = str.size();
        std::memcpy(result.ptr, str.data(), result.size);
        return result;
    }
};

std::unordered_map<std::string, std::string> ConvertReturnedProperties(
  Discord_Properties const& Properties)
{
    std::unordered_map<std::string, std::string> result;
    for (size_t i = 0; i < Properties.size; ++i) {
        std::string key(reinterpret_cast<char*>(Properties.keys[i].ptr), Properties.keys[i].size);
        std::string value(reinterpret_cast<char*>(Properties.values[i].ptr),
                          Properties.values[i].size);
        result.emplace(std::move(key), std::move(value));
    }
    return result;
}
const ActivityInvite ActivityInvite::nullobj{{}, DiscordObjectState::Invalid};
ActivityInvite::~ActivityInvite()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
ActivityInvite::ActivityInvite(ActivityInvite&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
ActivityInvite& ActivityInvite::operator=(ActivityInvite&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
ActivityInvite::ActivityInvite(const ActivityInvite& rhs)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (rhs.state_ == DiscordObjectState::Owned) {
        Discord_ActivityInvite_Clone(&instance_, rhs.instance());

        state_ = DiscordObjectState::Owned;
    }
}
ActivityInvite& ActivityInvite::operator=(const ActivityInvite& rhs)
{
    if (this != &rhs) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (rhs.state_ == DiscordObjectState::Owned) {
            Discord_ActivityInvite_Clone(&instance_, rhs.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
ActivityInvite::ActivityInvite(Discord_ActivityInvite instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
ActivityInvite::ActivityInvite()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_ActivityInvite_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void ActivityInvite::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_ActivityInvite_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t ActivityInvite::SenderId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_ActivityInvite_SenderId(&instance_);
    return returnValue__;
}
void ActivityInvite::SetSenderId(uint64_t SenderId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityInvite_SetSenderId(&instance_, SenderId);
}
uint64_t ActivityInvite::ChannelId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_ActivityInvite_ChannelId(&instance_);
    return returnValue__;
}
void ActivityInvite::SetChannelId(uint64_t ChannelId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityInvite_SetChannelId(&instance_, ChannelId);
}
uint64_t ActivityInvite::MessageId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_ActivityInvite_MessageId(&instance_);
    return returnValue__;
}
void ActivityInvite::SetMessageId(uint64_t MessageId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityInvite_SetMessageId(&instance_, MessageId);
}
discordpp::ActivityActionTypes ActivityInvite::Type() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityActionTypes returnValue__;
    returnValue__ = Discord_ActivityInvite_Type(&instance_);
    return static_cast<discordpp::ActivityActionTypes>(returnValue__);
}
void ActivityInvite::SetType(discordpp::ActivityActionTypes Type)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityInvite_SetType(&instance_, static_cast<Discord_ActivityActionTypes>(Type));
}
uint64_t ActivityInvite::ApplicationId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_ActivityInvite_ApplicationId(&instance_);
    return returnValue__;
}
void ActivityInvite::SetApplicationId(uint64_t ApplicationId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityInvite_SetApplicationId(&instance_, ApplicationId);
}
std::string ActivityInvite::PartyId() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_ActivityInvite_PartyId(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ActivityInvite::SetPartyId(std::string PartyId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String PartyId__str{(uint8_t*)(PartyId.data()), PartyId.size()};
    Discord_ActivityInvite_SetPartyId(&instance_, PartyId__str);
}
std::string ActivityInvite::SessionId() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_ActivityInvite_SessionId(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ActivityInvite::SetSessionId(std::string SessionId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String SessionId__str{(uint8_t*)(SessionId.data()), SessionId.size()};
    Discord_ActivityInvite_SetSessionId(&instance_, SessionId__str);
}
bool ActivityInvite::IsValid() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_ActivityInvite_IsValid(&instance_);
    return returnValue__;
}
void ActivityInvite::SetIsValid(bool IsValid)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityInvite_SetIsValid(&instance_, IsValid);
}
const ActivityAssets ActivityAssets::nullobj{{}, DiscordObjectState::Invalid};
ActivityAssets::~ActivityAssets()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
ActivityAssets::ActivityAssets(ActivityAssets&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
ActivityAssets& ActivityAssets::operator=(ActivityAssets&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
ActivityAssets::ActivityAssets(const ActivityAssets& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_ActivityAssets_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
ActivityAssets& ActivityAssets::operator=(const ActivityAssets& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_ActivityAssets_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
ActivityAssets::ActivityAssets(Discord_ActivityAssets instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
ActivityAssets::ActivityAssets()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_ActivityAssets_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void ActivityAssets::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_ActivityAssets_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::optional<std::string> ActivityAssets::LargeImage() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_ActivityAssets_LargeImage(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ActivityAssets::SetLargeImage(std::optional<std::string> LargeImage)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String LargeImage__str{};
    if (LargeImage.has_value()) {
        LargeImage__str.ptr = reinterpret_cast<uint8_t*>(LargeImage->data());
        LargeImage__str.size = LargeImage->size();
    }
    Discord_ActivityAssets_SetLargeImage(&instance_,
                                         (LargeImage.has_value() ? &LargeImage__str : nullptr));
}
std::optional<std::string> ActivityAssets::LargeText() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_ActivityAssets_LargeText(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ActivityAssets::SetLargeText(std::optional<std::string> LargeText)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String LargeText__str{};
    if (LargeText.has_value()) {
        LargeText__str.ptr = reinterpret_cast<uint8_t*>(LargeText->data());
        LargeText__str.size = LargeText->size();
    }
    Discord_ActivityAssets_SetLargeText(&instance_,
                                        (LargeText.has_value() ? &LargeText__str : nullptr));
}
std::optional<std::string> ActivityAssets::SmallImage() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_ActivityAssets_SmallImage(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ActivityAssets::SetSmallImage(std::optional<std::string> SmallImage)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String SmallImage__str{};
    if (SmallImage.has_value()) {
        SmallImage__str.ptr = reinterpret_cast<uint8_t*>(SmallImage->data());
        SmallImage__str.size = SmallImage->size();
    }
    Discord_ActivityAssets_SetSmallImage(&instance_,
                                         (SmallImage.has_value() ? &SmallImage__str : nullptr));
}
std::optional<std::string> ActivityAssets::SmallText() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_ActivityAssets_SmallText(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ActivityAssets::SetSmallText(std::optional<std::string> SmallText)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String SmallText__str{};
    if (SmallText.has_value()) {
        SmallText__str.ptr = reinterpret_cast<uint8_t*>(SmallText->data());
        SmallText__str.size = SmallText->size();
    }
    Discord_ActivityAssets_SetSmallText(&instance_,
                                        (SmallText.has_value() ? &SmallText__str : nullptr));
}
const ActivityTimestamps ActivityTimestamps::nullobj{{}, DiscordObjectState::Invalid};
ActivityTimestamps::~ActivityTimestamps()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
ActivityTimestamps::ActivityTimestamps(ActivityTimestamps&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
ActivityTimestamps& ActivityTimestamps::operator=(ActivityTimestamps&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
ActivityTimestamps::ActivityTimestamps(const ActivityTimestamps& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_ActivityTimestamps_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
ActivityTimestamps& ActivityTimestamps::operator=(const ActivityTimestamps& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_ActivityTimestamps_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
ActivityTimestamps::ActivityTimestamps(Discord_ActivityTimestamps instance,
                                       DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
ActivityTimestamps::ActivityTimestamps()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_ActivityTimestamps_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void ActivityTimestamps::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_ActivityTimestamps_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t ActivityTimestamps::Start() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_ActivityTimestamps_Start(&instance_);
    return returnValue__;
}
void ActivityTimestamps::SetStart(uint64_t Start)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityTimestamps_SetStart(&instance_, Start);
}
uint64_t ActivityTimestamps::End() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_ActivityTimestamps_End(&instance_);
    return returnValue__;
}
void ActivityTimestamps::SetEnd(uint64_t End)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityTimestamps_SetEnd(&instance_, End);
}
const ActivityParty ActivityParty::nullobj{{}, DiscordObjectState::Invalid};
ActivityParty::~ActivityParty()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
ActivityParty::ActivityParty(ActivityParty&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
ActivityParty& ActivityParty::operator=(ActivityParty&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
ActivityParty::ActivityParty(const ActivityParty& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_ActivityParty_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
ActivityParty& ActivityParty::operator=(const ActivityParty& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_ActivityParty_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
ActivityParty::ActivityParty(Discord_ActivityParty instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
ActivityParty::ActivityParty()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_ActivityParty_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void ActivityParty::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_ActivityParty_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::string ActivityParty::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_ActivityParty_Id(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ActivityParty::SetId(std::string Id)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Id__str{(uint8_t*)(Id.data()), Id.size()};
    Discord_ActivityParty_SetId(&instance_, Id__str);
}
int32_t ActivityParty::CurrentSize() const
{
    assert(state_ == DiscordObjectState::Owned);
    int32_t returnValue__;
    returnValue__ = Discord_ActivityParty_CurrentSize(&instance_);
    return returnValue__;
}
void ActivityParty::SetCurrentSize(int32_t CurrentSize)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityParty_SetCurrentSize(&instance_, CurrentSize);
}
int32_t ActivityParty::MaxSize() const
{
    assert(state_ == DiscordObjectState::Owned);
    int32_t returnValue__;
    returnValue__ = Discord_ActivityParty_MaxSize(&instance_);
    return returnValue__;
}
void ActivityParty::SetMaxSize(int32_t MaxSize)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityParty_SetMaxSize(&instance_, MaxSize);
}
discordpp::ActivityPartyPrivacy ActivityParty::Privacy() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityPartyPrivacy returnValue__;
    returnValue__ = Discord_ActivityParty_Privacy(&instance_);
    return static_cast<discordpp::ActivityPartyPrivacy>(returnValue__);
}
void ActivityParty::SetPrivacy(discordpp::ActivityPartyPrivacy Privacy)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityParty_SetPrivacy(&instance_,
                                     static_cast<Discord_ActivityPartyPrivacy>(Privacy));
}
const ActivitySecrets ActivitySecrets::nullobj{{}, DiscordObjectState::Invalid};
ActivitySecrets::~ActivitySecrets()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
ActivitySecrets::ActivitySecrets(ActivitySecrets&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
ActivitySecrets& ActivitySecrets::operator=(ActivitySecrets&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
ActivitySecrets::ActivitySecrets(const ActivitySecrets& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_ActivitySecrets_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
ActivitySecrets& ActivitySecrets::operator=(const ActivitySecrets& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_ActivitySecrets_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
ActivitySecrets::ActivitySecrets(Discord_ActivitySecrets instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
ActivitySecrets::ActivitySecrets()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_ActivitySecrets_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void ActivitySecrets::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_ActivitySecrets_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::string ActivitySecrets::Join() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_ActivitySecrets_Join(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ActivitySecrets::SetJoin(std::string Join)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Join__str{(uint8_t*)(Join.data()), Join.size()};
    Discord_ActivitySecrets_SetJoin(&instance_, Join__str);
}
const Activity Activity::nullobj{{}, DiscordObjectState::Invalid};
Activity::~Activity()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
Activity::Activity(Activity&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
Activity& Activity::operator=(Activity&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
Activity::Activity(const Activity& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_Activity_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
Activity& Activity::operator=(const Activity& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_Activity_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
Activity::Activity(Discord_Activity instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
Activity::Activity()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_Activity_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void Activity::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_Activity_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
bool Activity::Equals(discordpp::Activity other) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Activity_Equals(&instance_, other.instance());
    return returnValue__;
}
std::string Activity::Name() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_Activity_Name(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void Activity::SetName(std::string Name)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Name__str{(uint8_t*)(Name.data()), Name.size()};
    Discord_Activity_SetName(&instance_, Name__str);
}
discordpp::ActivityTypes Activity::Type() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityTypes returnValue__;
    returnValue__ = Discord_Activity_Type(&instance_);
    return static_cast<discordpp::ActivityTypes>(returnValue__);
}
void Activity::SetType(discordpp::ActivityTypes Type)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Activity_SetType(&instance_, static_cast<Discord_ActivityTypes>(Type));
}
std::optional<std::string> Activity::State() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_Activity_State(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void Activity::SetState(std::optional<std::string> State)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String State__str{};
    if (State.has_value()) {
        State__str.ptr = reinterpret_cast<uint8_t*>(State->data());
        State__str.size = State->size();
    }
    Discord_Activity_SetState(&instance_, (State.has_value() ? &State__str : nullptr));
}
std::optional<std::string> Activity::Details() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_Activity_Details(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void Activity::SetDetails(std::optional<std::string> Details)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Details__str{};
    if (Details.has_value()) {
        Details__str.ptr = reinterpret_cast<uint8_t*>(Details->data());
        Details__str.size = Details->size();
    }
    Discord_Activity_SetDetails(&instance_, (Details.has_value() ? &Details__str : nullptr));
}
std::optional<uint64_t> Activity::ApplicationId() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    uint64_t returnValue__;
    returnIsNonNull__ = Discord_Activity_ApplicationId(&instance_, &returnValue__);
    if (!returnIsNonNull__) {
        return std::nullopt;
    }
    return returnValue__;
}
void Activity::SetApplicationId(std::optional<uint64_t> ApplicationId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Activity_SetApplicationId(&instance_,
                                      (ApplicationId.has_value() ? &*ApplicationId : nullptr));
}
std::optional<discordpp::ActivityAssets> Activity::Assets() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_ActivityAssets returnValueNative__;
    returnIsNonNull__ = Discord_Activity_Assets(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::ActivityAssets returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void Activity::SetAssets(std::optional<discordpp::ActivityAssets> Assets)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Activity_SetAssets(&instance_, (Assets.has_value() ? Assets->instance() : nullptr));
}
std::optional<discordpp::ActivityTimestamps> Activity::Timestamps() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_ActivityTimestamps returnValueNative__;
    returnIsNonNull__ = Discord_Activity_Timestamps(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::ActivityTimestamps returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void Activity::SetTimestamps(std::optional<discordpp::ActivityTimestamps> Timestamps)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Activity_SetTimestamps(&instance_,
                                   (Timestamps.has_value() ? Timestamps->instance() : nullptr));
}
std::optional<discordpp::ActivityParty> Activity::Party() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_ActivityParty returnValueNative__;
    returnIsNonNull__ = Discord_Activity_Party(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::ActivityParty returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void Activity::SetParty(std::optional<discordpp::ActivityParty> Party)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Activity_SetParty(&instance_, (Party.has_value() ? Party->instance() : nullptr));
}
std::optional<discordpp::ActivitySecrets> Activity::Secrets() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_ActivitySecrets returnValueNative__;
    returnIsNonNull__ = Discord_Activity_Secrets(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::ActivitySecrets returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void Activity::SetSecrets(std::optional<discordpp::ActivitySecrets> Secrets)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Activity_SetSecrets(&instance_, (Secrets.has_value() ? Secrets->instance() : nullptr));
}
discordpp::ActivityGamePlatforms Activity::SupportedPlatforms() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ActivityGamePlatforms returnValue__;
    returnValue__ = Discord_Activity_SupportedPlatforms(&instance_);
    return static_cast<discordpp::ActivityGamePlatforms>(returnValue__);
}
void Activity::SetSupportedPlatforms(discordpp::ActivityGamePlatforms SupportedPlatforms)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Activity_SetSupportedPlatforms(
      &instance_, static_cast<Discord_ActivityGamePlatforms>(SupportedPlatforms));
}
const ClientResult ClientResult::nullobj{{}, DiscordObjectState::Invalid};
ClientResult::~ClientResult()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
ClientResult::ClientResult(ClientResult&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
ClientResult& ClientResult::operator=(ClientResult&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
ClientResult::ClientResult(const ClientResult& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_ClientResult_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
ClientResult& ClientResult::operator=(const ClientResult& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_ClientResult_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
ClientResult::ClientResult(Discord_ClientResult instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void ClientResult::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_ClientResult_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::string ClientResult::ToString() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_ClientResult_ToString(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
discordpp::ErrorType ClientResult::Type() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ErrorType returnValue__;
    returnValue__ = Discord_ClientResult_Type(&instance_);
    return static_cast<discordpp::ErrorType>(returnValue__);
}
void ClientResult::SetType(discordpp::ErrorType Type)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ClientResult_SetType(&instance_, static_cast<Discord_ErrorType>(Type));
}
std::string ClientResult::Error() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_ClientResult_Error(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ClientResult::SetError(std::string Error)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Error__str{(uint8_t*)(Error.data()), Error.size()};
    Discord_ClientResult_SetError(&instance_, Error__str);
}
int32_t ClientResult::ErrorCode() const
{
    assert(state_ == DiscordObjectState::Owned);
    int32_t returnValue__;
    returnValue__ = Discord_ClientResult_ErrorCode(&instance_);
    return returnValue__;
}
void ClientResult::SetErrorCode(int32_t ErrorCode)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ClientResult_SetErrorCode(&instance_, ErrorCode);
}
discordpp::HttpStatusCode ClientResult::Status() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_HttpStatusCode returnValue__;
    returnValue__ = Discord_ClientResult_Status(&instance_);
    return static_cast<discordpp::HttpStatusCode>(returnValue__);
}
void ClientResult::SetStatus(discordpp::HttpStatusCode Status)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ClientResult_SetStatus(&instance_, static_cast<Discord_HttpStatusCode>(Status));
}
std::string ClientResult::ResponseBody() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_ClientResult_ResponseBody(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void ClientResult::SetResponseBody(std::string ResponseBody)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String ResponseBody__str{(uint8_t*)(ResponseBody.data()), ResponseBody.size()};
    Discord_ClientResult_SetResponseBody(&instance_, ResponseBody__str);
}
bool ClientResult::Successful() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_ClientResult_Successful(&instance_);
    return returnValue__;
}
void ClientResult::SetSuccessful(bool Successful)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ClientResult_SetSuccessful(&instance_, Successful);
}
bool ClientResult::Retryable() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_ClientResult_Retryable(&instance_);
    return returnValue__;
}
void ClientResult::SetRetryable(bool Retryable)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ClientResult_SetRetryable(&instance_, Retryable);
}
float ClientResult::RetryAfter() const
{
    assert(state_ == DiscordObjectState::Owned);
    float returnValue__;
    returnValue__ = Discord_ClientResult_RetryAfter(&instance_);
    return returnValue__;
}
void ClientResult::SetRetryAfter(float RetryAfter)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ClientResult_SetRetryAfter(&instance_, RetryAfter);
}
const AuthorizationCodeChallenge AuthorizationCodeChallenge::nullobj{{},
                                                                     DiscordObjectState::Invalid};
AuthorizationCodeChallenge::~AuthorizationCodeChallenge()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
AuthorizationCodeChallenge::AuthorizationCodeChallenge(AuthorizationCodeChallenge&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
AuthorizationCodeChallenge& AuthorizationCodeChallenge::operator=(
  AuthorizationCodeChallenge&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
AuthorizationCodeChallenge::AuthorizationCodeChallenge(const AuthorizationCodeChallenge& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_AuthorizationCodeChallenge_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
AuthorizationCodeChallenge& AuthorizationCodeChallenge::operator=(
  const AuthorizationCodeChallenge& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_AuthorizationCodeChallenge_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
AuthorizationCodeChallenge::AuthorizationCodeChallenge(Discord_AuthorizationCodeChallenge instance,
                                                       DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
AuthorizationCodeChallenge::AuthorizationCodeChallenge()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_AuthorizationCodeChallenge_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void AuthorizationCodeChallenge::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_AuthorizationCodeChallenge_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
discordpp::AuthenticationCodeChallengeMethod AuthorizationCodeChallenge::Method() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AuthenticationCodeChallengeMethod returnValue__;
    returnValue__ = Discord_AuthorizationCodeChallenge_Method(&instance_);
    return static_cast<discordpp::AuthenticationCodeChallengeMethod>(returnValue__);
}
void AuthorizationCodeChallenge::SetMethod(discordpp::AuthenticationCodeChallengeMethod Method)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AuthorizationCodeChallenge_SetMethod(
      &instance_, static_cast<Discord_AuthenticationCodeChallengeMethod>(Method));
}
std::string AuthorizationCodeChallenge::Challenge() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_AuthorizationCodeChallenge_Challenge(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void AuthorizationCodeChallenge::SetChallenge(std::string Challenge)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Challenge__str{(uint8_t*)(Challenge.data()), Challenge.size()};
    Discord_AuthorizationCodeChallenge_SetChallenge(&instance_, Challenge__str);
}
const AuthorizationCodeVerifier AuthorizationCodeVerifier::nullobj{{}, DiscordObjectState::Invalid};
AuthorizationCodeVerifier::~AuthorizationCodeVerifier()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
AuthorizationCodeVerifier::AuthorizationCodeVerifier(AuthorizationCodeVerifier&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
AuthorizationCodeVerifier& AuthorizationCodeVerifier::operator=(
  AuthorizationCodeVerifier&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
AuthorizationCodeVerifier::AuthorizationCodeVerifier(const AuthorizationCodeVerifier& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_AuthorizationCodeVerifier_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
AuthorizationCodeVerifier& AuthorizationCodeVerifier::operator=(
  const AuthorizationCodeVerifier& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_AuthorizationCodeVerifier_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
AuthorizationCodeVerifier::AuthorizationCodeVerifier(Discord_AuthorizationCodeVerifier instance,
                                                     DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void AuthorizationCodeVerifier::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_AuthorizationCodeVerifier_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
discordpp::AuthorizationCodeChallenge AuthorizationCodeVerifier::Challenge() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AuthorizationCodeChallenge returnValueNative__{};
    Discord_AuthorizationCodeVerifier_Challenge(&instance_, &returnValueNative__);
    discordpp::AuthorizationCodeChallenge returnValue__(returnValueNative__,
                                                        DiscordObjectState::Owned);
    return returnValue__;
}
void AuthorizationCodeVerifier::SetChallenge(discordpp::AuthorizationCodeChallenge Challenge)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AuthorizationCodeVerifier_SetChallenge(&instance_, Challenge.instance());
}
std::string AuthorizationCodeVerifier::Verifier() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_AuthorizationCodeVerifier_Verifier(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void AuthorizationCodeVerifier::SetVerifier(std::string Verifier)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Verifier__str{(uint8_t*)(Verifier.data()), Verifier.size()};
    Discord_AuthorizationCodeVerifier_SetVerifier(&instance_, Verifier__str);
}
const AuthorizationArgs AuthorizationArgs::nullobj{{}, DiscordObjectState::Invalid};
AuthorizationArgs::~AuthorizationArgs()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
AuthorizationArgs::AuthorizationArgs(AuthorizationArgs&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
AuthorizationArgs& AuthorizationArgs::operator=(AuthorizationArgs&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
AuthorizationArgs::AuthorizationArgs(const AuthorizationArgs& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_AuthorizationArgs_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
AuthorizationArgs& AuthorizationArgs::operator=(const AuthorizationArgs& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_AuthorizationArgs_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
AuthorizationArgs::AuthorizationArgs(Discord_AuthorizationArgs instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
AuthorizationArgs::AuthorizationArgs()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_AuthorizationArgs_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void AuthorizationArgs::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_AuthorizationArgs_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t AuthorizationArgs::ClientId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_AuthorizationArgs_ClientId(&instance_);
    return returnValue__;
}
void AuthorizationArgs::SetClientId(uint64_t ClientId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AuthorizationArgs_SetClientId(&instance_, ClientId);
}
std::string AuthorizationArgs::Scopes() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_AuthorizationArgs_Scopes(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void AuthorizationArgs::SetScopes(std::string Scopes)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Scopes__str{(uint8_t*)(Scopes.data()), Scopes.size()};
    Discord_AuthorizationArgs_SetScopes(&instance_, Scopes__str);
}
std::optional<std::string> AuthorizationArgs::State() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_AuthorizationArgs_State(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void AuthorizationArgs::SetState(std::optional<std::string> State)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String State__str{};
    if (State.has_value()) {
        State__str.ptr = reinterpret_cast<uint8_t*>(State->data());
        State__str.size = State->size();
    }
    Discord_AuthorizationArgs_SetState(&instance_, (State.has_value() ? &State__str : nullptr));
}
std::optional<std::string> AuthorizationArgs::Nonce() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_AuthorizationArgs_Nonce(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void AuthorizationArgs::SetNonce(std::optional<std::string> Nonce)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Nonce__str{};
    if (Nonce.has_value()) {
        Nonce__str.ptr = reinterpret_cast<uint8_t*>(Nonce->data());
        Nonce__str.size = Nonce->size();
    }
    Discord_AuthorizationArgs_SetNonce(&instance_, (Nonce.has_value() ? &Nonce__str : nullptr));
}
std::optional<discordpp::AuthorizationCodeChallenge> AuthorizationArgs::CodeChallenge() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_AuthorizationCodeChallenge returnValueNative__;
    returnIsNonNull__ = Discord_AuthorizationArgs_CodeChallenge(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::AuthorizationCodeChallenge returnValue__(returnValueNative__,
                                                        DiscordObjectState::Owned);
    return returnValue__;
}
void AuthorizationArgs::SetCodeChallenge(
  std::optional<discordpp::AuthorizationCodeChallenge> CodeChallenge)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AuthorizationArgs_SetCodeChallenge(
      &instance_, (CodeChallenge.has_value() ? CodeChallenge->instance() : nullptr));
}
const DeviceAuthorizationArgs DeviceAuthorizationArgs::nullobj{{}, DiscordObjectState::Invalid};
DeviceAuthorizationArgs::~DeviceAuthorizationArgs()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
DeviceAuthorizationArgs::DeviceAuthorizationArgs(DeviceAuthorizationArgs&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
DeviceAuthorizationArgs& DeviceAuthorizationArgs::operator=(
  DeviceAuthorizationArgs&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
DeviceAuthorizationArgs::DeviceAuthorizationArgs(const DeviceAuthorizationArgs& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_DeviceAuthorizationArgs_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
DeviceAuthorizationArgs& DeviceAuthorizationArgs::operator=(const DeviceAuthorizationArgs& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_DeviceAuthorizationArgs_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
DeviceAuthorizationArgs::DeviceAuthorizationArgs(Discord_DeviceAuthorizationArgs instance,
                                                 DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
DeviceAuthorizationArgs::DeviceAuthorizationArgs()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_DeviceAuthorizationArgs_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void DeviceAuthorizationArgs::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_DeviceAuthorizationArgs_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t DeviceAuthorizationArgs::ClientId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_DeviceAuthorizationArgs_ClientId(&instance_);
    return returnValue__;
}
void DeviceAuthorizationArgs::SetClientId(uint64_t ClientId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_DeviceAuthorizationArgs_SetClientId(&instance_, ClientId);
}
std::string DeviceAuthorizationArgs::Scopes() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_DeviceAuthorizationArgs_Scopes(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void DeviceAuthorizationArgs::SetScopes(std::string Scopes)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Scopes__str{(uint8_t*)(Scopes.data()), Scopes.size()};
    Discord_DeviceAuthorizationArgs_SetScopes(&instance_, Scopes__str);
}
const VoiceStateHandle VoiceStateHandle::nullobj{{}, DiscordObjectState::Invalid};
VoiceStateHandle::~VoiceStateHandle()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
VoiceStateHandle::VoiceStateHandle(VoiceStateHandle&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
VoiceStateHandle& VoiceStateHandle::operator=(VoiceStateHandle&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
VoiceStateHandle::VoiceStateHandle(const VoiceStateHandle& other)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (other.state_ == DiscordObjectState::Owned) {
        Discord_VoiceStateHandle_Clone(&instance_, other.instance());

        state_ = DiscordObjectState::Owned;
    }
}
VoiceStateHandle& VoiceStateHandle::operator=(const VoiceStateHandle& other)
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (other.state_ == DiscordObjectState::Owned) {
            Discord_VoiceStateHandle_Clone(&instance_, other.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
VoiceStateHandle::VoiceStateHandle(Discord_VoiceStateHandle instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void VoiceStateHandle::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_VoiceStateHandle_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
bool VoiceStateHandle::SelfDeaf() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_VoiceStateHandle_SelfDeaf(&instance_);
    return returnValue__;
}
bool VoiceStateHandle::SelfMute() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_VoiceStateHandle_SelfMute(&instance_);
    return returnValue__;
}
const VADThresholdSettings VADThresholdSettings::nullobj{{}, DiscordObjectState::Invalid};
VADThresholdSettings::~VADThresholdSettings()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
VADThresholdSettings::VADThresholdSettings(VADThresholdSettings&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
VADThresholdSettings& VADThresholdSettings::operator=(VADThresholdSettings&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
VADThresholdSettings::VADThresholdSettings(Discord_VADThresholdSettings instance,
                                           DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void VADThresholdSettings::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_VADThresholdSettings_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
float VADThresholdSettings::VadThreshold() const
{
    assert(state_ == DiscordObjectState::Owned);
    float returnValue__;
    returnValue__ = Discord_VADThresholdSettings_VadThreshold(&instance_);
    return returnValue__;
}
void VADThresholdSettings::SetVadThreshold(float VadThreshold)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_VADThresholdSettings_SetVadThreshold(&instance_, VadThreshold);
}
bool VADThresholdSettings::Automatic() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_VADThresholdSettings_Automatic(&instance_);
    return returnValue__;
}
void VADThresholdSettings::SetAutomatic(bool Automatic)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_VADThresholdSettings_SetAutomatic(&instance_, Automatic);
}
const Call Call::nullobj{{}, DiscordObjectState::Invalid};
Call::~Call()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
Call::Call(Call&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
Call& Call::operator=(Call&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
Call::Call(const Call& other)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (other.state_ == DiscordObjectState::Owned) {
        Discord_Call_Clone(&instance_, other.instance());

        state_ = DiscordObjectState::Owned;
    }
}
Call& Call::operator=(const Call& other)
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (other.state_ == DiscordObjectState::Owned) {
            Discord_Call_Clone(&instance_, other.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
Call::Call(Discord_Call instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void Call::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_Call_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::string Call::ErrorToString(discordpp::Call::Error type)
{
    Discord_String returnValueNative__;
    Discord_Call_ErrorToString(static_cast<Discord_Call_Error>(type), &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
discordpp::AudioModeType Call::GetAudioMode()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AudioModeType returnValue__;
    returnValue__ = Discord_Call_GetAudioMode(&instance_);
    return static_cast<discordpp::AudioModeType>(returnValue__);
}
uint64_t Call::GetChannelId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_Call_GetChannelId(&instance_);
    return returnValue__;
}
uint64_t Call::GetGuildId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_Call_GetGuildId(&instance_);
    return returnValue__;
}
bool Call::GetLocalMute(uint64_t userId)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Call_GetLocalMute(&instance_, userId);
    return returnValue__;
}
std::vector<uint64_t> Call::GetParticipants() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_UInt64Span returnValueNative__;
    Discord_Call_GetParticipants(&instance_, &returnValueNative__);
    std::vector<uint64_t> returnValue__(returnValueNative__.ptr,
                                        returnValueNative__.ptr + returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
float Call::GetParticipantVolume(uint64_t userId)
{
    assert(state_ == DiscordObjectState::Owned);
    float returnValue__;
    returnValue__ = Discord_Call_GetParticipantVolume(&instance_, userId);
    return returnValue__;
}
bool Call::GetPTTActive()
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Call_GetPTTActive(&instance_);
    return returnValue__;
}
uint32_t Call::GetPTTReleaseDelay()
{
    assert(state_ == DiscordObjectState::Owned);
    uint32_t returnValue__;
    returnValue__ = Discord_Call_GetPTTReleaseDelay(&instance_);
    return returnValue__;
}
bool Call::GetSelfDeaf()
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Call_GetSelfDeaf(&instance_);
    return returnValue__;
}
bool Call::GetSelfMute()
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Call_GetSelfMute(&instance_);
    return returnValue__;
}
discordpp::Call::Status Call::GetStatus() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_Status returnValue__;
    returnValue__ = Discord_Call_GetStatus(&instance_);
    return static_cast<discordpp::Call::Status>(returnValue__);
}
discordpp::VADThresholdSettings Call::GetVADThreshold() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_VADThresholdSettings returnValueNative__{};
    Discord_Call_GetVADThreshold(&instance_, &returnValueNative__);
    discordpp::VADThresholdSettings returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::optional<discordpp::VoiceStateHandle> Call::GetVoiceStateHandle(uint64_t userId) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_VoiceStateHandle returnValueNative__;
    returnIsNonNull__ = Discord_Call_GetVoiceStateHandle(&instance_, userId, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::VoiceStateHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void Call::SetAudioMode(discordpp::AudioModeType audioMode)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_SetAudioMode(&instance_, static_cast<Discord_AudioModeType>(audioMode));
}
void Call::SetLocalMute(uint64_t userId, bool mute)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_SetLocalMute(&instance_, userId, mute);
}
void Call::SetOnVoiceStateChangedCallback(discordpp::Call::OnVoiceStateChanged cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Call_OnVoiceStateChanged cb__native = [](auto userId, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate(userId);
    };
    Discord_Call_SetOnVoiceStateChangedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Call::SetParticipantChangedCallback(discordpp::Call::OnParticipantChanged cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Call_OnParticipantChanged cb__native = [](auto userId, auto added, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate(userId, added);
    };
    Discord_Call_SetParticipantChangedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Call::SetParticipantVolume(uint64_t userId, float volume)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_SetParticipantVolume(&instance_, userId, volume);
}
void Call::SetPTTActive(bool active)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_SetPTTActive(&instance_, active);
}
void Call::SetPTTReleaseDelay(uint32_t releaseDelayMs)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_SetPTTReleaseDelay(&instance_, releaseDelayMs);
}
void Call::SetSelfDeaf(bool deaf)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_SetSelfDeaf(&instance_, deaf);
}
void Call::SetSelfMute(bool mute)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_SetSelfMute(&instance_, mute);
}
void Call::SetSpeakingStatusChangedCallback(discordpp::Call::OnSpeakingStatusChanged cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Call_OnSpeakingStatusChanged cb__native =
      [](auto userId, auto isPlayingSound, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(userId, isPlayingSound);
      };
    Discord_Call_SetSpeakingStatusChangedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Call::SetStatusChangedCallback(discordpp::Call::OnStatusChanged cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Call_OnStatusChanged cb__native =
      [](auto status, auto error, auto errorDetail, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(static_cast<discordpp::Call::Status>(status),
                                    static_cast<discordpp::Call::Error>(error),
                                    errorDetail);
      };
    Discord_Call_SetStatusChangedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Call::SetVADThreshold(bool automatic, float threshold)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Call_SetVADThreshold(&instance_, automatic, threshold);
}
std::string Call::StatusToString(discordpp::Call::Status type)
{
    Discord_String returnValueNative__;
    Discord_Call_StatusToString(static_cast<Discord_Call_Status>(type), &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
const ChannelHandle ChannelHandle::nullobj{{}, DiscordObjectState::Invalid};
ChannelHandle::~ChannelHandle()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
ChannelHandle::ChannelHandle(ChannelHandle&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
ChannelHandle& ChannelHandle::operator=(ChannelHandle&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
ChannelHandle::ChannelHandle(const ChannelHandle& other)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (other.state_ == DiscordObjectState::Owned) {
        Discord_ChannelHandle_Clone(&instance_, other.instance());

        state_ = DiscordObjectState::Owned;
    }
}
ChannelHandle& ChannelHandle::operator=(const ChannelHandle& other)
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (other.state_ == DiscordObjectState::Owned) {
            Discord_ChannelHandle_Clone(&instance_, other.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
ChannelHandle::ChannelHandle(Discord_ChannelHandle instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void ChannelHandle::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_ChannelHandle_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t ChannelHandle::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_ChannelHandle_Id(&instance_);
    return returnValue__;
}
std::string ChannelHandle::Name() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_ChannelHandle_Name(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::vector<uint64_t> ChannelHandle::Recipients() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_UInt64Span returnValueNative__;
    Discord_ChannelHandle_Recipients(&instance_, &returnValueNative__);
    std::vector<uint64_t> returnValue__(returnValueNative__.ptr,
                                        returnValueNative__.ptr + returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
discordpp::ChannelType ChannelHandle::Type() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_ChannelType returnValue__;
    returnValue__ = Discord_ChannelHandle_Type(&instance_);
    return static_cast<discordpp::ChannelType>(returnValue__);
}
const GuildMinimal GuildMinimal::nullobj{{}, DiscordObjectState::Invalid};
GuildMinimal::~GuildMinimal()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
GuildMinimal::GuildMinimal(GuildMinimal&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
GuildMinimal& GuildMinimal::operator=(GuildMinimal&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
GuildMinimal::GuildMinimal(const GuildMinimal& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_GuildMinimal_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
GuildMinimal& GuildMinimal::operator=(const GuildMinimal& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_GuildMinimal_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
GuildMinimal::GuildMinimal(Discord_GuildMinimal instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void GuildMinimal::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_GuildMinimal_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t GuildMinimal::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_GuildMinimal_Id(&instance_);
    return returnValue__;
}
void GuildMinimal::SetId(uint64_t Id)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_GuildMinimal_SetId(&instance_, Id);
}
std::string GuildMinimal::Name() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_GuildMinimal_Name(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void GuildMinimal::SetName(std::string Name)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Name__str{(uint8_t*)(Name.data()), Name.size()};
    Discord_GuildMinimal_SetName(&instance_, Name__str);
}
const GuildChannel GuildChannel::nullobj{{}, DiscordObjectState::Invalid};
GuildChannel::~GuildChannel()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
GuildChannel::GuildChannel(GuildChannel&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
GuildChannel& GuildChannel::operator=(GuildChannel&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
GuildChannel::GuildChannel(const GuildChannel& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_GuildChannel_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
GuildChannel& GuildChannel::operator=(const GuildChannel& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_GuildChannel_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
GuildChannel::GuildChannel(Discord_GuildChannel instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void GuildChannel::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_GuildChannel_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t GuildChannel::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_GuildChannel_Id(&instance_);
    return returnValue__;
}
void GuildChannel::SetId(uint64_t Id)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_GuildChannel_SetId(&instance_, Id);
}
std::string GuildChannel::Name() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_GuildChannel_Name(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void GuildChannel::SetName(std::string Name)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Name__str{(uint8_t*)(Name.data()), Name.size()};
    Discord_GuildChannel_SetName(&instance_, Name__str);
}
bool GuildChannel::IsLinkable() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_GuildChannel_IsLinkable(&instance_);
    return returnValue__;
}
void GuildChannel::SetIsLinkable(bool IsLinkable)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_GuildChannel_SetIsLinkable(&instance_, IsLinkable);
}
bool GuildChannel::IsViewableAndWriteableByAllMembers() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_GuildChannel_IsViewableAndWriteableByAllMembers(&instance_);
    return returnValue__;
}
void GuildChannel::SetIsViewableAndWriteableByAllMembers(bool IsViewableAndWriteableByAllMembers)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_GuildChannel_SetIsViewableAndWriteableByAllMembers(&instance_,
                                                               IsViewableAndWriteableByAllMembers);
}
std::optional<discordpp::LinkedLobby> GuildChannel::LinkedLobby() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_LinkedLobby returnValueNative__;
    returnIsNonNull__ = Discord_GuildChannel_LinkedLobby(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::LinkedLobby returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void GuildChannel::SetLinkedLobby(std::optional<discordpp::LinkedLobby> LinkedLobby)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_GuildChannel_SetLinkedLobby(
      &instance_, (LinkedLobby.has_value() ? LinkedLobby->instance() : nullptr));
}
const LinkedLobby LinkedLobby::nullobj{{}, DiscordObjectState::Invalid};
LinkedLobby::~LinkedLobby()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
LinkedLobby::LinkedLobby(LinkedLobby&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
LinkedLobby& LinkedLobby::operator=(LinkedLobby&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
LinkedLobby::LinkedLobby(const LinkedLobby& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_LinkedLobby_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
LinkedLobby& LinkedLobby::operator=(const LinkedLobby& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_LinkedLobby_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
LinkedLobby::LinkedLobby(Discord_LinkedLobby instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
LinkedLobby::LinkedLobby()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_LinkedLobby_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void LinkedLobby::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_LinkedLobby_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t LinkedLobby::ApplicationId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_LinkedLobby_ApplicationId(&instance_);
    return returnValue__;
}
void LinkedLobby::SetApplicationId(uint64_t ApplicationId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_LinkedLobby_SetApplicationId(&instance_, ApplicationId);
}
uint64_t LinkedLobby::LobbyId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_LinkedLobby_LobbyId(&instance_);
    return returnValue__;
}
void LinkedLobby::SetLobbyId(uint64_t LobbyId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_LinkedLobby_SetLobbyId(&instance_, LobbyId);
}
const LinkedChannel LinkedChannel::nullobj{{}, DiscordObjectState::Invalid};
LinkedChannel::~LinkedChannel()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
LinkedChannel::LinkedChannel(LinkedChannel&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
LinkedChannel& LinkedChannel::operator=(LinkedChannel&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
LinkedChannel::LinkedChannel(const LinkedChannel& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_LinkedChannel_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
LinkedChannel& LinkedChannel::operator=(const LinkedChannel& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_LinkedChannel_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
LinkedChannel::LinkedChannel(Discord_LinkedChannel instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void LinkedChannel::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_LinkedChannel_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t LinkedChannel::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_LinkedChannel_Id(&instance_);
    return returnValue__;
}
void LinkedChannel::SetId(uint64_t Id)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_LinkedChannel_SetId(&instance_, Id);
}
std::string LinkedChannel::Name() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_LinkedChannel_Name(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void LinkedChannel::SetName(std::string Name)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Name__str{(uint8_t*)(Name.data()), Name.size()};
    Discord_LinkedChannel_SetName(&instance_, Name__str);
}
uint64_t LinkedChannel::GuildId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_LinkedChannel_GuildId(&instance_);
    return returnValue__;
}
void LinkedChannel::SetGuildId(uint64_t GuildId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_LinkedChannel_SetGuildId(&instance_, GuildId);
}
const RelationshipHandle RelationshipHandle::nullobj{{}, DiscordObjectState::Invalid};
RelationshipHandle::~RelationshipHandle()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
RelationshipHandle::RelationshipHandle(RelationshipHandle&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
RelationshipHandle& RelationshipHandle::operator=(RelationshipHandle&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
RelationshipHandle::RelationshipHandle(const RelationshipHandle& other)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (other.state_ == DiscordObjectState::Owned) {
        Discord_RelationshipHandle_Clone(&instance_, other.instance());

        state_ = DiscordObjectState::Owned;
    }
}
RelationshipHandle& RelationshipHandle::operator=(const RelationshipHandle& other)
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (other.state_ == DiscordObjectState::Owned) {
            Discord_RelationshipHandle_Clone(&instance_, other.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
RelationshipHandle::RelationshipHandle(Discord_RelationshipHandle instance,
                                       DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void RelationshipHandle::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_RelationshipHandle_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
discordpp::RelationshipType RelationshipHandle::DiscordRelationshipType() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_RelationshipType returnValue__;
    returnValue__ = Discord_RelationshipHandle_DiscordRelationshipType(&instance_);
    return static_cast<discordpp::RelationshipType>(returnValue__);
}
discordpp::RelationshipType RelationshipHandle::GameRelationshipType() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_RelationshipType returnValue__;
    returnValue__ = Discord_RelationshipHandle_GameRelationshipType(&instance_);
    return static_cast<discordpp::RelationshipType>(returnValue__);
}
uint64_t RelationshipHandle::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_RelationshipHandle_Id(&instance_);
    return returnValue__;
}
std::optional<discordpp::UserHandle> RelationshipHandle::User() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_UserHandle returnValueNative__;
    returnIsNonNull__ = Discord_RelationshipHandle_User(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::UserHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
const UserHandle UserHandle::nullobj{{}, DiscordObjectState::Invalid};
UserHandle::~UserHandle()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
UserHandle::UserHandle(UserHandle&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
UserHandle& UserHandle::operator=(UserHandle&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
UserHandle::UserHandle(const UserHandle& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_UserHandle_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
UserHandle& UserHandle::operator=(const UserHandle& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_UserHandle_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
UserHandle::UserHandle(Discord_UserHandle instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void UserHandle::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_UserHandle_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::optional<std::string> UserHandle::Avatar() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_UserHandle_Avatar(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::string UserHandle::AvatarTypeToString(discordpp::UserHandle::AvatarType type)
{
    Discord_String returnValueNative__;
    Discord_UserHandle_AvatarTypeToString(static_cast<Discord_UserHandle_AvatarType>(type),
                                          &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::string UserHandle::AvatarUrl(discordpp::UserHandle::AvatarType animatedType,
                                  discordpp::UserHandle::AvatarType staticType) const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_UserHandle_AvatarUrl(&instance_,
                                 static_cast<Discord_UserHandle_AvatarType>(animatedType),
                                 static_cast<Discord_UserHandle_AvatarType>(staticType),
                                 &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::string UserHandle::DisplayName() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_UserHandle_DisplayName(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::optional<discordpp::Activity> UserHandle::GameActivity() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_Activity returnValueNative__;
    returnIsNonNull__ = Discord_UserHandle_GameActivity(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::Activity returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::optional<std::string> UserHandle::GlobalName() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_UserHandle_GlobalName(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
uint64_t UserHandle::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_UserHandle_Id(&instance_);
    return returnValue__;
}
bool UserHandle::IsProvisional() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_UserHandle_IsProvisional(&instance_);
    return returnValue__;
}
discordpp::RelationshipHandle UserHandle::Relationship() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_RelationshipHandle returnValueNative__{};
    Discord_UserHandle_Relationship(&instance_, &returnValueNative__);
    discordpp::RelationshipHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
discordpp::StatusType UserHandle::Status() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_StatusType returnValue__;
    returnValue__ = Discord_UserHandle_Status(&instance_);
    return static_cast<discordpp::StatusType>(returnValue__);
}
std::string UserHandle::Username() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_UserHandle_Username(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
const LobbyMemberHandle LobbyMemberHandle::nullobj{{}, DiscordObjectState::Invalid};
LobbyMemberHandle::~LobbyMemberHandle()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
LobbyMemberHandle::LobbyMemberHandle(LobbyMemberHandle&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
LobbyMemberHandle& LobbyMemberHandle::operator=(LobbyMemberHandle&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
LobbyMemberHandle::LobbyMemberHandle(const LobbyMemberHandle& other)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (other.state_ == DiscordObjectState::Owned) {
        Discord_LobbyMemberHandle_Clone(&instance_, other.instance());

        state_ = DiscordObjectState::Owned;
    }
}
LobbyMemberHandle& LobbyMemberHandle::operator=(const LobbyMemberHandle& other)
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (other.state_ == DiscordObjectState::Owned) {
            Discord_LobbyMemberHandle_Clone(&instance_, other.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
LobbyMemberHandle::LobbyMemberHandle(Discord_LobbyMemberHandle instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void LobbyMemberHandle::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_LobbyMemberHandle_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
bool LobbyMemberHandle::CanLinkLobby() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_LobbyMemberHandle_CanLinkLobby(&instance_);
    return returnValue__;
}
bool LobbyMemberHandle::Connected() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_LobbyMemberHandle_Connected(&instance_);
    return returnValue__;
}
uint64_t LobbyMemberHandle::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_LobbyMemberHandle_Id(&instance_);
    return returnValue__;
}
std::unordered_map<std::string, std::string> LobbyMemberHandle::Metadata() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Properties returnValueNative__;
    Discord_LobbyMemberHandle_Metadata(&instance_, &returnValueNative__);
    std::unordered_map<std::string, std::string> returnValue__ =
      ConvertReturnedProperties(returnValueNative__);
    Discord_FreeProperties(returnValueNative__);
    return returnValue__;
}
std::optional<discordpp::UserHandle> LobbyMemberHandle::User() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_UserHandle returnValueNative__;
    returnIsNonNull__ = Discord_LobbyMemberHandle_User(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::UserHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
const LobbyHandle LobbyHandle::nullobj{{}, DiscordObjectState::Invalid};
LobbyHandle::~LobbyHandle()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
LobbyHandle::LobbyHandle(LobbyHandle&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
LobbyHandle& LobbyHandle::operator=(LobbyHandle&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
LobbyHandle::LobbyHandle(const LobbyHandle& other)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (other.state_ == DiscordObjectState::Owned) {
        Discord_LobbyHandle_Clone(&instance_, other.instance());

        state_ = DiscordObjectState::Owned;
    }
}
LobbyHandle& LobbyHandle::operator=(const LobbyHandle& other)
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (other.state_ == DiscordObjectState::Owned) {
            Discord_LobbyHandle_Clone(&instance_, other.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
LobbyHandle::LobbyHandle(Discord_LobbyHandle instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void LobbyHandle::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_LobbyHandle_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::optional<discordpp::CallInfoHandle> LobbyHandle::GetCallInfoHandle() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_CallInfoHandle returnValueNative__;
    returnIsNonNull__ = Discord_LobbyHandle_GetCallInfoHandle(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::CallInfoHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::optional<discordpp::LobbyMemberHandle> LobbyHandle::GetLobbyMemberHandle(
  uint64_t memberId) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_LobbyMemberHandle returnValueNative__;
    returnIsNonNull__ =
      Discord_LobbyHandle_GetLobbyMemberHandle(&instance_, memberId, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::LobbyMemberHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
uint64_t LobbyHandle::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_LobbyHandle_Id(&instance_);
    return returnValue__;
}
std::optional<discordpp::LinkedChannel> LobbyHandle::LinkedChannel() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_LinkedChannel returnValueNative__;
    returnIsNonNull__ = Discord_LobbyHandle_LinkedChannel(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::LinkedChannel returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::vector<uint64_t> LobbyHandle::LobbyMemberIds() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_UInt64Span returnValueNative__;
    Discord_LobbyHandle_LobbyMemberIds(&instance_, &returnValueNative__);
    std::vector<uint64_t> returnValue__(returnValueNative__.ptr,
                                        returnValueNative__.ptr + returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::vector<discordpp::LobbyMemberHandle> LobbyHandle::LobbyMembers() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_LobbyMemberHandleSpan returnValueNative__;
    Discord_LobbyHandle_LobbyMembers(&instance_, &returnValueNative__);
    std::vector<discordpp::LobbyMemberHandle> returnValue__;
    returnValue__.reserve(returnValueNative__.size);
    for (size_t i__ = 0; i__ < returnValueNative__.size; ++i__) {
        returnValue__.emplace_back(returnValueNative__.ptr[i__], DiscordObjectState::Owned);
    }
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::unordered_map<std::string, std::string> LobbyHandle::Metadata() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Properties returnValueNative__;
    Discord_LobbyHandle_Metadata(&instance_, &returnValueNative__);
    std::unordered_map<std::string, std::string> returnValue__ =
      ConvertReturnedProperties(returnValueNative__);
    Discord_FreeProperties(returnValueNative__);
    return returnValue__;
}
const AdditionalContent AdditionalContent::nullobj{{}, DiscordObjectState::Invalid};
AdditionalContent::~AdditionalContent()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
AdditionalContent::AdditionalContent(AdditionalContent&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
AdditionalContent& AdditionalContent::operator=(AdditionalContent&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
AdditionalContent::AdditionalContent(const AdditionalContent& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_AdditionalContent_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
AdditionalContent& AdditionalContent::operator=(const AdditionalContent& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_AdditionalContent_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
AdditionalContent::AdditionalContent(Discord_AdditionalContent instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
AdditionalContent::AdditionalContent()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_AdditionalContent_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
void AdditionalContent::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_AdditionalContent_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
bool AdditionalContent::Equals(discordpp::AdditionalContent rhs) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_AdditionalContent_Equals(&instance_, rhs.instance());
    return returnValue__;
}
std::string AdditionalContent::TypeToString(discordpp::AdditionalContentType type)
{
    Discord_String returnValueNative__;
    Discord_AdditionalContent_TypeToString(static_cast<Discord_AdditionalContentType>(type),
                                           &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
discordpp::AdditionalContentType AdditionalContent::Type() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AdditionalContentType returnValue__;
    returnValue__ = Discord_AdditionalContent_Type(&instance_);
    return static_cast<discordpp::AdditionalContentType>(returnValue__);
}
void AdditionalContent::SetType(discordpp::AdditionalContentType Type)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AdditionalContent_SetType(&instance_, static_cast<Discord_AdditionalContentType>(Type));
}
std::optional<std::string> AdditionalContent::Title() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_String returnValueNative__;
    returnIsNonNull__ = Discord_AdditionalContent_Title(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void AdditionalContent::SetTitle(std::optional<std::string> Title)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Title__str{};
    if (Title.has_value()) {
        Title__str.ptr = reinterpret_cast<uint8_t*>(Title->data());
        Title__str.size = Title->size();
    }
    Discord_AdditionalContent_SetTitle(&instance_, (Title.has_value() ? &Title__str : nullptr));
}
uint8_t AdditionalContent::Count() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint8_t returnValue__;
    returnValue__ = Discord_AdditionalContent_Count(&instance_);
    return returnValue__;
}
void AdditionalContent::SetCount(uint8_t Count)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AdditionalContent_SetCount(&instance_, Count);
}
const MessageHandle MessageHandle::nullobj{{}, DiscordObjectState::Invalid};
MessageHandle::~MessageHandle()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
MessageHandle::MessageHandle(MessageHandle&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
MessageHandle& MessageHandle::operator=(MessageHandle&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
MessageHandle::MessageHandle(const MessageHandle& other)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (other.state_ == DiscordObjectState::Owned) {
        Discord_MessageHandle_Clone(&instance_, other.instance());

        state_ = DiscordObjectState::Owned;
    }
}
MessageHandle& MessageHandle::operator=(const MessageHandle& other)
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (other.state_ == DiscordObjectState::Owned) {
            Discord_MessageHandle_Clone(&instance_, other.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
MessageHandle::MessageHandle(Discord_MessageHandle instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void MessageHandle::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_MessageHandle_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::optional<discordpp::AdditionalContent> MessageHandle::AdditionalContent() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_AdditionalContent returnValueNative__;
    returnIsNonNull__ = Discord_MessageHandle_AdditionalContent(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::AdditionalContent returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::optional<discordpp::UserHandle> MessageHandle::Author() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_UserHandle returnValueNative__;
    returnIsNonNull__ = Discord_MessageHandle_Author(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::UserHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
uint64_t MessageHandle::AuthorId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_MessageHandle_AuthorId(&instance_);
    return returnValue__;
}
std::optional<discordpp::ChannelHandle> MessageHandle::Channel() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_ChannelHandle returnValueNative__;
    returnIsNonNull__ = Discord_MessageHandle_Channel(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::ChannelHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
uint64_t MessageHandle::ChannelId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_MessageHandle_ChannelId(&instance_);
    return returnValue__;
}
std::string MessageHandle::Content() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_MessageHandle_Content(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::optional<discordpp::DisclosureTypes> MessageHandle::DisclosureType() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_DisclosureTypes returnValueNative__;
    returnIsNonNull__ = Discord_MessageHandle_DisclosureType(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    auto returnValue__ = static_cast<discordpp::DisclosureTypes>(returnValueNative__);
    return returnValue__;
}
uint64_t MessageHandle::EditedTimestamp() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_MessageHandle_EditedTimestamp(&instance_);
    return returnValue__;
}
uint64_t MessageHandle::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_MessageHandle_Id(&instance_);
    return returnValue__;
}
std::optional<discordpp::LobbyHandle> MessageHandle::Lobby() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_LobbyHandle returnValueNative__;
    returnIsNonNull__ = Discord_MessageHandle_Lobby(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::LobbyHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::unordered_map<std::string, std::string> MessageHandle::Metadata() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Properties returnValueNative__;
    Discord_MessageHandle_Metadata(&instance_, &returnValueNative__);
    std::unordered_map<std::string, std::string> returnValue__ =
      ConvertReturnedProperties(returnValueNative__);
    Discord_FreeProperties(returnValueNative__);
    return returnValue__;
}
std::string MessageHandle::RawContent() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_MessageHandle_RawContent(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::optional<discordpp::UserHandle> MessageHandle::Recipient() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_UserHandle returnValueNative__;
    returnIsNonNull__ = Discord_MessageHandle_Recipient(&instance_, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::UserHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
uint64_t MessageHandle::RecipientId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_MessageHandle_RecipientId(&instance_);
    return returnValue__;
}
bool MessageHandle::SentFromGame() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_MessageHandle_SentFromGame(&instance_);
    return returnValue__;
}
uint64_t MessageHandle::SentTimestamp() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_MessageHandle_SentTimestamp(&instance_);
    return returnValue__;
}
const AudioDevice AudioDevice::nullobj{{}, DiscordObjectState::Invalid};
AudioDevice::~AudioDevice()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
AudioDevice::AudioDevice(AudioDevice&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
AudioDevice& AudioDevice::operator=(AudioDevice&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
AudioDevice::AudioDevice(const AudioDevice& arg0)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (arg0.state_ == DiscordObjectState::Owned) {
        Discord_AudioDevice_Clone(&instance_, arg0.instance());

        state_ = DiscordObjectState::Owned;
    }
}
AudioDevice& AudioDevice::operator=(const AudioDevice& arg0)
{
    if (this != &arg0) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (arg0.state_ == DiscordObjectState::Owned) {
            Discord_AudioDevice_Clone(&instance_, arg0.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
AudioDevice::AudioDevice(Discord_AudioDevice instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void AudioDevice::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_AudioDevice_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
bool AudioDevice::Equals(discordpp::AudioDevice rhs)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_AudioDevice_Equals(&instance_, rhs.instance());
    return returnValue__;
}
std::string AudioDevice::Id() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_AudioDevice_Id(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void AudioDevice::SetId(std::string Id)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Id__str{(uint8_t*)(Id.data()), Id.size()};
    Discord_AudioDevice_SetId(&instance_, Id__str);
}
std::string AudioDevice::Name() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String returnValueNative__;
    Discord_AudioDevice_Name(&instance_, &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void AudioDevice::SetName(std::string Name)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String Name__str{(uint8_t*)(Name.data()), Name.size()};
    Discord_AudioDevice_SetName(&instance_, Name__str);
}
bool AudioDevice::IsDefault() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_AudioDevice_IsDefault(&instance_);
    return returnValue__;
}
void AudioDevice::SetIsDefault(bool IsDefault)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AudioDevice_SetIsDefault(&instance_, IsDefault);
}
const Client Client::nullobj{{}, DiscordObjectState::Invalid};
Client::~Client()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
Client::Client(Client&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
Client& Client::operator=(Client&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
Client::Client(Discord_Client instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
Client::Client()
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_Client_Init(&instance_);
    state_ = DiscordObjectState::Owned;
}
Client::Client(std::string apiBase, std::string webBase)
{
    assert(state_ == DiscordObjectState::Invalid);
    Discord_String apiBase__str{(uint8_t*)(apiBase.data()), apiBase.size()};
    Discord_String webBase__str{(uint8_t*)(webBase.data()), webBase.size()};
    Discord_Client_InitWithBases(&instance_, apiBase__str, webBase__str);
    state_ = DiscordObjectState::Owned;
}
void Client::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_Client_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
std::string Client::ErrorToString(discordpp::Client::Error type)
{
    Discord_String returnValueNative__;
    Discord_Client_ErrorToString(static_cast<Discord_Client_Error>(type), &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
uint64_t Client::GetApplicationId()
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_Client_GetApplicationId(&instance_);
    return returnValue__;
}
std::string Client::GetDefaultAudioDeviceId()
{
    Discord_String returnValueNative__;
    Discord_Client_GetDefaultAudioDeviceId(&returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::string Client::GetDefaultCommunicationScopes()
{
    Discord_String returnValueNative__;
    Discord_Client_GetDefaultCommunicationScopes(&returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::string Client::GetDefaultPresenceScopes()
{
    Discord_String returnValueNative__;
    Discord_Client_GetDefaultPresenceScopes(&returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::string Client::GetVersionHash()
{
    Discord_String returnValueNative__;
    Discord_Client_GetVersionHash(&returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
int32_t Client::GetVersionMajor()
{
    int32_t returnValue__;
    returnValue__ = Discord_Client_GetVersionMajor();
    return returnValue__;
}
int32_t Client::GetVersionMinor()
{
    int32_t returnValue__;
    returnValue__ = Discord_Client_GetVersionMinor();
    return returnValue__;
}
int32_t Client::GetVersionPatch()
{
    int32_t returnValue__;
    returnValue__ = Discord_Client_GetVersionPatch();
    return returnValue__;
}
std::string Client::StatusToString(discordpp::Client::Status type)
{
    Discord_String returnValueNative__;
    Discord_Client_StatusToString(static_cast<Discord_Client_Status>(type), &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::string Client::ThreadToString(discordpp::Client::Thread type)
{
    Discord_String returnValueNative__;
    Discord_Client_ThreadToString(static_cast<Discord_Client_Thread>(type), &returnValueNative__);
    std::string returnValue__(reinterpret_cast<char*>(returnValueNative__.ptr),
                              returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void Client::EndCall(uint64_t channelId, discordpp::Client::EndCallCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_EndCallCallback callback__native = [](void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        userData__typed->delegate();
    };
    Discord_Client_EndCall(
      &instance_, channelId, callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::EndCalls(discordpp::Client::EndCallsCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_EndCallsCallback callback__native = [](void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        userData__typed->delegate();
    };
    Discord_Client_EndCalls(
      &instance_, callback__native, Tcallback__UserData::Free, callback__userData);
}
discordpp::Call Client::GetCall(uint64_t channelId)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_Call returnValueNative__{};
    returnIsNonNull__ = Discord_Client_GetCall(&instance_, channelId, &returnValueNative__);
    discordpp::Call returnValue__(
      returnValueNative__,
      returnIsNonNull__ ? DiscordObjectState::Owned : DiscordObjectState::Invalid);
    return returnValue__;
}
std::vector<discordpp::Call> Client::GetCalls()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_CallSpan returnValueNative__;
    Discord_Client_GetCalls(&instance_, &returnValueNative__);
    std::vector<discordpp::Call> returnValue__;
    returnValue__.reserve(returnValueNative__.size);
    for (size_t i__ = 0; i__ < returnValueNative__.size; ++i__) {
        returnValue__.emplace_back(returnValueNative__.ptr[i__], DiscordObjectState::Owned);
    }
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void Client::GetCurrentInputDevice(discordpp::Client::GetCurrentInputDeviceCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_GetCurrentInputDeviceCallback cb__native = [](auto device, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::AudioDevice device__obj(*device, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(device__obj));
    };
    Discord_Client_GetCurrentInputDevice(&instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::GetCurrentOutputDevice(discordpp::Client::GetCurrentOutputDeviceCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_GetCurrentOutputDeviceCallback cb__native = [](auto device, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::AudioDevice device__obj(*device, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(device__obj));
    };
    Discord_Client_GetCurrentOutputDevice(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::GetInputDevices(discordpp::Client::GetInputDevicesCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_GetInputDevicesCallback cb__native = [](auto devices, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        std::vector<discordpp::AudioDevice> devices__vec;
        devices__vec.reserve(devices.size);
        for (size_t i__ = 0; i__ < devices.size; ++i__) {
            devices__vec.emplace_back(devices.ptr[i__], DiscordObjectState::Owned);
        }
        Discord_Free(devices.ptr);
        userData__typed->delegate(std::move(devices__vec));
    };
    Discord_Client_GetInputDevices(&instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
float Client::GetInputVolume()
{
    assert(state_ == DiscordObjectState::Owned);
    float returnValue__;
    returnValue__ = Discord_Client_GetInputVolume(&instance_);
    return returnValue__;
}
void Client::GetOutputDevices(discordpp::Client::GetOutputDevicesCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_GetOutputDevicesCallback cb__native = [](auto devices, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        std::vector<discordpp::AudioDevice> devices__vec;
        devices__vec.reserve(devices.size);
        for (size_t i__ = 0; i__ < devices.size; ++i__) {
            devices__vec.emplace_back(devices.ptr[i__], DiscordObjectState::Owned);
        }
        Discord_Free(devices.ptr);
        userData__typed->delegate(std::move(devices__vec));
    };
    Discord_Client_GetOutputDevices(&instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
float Client::GetOutputVolume()
{
    assert(state_ == DiscordObjectState::Owned);
    float returnValue__;
    returnValue__ = Discord_Client_GetOutputVolume(&instance_);
    return returnValue__;
}
bool Client::GetSelfDeafAll() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Client_GetSelfDeafAll(&instance_);
    return returnValue__;
}
bool Client::GetSelfMuteAll() const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Client_GetSelfMuteAll(&instance_);
    return returnValue__;
}
void Client::SetAutomaticGainControl(bool on)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetAutomaticGainControl(&instance_, on);
}
void Client::SetDeviceChangeCallback(discordpp::Client::DeviceChangeCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_DeviceChangeCallback callback__native =
      [](auto inputDevices, auto outputDevices, void* userData__) {
          auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
          std::vector<discordpp::AudioDevice> inputDevices__vec;
          inputDevices__vec.reserve(inputDevices.size);
          for (size_t i__ = 0; i__ < inputDevices.size; ++i__) {
              inputDevices__vec.emplace_back(inputDevices.ptr[i__], DiscordObjectState::Owned);
          }
          Discord_Free(inputDevices.ptr);
          std::vector<discordpp::AudioDevice> outputDevices__vec;
          outputDevices__vec.reserve(outputDevices.size);
          for (size_t i__ = 0; i__ < outputDevices.size; ++i__) {
              outputDevices__vec.emplace_back(outputDevices.ptr[i__], DiscordObjectState::Owned);
          }
          Discord_Free(outputDevices.ptr);
          userData__typed->delegate(std::move(inputDevices__vec), std::move(outputDevices__vec));
      };
    Discord_Client_SetDeviceChangeCallback(
      &instance_, callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::SetEchoCancellation(bool on)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetEchoCancellation(&instance_, on);
}
void Client::SetInputDevice(std::string deviceId, discordpp::Client::SetInputDeviceCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String deviceId__str{(uint8_t*)(deviceId.data()), deviceId.size()};
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SetInputDeviceCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SetInputDevice(
      &instance_, deviceId__str, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetInputVolume(float inputVolume)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetInputVolume(&instance_, inputVolume);
}
void Client::SetNoAudioInputCallback(discordpp::Client::NoAudioInputCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_NoAudioInputCallback callback__native = [](auto inputDetected,
                                                              void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        userData__typed->delegate(inputDetected);
    };
    Discord_Client_SetNoAudioInputCallback(
      &instance_, callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::SetNoAudioInputThreshold(float dBFSThreshold)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetNoAudioInputThreshold(&instance_, dBFSThreshold);
}
void Client::SetNoiseSuppression(bool on)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetNoiseSuppression(&instance_, on);
}
void Client::SetOpusHardwareCoding(bool encode, bool decode)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetOpusHardwareCoding(&instance_, encode, decode);
}
void Client::SetOutputDevice(std::string deviceId, discordpp::Client::SetOutputDeviceCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String deviceId__str{(uint8_t*)(deviceId.data()), deviceId.size()};
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SetOutputDeviceCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SetOutputDevice(
      &instance_, deviceId__str, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetOutputVolume(float outputVolume)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetOutputVolume(&instance_, outputVolume);
}
void Client::SetSelfDeafAll(bool deaf)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetSelfDeafAll(&instance_, deaf);
}
void Client::SetSelfMuteAll(bool mute)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetSelfMuteAll(&instance_, mute);
}
bool Client::SetSpeakerMode(bool speakerMode)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Client_SetSpeakerMode(&instance_, speakerMode);
    return returnValue__;
}
void Client::SetThreadPriority(discordpp::Client::Thread thread, int32_t priority)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetThreadPriority(
      &instance_, static_cast<Discord_Client_Thread>(thread), priority);
}
void Client::SetVoiceParticipantChangedCallback(
  discordpp::Client::VoiceParticipantChangedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_VoiceParticipantChangedCallback cb__native =
      [](auto lobbyId, auto memberId, auto added, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(lobbyId, memberId, added);
      };
    Discord_Client_SetVoiceParticipantChangedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
bool Client::ShowAudioRoutePicker()
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Client_ShowAudioRoutePicker(&instance_);
    return returnValue__;
}
discordpp::Call Client::StartCall(uint64_t channelId)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_Call returnValueNative__{};
    returnIsNonNull__ = Discord_Client_StartCall(&instance_, channelId, &returnValueNative__);
    discordpp::Call returnValue__(
      returnValueNative__,
      returnIsNonNull__ ? DiscordObjectState::Owned : DiscordObjectState::Invalid);
    return returnValue__;
}
discordpp::Call Client::StartCallWithAudioCallbacks(
  uint64_t lobbyId,
  discordpp::Client::UserAudioReceivedCallback receivedCb,
  discordpp::Client::UserAudioCapturedCallback capturedCb)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_Call returnValueNative__{};
    using TreceivedCb__UserData = TDelegateUserData<std::remove_reference_t<decltype(receivedCb)>>;
    auto receivedCb__userData = new TreceivedCb__UserData(receivedCb);
    Discord_Client_UserAudioReceivedCallback receivedCb__native = [](auto userId,
                                                                     auto data,
                                                                     auto samplesPerChannel,
                                                                     auto sampleRate,
                                                                     auto channels,
                                                                     auto outShouldMute,
                                                                     void* userData__) {
        auto userData__typed = static_cast<TreceivedCb__UserData*>(userData__);
        userData__typed->delegate(
          userId, data, samplesPerChannel, sampleRate, channels, *outShouldMute);
    };
    using TcapturedCb__UserData = TDelegateUserData<std::remove_reference_t<decltype(capturedCb)>>;
    auto capturedCb__userData = new TcapturedCb__UserData(capturedCb);
    Discord_Client_UserAudioCapturedCallback capturedCb__native =
      [](auto data, auto samplesPerChannel, auto sampleRate, auto channels, void* userData__) {
          auto userData__typed = static_cast<TcapturedCb__UserData*>(userData__);
          userData__typed->delegate(data, samplesPerChannel, sampleRate, channels);
      };
    returnIsNonNull__ = Discord_Client_StartCallWithAudioCallbacks(&instance_,
                                                                   lobbyId,
                                                                   receivedCb__native,
                                                                   TreceivedCb__UserData::Free,
                                                                   receivedCb__userData,
                                                                   capturedCb__native,
                                                                   TcapturedCb__UserData::Free,
                                                                   capturedCb__userData,
                                                                   &returnValueNative__);
    discordpp::Call returnValue__(
      returnValueNative__,
      returnIsNonNull__ ? DiscordObjectState::Owned : DiscordObjectState::Invalid);
    return returnValue__;
}
void Client::AbortAuthorize()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_AbortAuthorize(&instance_);
}
void Client::AbortGetTokenFromDevice()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_AbortGetTokenFromDevice(&instance_);
}
void Client::Authorize(discordpp::AuthorizationArgs args,
                       discordpp::Client::AuthorizationCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_AuthorizationCallback callback__native =
      [](auto result, auto code, auto redirectUri, void* userData__) {
          auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          std::string code__str(reinterpret_cast<char*>(code.ptr), code.size);
          std::string redirectUri__str(reinterpret_cast<char*>(redirectUri.ptr), redirectUri.size);
          userData__typed->delegate(
            std::move(result__obj), std::move(code__str), std::move(redirectUri__str));
          Discord_Free(redirectUri.ptr);
          Discord_Free(code.ptr);
      };
    Discord_Client_Authorize(
      &instance_, args.instance(), callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::CloseAuthorizeDeviceScreen()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_CloseAuthorizeDeviceScreen(&instance_);
}
discordpp::AuthorizationCodeVerifier Client::CreateAuthorizationCodeVerifier()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_AuthorizationCodeVerifier returnValueNative__{};
    Discord_Client_CreateAuthorizationCodeVerifier(&instance_, &returnValueNative__);
    discordpp::AuthorizationCodeVerifier returnValue__(returnValueNative__,
                                                       DiscordObjectState::Owned);
    return returnValue__;
}
void Client::FetchCurrentUser(discordpp::AuthorizationTokenType tokenType,
                              std::string const& token,
                              discordpp::Client::FetchCurrentUserCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String token__str{(uint8_t*)(token.data()), token.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_FetchCurrentUserCallback callback__native =
      [](auto result, auto id, auto name, void* userData__) {
          auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          std::string name__str(reinterpret_cast<char*>(name.ptr), name.size);
          userData__typed->delegate(std::move(result__obj), id, std::move(name__str));
          Discord_Free(name.ptr);
      };
    Discord_Client_FetchCurrentUser(&instance_,
                                    static_cast<Discord_AuthorizationTokenType>(tokenType),
                                    token__str,
                                    callback__native,
                                    Tcallback__UserData::Free,
                                    callback__userData);
}
void Client::GetProvisionalToken(uint64_t applicationId,
                                 discordpp::AuthenticationExternalAuthType externalAuthType,
                                 std::string const& externalAuthToken,
                                 discordpp::Client::TokenExchangeCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String externalAuthToken__str{(uint8_t*)(externalAuthToken.data()),
                                          externalAuthToken.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_TokenExchangeCallback callback__native = [](auto result,
                                                               auto accessToken,
                                                               auto refreshToken,
                                                               auto tokenType,
                                                               auto expiresIn,
                                                               auto scopes,
                                                               void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        std::string accessToken__str(reinterpret_cast<char*>(accessToken.ptr), accessToken.size);
        std::string refreshToken__str(reinterpret_cast<char*>(refreshToken.ptr), refreshToken.size);
        std::string scopes__str(reinterpret_cast<char*>(scopes.ptr), scopes.size);
        userData__typed->delegate(std::move(result__obj),
                                  std::move(accessToken__str),
                                  std::move(refreshToken__str),
                                  static_cast<discordpp::AuthorizationTokenType>(tokenType),
                                  expiresIn,
                                  std::move(scopes__str));
        Discord_Free(scopes.ptr);
        Discord_Free(refreshToken.ptr);
        Discord_Free(accessToken.ptr);
    };
    Discord_Client_GetProvisionalToken(
      &instance_,
      applicationId,
      static_cast<Discord_AuthenticationExternalAuthType>(externalAuthType),
      externalAuthToken__str,
      callback__native,
      Tcallback__UserData::Free,
      callback__userData);
}
void Client::GetToken(uint64_t applicationId,
                      std::string const& code,
                      std::string const& codeVerifier,
                      std::string const& redirectUri,
                      discordpp::Client::TokenExchangeCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String code__str{(uint8_t*)(code.data()), code.size()};
    Discord_String codeVerifier__str{(uint8_t*)(codeVerifier.data()), codeVerifier.size()};
    Discord_String redirectUri__str{(uint8_t*)(redirectUri.data()), redirectUri.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_TokenExchangeCallback callback__native = [](auto result,
                                                               auto accessToken,
                                                               auto refreshToken,
                                                               auto tokenType,
                                                               auto expiresIn,
                                                               auto scopes,
                                                               void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        std::string accessToken__str(reinterpret_cast<char*>(accessToken.ptr), accessToken.size);
        std::string refreshToken__str(reinterpret_cast<char*>(refreshToken.ptr), refreshToken.size);
        std::string scopes__str(reinterpret_cast<char*>(scopes.ptr), scopes.size);
        userData__typed->delegate(std::move(result__obj),
                                  std::move(accessToken__str),
                                  std::move(refreshToken__str),
                                  static_cast<discordpp::AuthorizationTokenType>(tokenType),
                                  expiresIn,
                                  std::move(scopes__str));
        Discord_Free(scopes.ptr);
        Discord_Free(refreshToken.ptr);
        Discord_Free(accessToken.ptr);
    };
    Discord_Client_GetToken(&instance_,
                            applicationId,
                            code__str,
                            codeVerifier__str,
                            redirectUri__str,
                            callback__native,
                            Tcallback__UserData::Free,
                            callback__userData);
}
void Client::GetTokenFromDevice(discordpp::DeviceAuthorizationArgs args,
                                discordpp::Client::TokenExchangeCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_TokenExchangeCallback callback__native = [](auto result,
                                                               auto accessToken,
                                                               auto refreshToken,
                                                               auto tokenType,
                                                               auto expiresIn,
                                                               auto scopes,
                                                               void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        std::string accessToken__str(reinterpret_cast<char*>(accessToken.ptr), accessToken.size);
        std::string refreshToken__str(reinterpret_cast<char*>(refreshToken.ptr), refreshToken.size);
        std::string scopes__str(reinterpret_cast<char*>(scopes.ptr), scopes.size);
        userData__typed->delegate(std::move(result__obj),
                                  std::move(accessToken__str),
                                  std::move(refreshToken__str),
                                  static_cast<discordpp::AuthorizationTokenType>(tokenType),
                                  expiresIn,
                                  std::move(scopes__str));
        Discord_Free(scopes.ptr);
        Discord_Free(refreshToken.ptr);
        Discord_Free(accessToken.ptr);
    };
    Discord_Client_GetTokenFromDevice(
      &instance_, args.instance(), callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::GetTokenFromDeviceProvisionalMerge(
  discordpp::DeviceAuthorizationArgs args,
  discordpp::AuthenticationExternalAuthType externalAuthType,
  std::string const& externalAuthToken,
  discordpp::Client::TokenExchangeCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String externalAuthToken__str{(uint8_t*)(externalAuthToken.data()),
                                          externalAuthToken.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_TokenExchangeCallback callback__native = [](auto result,
                                                               auto accessToken,
                                                               auto refreshToken,
                                                               auto tokenType,
                                                               auto expiresIn,
                                                               auto scopes,
                                                               void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        std::string accessToken__str(reinterpret_cast<char*>(accessToken.ptr), accessToken.size);
        std::string refreshToken__str(reinterpret_cast<char*>(refreshToken.ptr), refreshToken.size);
        std::string scopes__str(reinterpret_cast<char*>(scopes.ptr), scopes.size);
        userData__typed->delegate(std::move(result__obj),
                                  std::move(accessToken__str),
                                  std::move(refreshToken__str),
                                  static_cast<discordpp::AuthorizationTokenType>(tokenType),
                                  expiresIn,
                                  std::move(scopes__str));
        Discord_Free(scopes.ptr);
        Discord_Free(refreshToken.ptr);
        Discord_Free(accessToken.ptr);
    };
    Discord_Client_GetTokenFromDeviceProvisionalMerge(
      &instance_,
      args.instance(),
      static_cast<Discord_AuthenticationExternalAuthType>(externalAuthType),
      externalAuthToken__str,
      callback__native,
      Tcallback__UserData::Free,
      callback__userData);
}
void Client::GetTokenFromProvisionalMerge(
  uint64_t applicationId,
  std::string const& code,
  std::string const& codeVerifier,
  std::string const& redirectUri,
  discordpp::AuthenticationExternalAuthType externalAuthType,
  std::string const& externalAuthToken,
  discordpp::Client::TokenExchangeCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String code__str{(uint8_t*)(code.data()), code.size()};
    Discord_String codeVerifier__str{(uint8_t*)(codeVerifier.data()), codeVerifier.size()};
    Discord_String redirectUri__str{(uint8_t*)(redirectUri.data()), redirectUri.size()};
    Discord_String externalAuthToken__str{(uint8_t*)(externalAuthToken.data()),
                                          externalAuthToken.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_TokenExchangeCallback callback__native = [](auto result,
                                                               auto accessToken,
                                                               auto refreshToken,
                                                               auto tokenType,
                                                               auto expiresIn,
                                                               auto scopes,
                                                               void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        std::string accessToken__str(reinterpret_cast<char*>(accessToken.ptr), accessToken.size);
        std::string refreshToken__str(reinterpret_cast<char*>(refreshToken.ptr), refreshToken.size);
        std::string scopes__str(reinterpret_cast<char*>(scopes.ptr), scopes.size);
        userData__typed->delegate(std::move(result__obj),
                                  std::move(accessToken__str),
                                  std::move(refreshToken__str),
                                  static_cast<discordpp::AuthorizationTokenType>(tokenType),
                                  expiresIn,
                                  std::move(scopes__str));
        Discord_Free(scopes.ptr);
        Discord_Free(refreshToken.ptr);
        Discord_Free(accessToken.ptr);
    };
    Discord_Client_GetTokenFromProvisionalMerge(
      &instance_,
      applicationId,
      code__str,
      codeVerifier__str,
      redirectUri__str,
      static_cast<Discord_AuthenticationExternalAuthType>(externalAuthType),
      externalAuthToken__str,
      callback__native,
      Tcallback__UserData::Free,
      callback__userData);
}
bool Client::IsAuthenticated()
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Client_IsAuthenticated(&instance_);
    return returnValue__;
}
void Client::OpenAuthorizeDeviceScreen(uint64_t clientId, std::string const& userCode)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String userCode__str{(uint8_t*)(userCode.data()), userCode.size()};
    Discord_Client_OpenAuthorizeDeviceScreen(&instance_, clientId, userCode__str);
}
void Client::ProvisionalUserMergeCompleted(bool success)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_ProvisionalUserMergeCompleted(&instance_, success);
}
void Client::RefreshToken(uint64_t applicationId,
                          std::string const& refreshToken,
                          discordpp::Client::TokenExchangeCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String refreshToken__str{(uint8_t*)(refreshToken.data()), refreshToken.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_TokenExchangeCallback callback__native = [](auto result,
                                                               auto accessToken,
                                                               auto refreshToken,
                                                               auto tokenType,
                                                               auto expiresIn,
                                                               auto scopes,
                                                               void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        std::string accessToken__str(reinterpret_cast<char*>(accessToken.ptr), accessToken.size);
        std::string refreshToken__str(reinterpret_cast<char*>(refreshToken.ptr), refreshToken.size);
        std::string scopes__str(reinterpret_cast<char*>(scopes.ptr), scopes.size);
        userData__typed->delegate(std::move(result__obj),
                                  std::move(accessToken__str),
                                  std::move(refreshToken__str),
                                  static_cast<discordpp::AuthorizationTokenType>(tokenType),
                                  expiresIn,
                                  std::move(scopes__str));
        Discord_Free(scopes.ptr);
        Discord_Free(refreshToken.ptr);
        Discord_Free(accessToken.ptr);
    };
    Discord_Client_RefreshToken(&instance_,
                                applicationId,
                                refreshToken__str,
                                callback__native,
                                Tcallback__UserData::Free,
                                callback__userData);
}
void Client::SetAuthorizeDeviceScreenClosedCallback(
  discordpp::Client::AuthorizeDeviceScreenClosedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_AuthorizeDeviceScreenClosedCallback cb__native = [](void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate();
    };
    Discord_Client_SetAuthorizeDeviceScreenClosedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetGameWindowPid(int32_t pid)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetGameWindowPid(&instance_, pid);
}
void Client::SetTokenExpirationCallback(discordpp::Client::TokenExpirationCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_TokenExpirationCallback callback__native = [](void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        userData__typed->delegate();
    };
    Discord_Client_SetTokenExpirationCallback(
      &instance_, callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::UpdateProvisionalAccountDisplayName(
  std::string const& name,
  discordpp::Client::UpdateProvisionalAccountDisplayNameCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String name__str{(uint8_t*)(name.data()), name.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_UpdateProvisionalAccountDisplayNameCallback callback__native =
      [](auto result, void* userData__) {
          auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          userData__typed->delegate(std::move(result__obj));
      };
    Discord_Client_UpdateProvisionalAccountDisplayName(
      &instance_, name__str, callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::UpdateToken(discordpp::AuthorizationTokenType tokenType,
                         std::string token,
                         discordpp::Client::UpdateTokenCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String token__str{(uint8_t*)(token.data()), token.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_UpdateTokenCallback callback__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_UpdateToken(&instance_,
                               static_cast<Discord_AuthorizationTokenType>(tokenType),
                               token__str,
                               callback__native,
                               Tcallback__UserData::Free,
                               callback__userData);
}
bool Client::CanOpenMessageInDiscord(uint64_t messageId)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ = Discord_Client_CanOpenMessageInDiscord(&instance_, messageId);
    return returnValue__;
}
void Client::DeleteUserMessage(uint64_t recipientId,
                               uint64_t messageId,
                               discordpp::Client::DeleteUserMessageCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_DeleteUserMessageCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_DeleteUserMessage(
      &instance_, recipientId, messageId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::EditUserMessage(uint64_t recipientId,
                             uint64_t messageId,
                             std::string const& content,
                             discordpp::Client::EditUserMessageCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String content__str{(uint8_t*)(content.data()), content.size()};
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_EditUserMessageCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_EditUserMessage(&instance_,
                                   recipientId,
                                   messageId,
                                   content__str,
                                   cb__native,
                                   Tcb__UserData::Free,
                                   cb__userData);
}
std::optional<discordpp::ChannelHandle> Client::GetChannelHandle(uint64_t channelId) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_ChannelHandle returnValueNative__;
    returnIsNonNull__ =
      Discord_Client_GetChannelHandle(&instance_, channelId, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::ChannelHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::optional<discordpp::MessageHandle> Client::GetMessageHandle(uint64_t messageId) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_MessageHandle returnValueNative__;
    returnIsNonNull__ =
      Discord_Client_GetMessageHandle(&instance_, messageId, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::MessageHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void Client::OpenMessageInDiscord(
  uint64_t messageId,
  discordpp::Client::ProvisionalUserMergeRequiredCallback provisionalUserMergeRequiredCallback,
  discordpp::Client::OpenMessageInDiscordCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using TprovisionalUserMergeRequiredCallback__UserData =
      TDelegateUserData<std::remove_reference_t<decltype(provisionalUserMergeRequiredCallback)>>;
    auto provisionalUserMergeRequiredCallback__userData =
      new TprovisionalUserMergeRequiredCallback__UserData(provisionalUserMergeRequiredCallback);
    Discord_Client_ProvisionalUserMergeRequiredCallback
      provisionalUserMergeRequiredCallback__native = [](void* userData__) {
          auto userData__typed =
            static_cast<TprovisionalUserMergeRequiredCallback__UserData*>(userData__);
          userData__typed->delegate();
      };
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_OpenMessageInDiscordCallback callback__native = [](auto result,
                                                                      void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_OpenMessageInDiscord(&instance_,
                                        messageId,
                                        provisionalUserMergeRequiredCallback__native,
                                        TprovisionalUserMergeRequiredCallback__UserData::Free,
                                        provisionalUserMergeRequiredCallback__userData,
                                        callback__native,
                                        Tcallback__UserData::Free,
                                        callback__userData);
}
void Client::SendLobbyMessage(uint64_t lobbyId,
                              std::string const& content,
                              discordpp::Client::SendUserMessageCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String content__str{(uint8_t*)(content.data()), content.size()};
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendUserMessageCallback cb__native =
      [](auto result, auto messageId, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          userData__typed->delegate(std::move(result__obj), messageId);
      };
    Discord_Client_SendLobbyMessage(
      &instance_, lobbyId, content__str, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SendLobbyMessageWithMetadata(
  uint64_t lobbyId,
  std::string const& content,
  std::unordered_map<std::string, std::string> const& metadata,
  discordpp::Client::SendUserMessageCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String content__str{(uint8_t*)(content.data()), content.size()};
    ConvertedProperties metadata__convert(metadata);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendUserMessageCallback cb__native =
      [](auto result, auto messageId, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          userData__typed->delegate(std::move(result__obj), messageId);
      };
    Discord_Client_SendLobbyMessageWithMetadata(&instance_,
                                                lobbyId,
                                                content__str,
                                                metadata__convert.Properties,
                                                cb__native,
                                                Tcb__UserData::Free,
                                                cb__userData);
}
void Client::SendUserMessage(uint64_t recipientId,
                             std::string const& content,
                             discordpp::Client::SendUserMessageCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String content__str{(uint8_t*)(content.data()), content.size()};
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendUserMessageCallback cb__native =
      [](auto result, auto messageId, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          userData__typed->delegate(std::move(result__obj), messageId);
      };
    Discord_Client_SendUserMessage(
      &instance_, recipientId, content__str, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SendUserMessageWithMetadata(
  uint64_t recipientId,
  std::string const& content,
  std::unordered_map<std::string, std::string> const& metadata,
  discordpp::Client::SendUserMessageCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String content__str{(uint8_t*)(content.data()), content.size()};
    ConvertedProperties metadata__convert(metadata);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendUserMessageCallback cb__native =
      [](auto result, auto messageId, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          userData__typed->delegate(std::move(result__obj), messageId);
      };
    Discord_Client_SendUserMessageWithMetadata(&instance_,
                                               recipientId,
                                               content__str,
                                               metadata__convert.Properties,
                                               cb__native,
                                               Tcb__UserData::Free,
                                               cb__userData);
}
void Client::SetMessageCreatedCallback(discordpp::Client::MessageCreatedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_MessageCreatedCallback cb__native = [](auto messageId, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate(messageId);
    };
    Discord_Client_SetMessageCreatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetMessageDeletedCallback(discordpp::Client::MessageDeletedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_MessageDeletedCallback cb__native =
      [](auto messageId, auto channelId, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(messageId, channelId);
      };
    Discord_Client_SetMessageDeletedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetMessageUpdatedCallback(discordpp::Client::MessageUpdatedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_MessageUpdatedCallback cb__native = [](auto messageId, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate(messageId);
    };
    Discord_Client_SetMessageUpdatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetShowingChat(bool showingChat)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetShowingChat(&instance_, showingChat);
}
void Client::AddLogCallback(discordpp::Client::LogCallback callback,
                            discordpp::LoggingSeverity minSeverity)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_LogCallback callback__native = [](
                                                    auto message, auto severity, void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        std::string message__str(reinterpret_cast<char*>(message.ptr), message.size);
        userData__typed->delegate(std::move(message__str),
                                  static_cast<discordpp::LoggingSeverity>(severity));
        Discord_Free(message.ptr);
    };
    Discord_Client_AddLogCallback(&instance_,
                                  callback__native,
                                  Tcallback__UserData::Free,
                                  callback__userData,
                                  static_cast<Discord_LoggingSeverity>(minSeverity));
}
void Client::AddVoiceLogCallback(discordpp::Client::LogCallback callback,
                                 discordpp::LoggingSeverity minSeverity)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_LogCallback callback__native = [](
                                                    auto message, auto severity, void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        std::string message__str(reinterpret_cast<char*>(message.ptr), message.size);
        userData__typed->delegate(std::move(message__str),
                                  static_cast<discordpp::LoggingSeverity>(severity));
        Discord_Free(message.ptr);
    };
    Discord_Client_AddVoiceLogCallback(&instance_,
                                       callback__native,
                                       Tcallback__UserData::Free,
                                       callback__userData,
                                       static_cast<Discord_LoggingSeverity>(minSeverity));
}
void Client::Connect()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_Connect(&instance_);
}
void Client::Disconnect()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_Disconnect(&instance_);
}
discordpp::Client::Status Client::GetStatus() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_Status returnValue__;
    returnValue__ = Discord_Client_GetStatus(&instance_);
    return static_cast<discordpp::Client::Status>(returnValue__);
}
void Client::SetApplicationId(uint64_t applicationId)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_SetApplicationId(&instance_, applicationId);
}
bool Client::SetLogDir(std::string const& path, discordpp::LoggingSeverity minSeverity)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    Discord_String path__str{(uint8_t*)(path.data()), path.size()};
    returnValue__ = Discord_Client_SetLogDir(
      &instance_, path__str, static_cast<Discord_LoggingSeverity>(minSeverity));
    return returnValue__;
}
void Client::SetStatusChangedCallback(discordpp::Client::OnStatusChanged cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_OnStatusChanged cb__native =
      [](auto status, auto error, auto errorDetail, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(static_cast<discordpp::Client::Status>(status),
                                    static_cast<discordpp::Client::Error>(error),
                                    errorDetail);
      };
    Discord_Client_SetStatusChangedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetVoiceLogDir(std::string const& path, discordpp::LoggingSeverity minSeverity)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String path__str{(uint8_t*)(path.data()), path.size()};
    Discord_Client_SetVoiceLogDir(
      &instance_, path__str, static_cast<Discord_LoggingSeverity>(minSeverity));
}
void Client::CreateOrJoinLobby(std::string const& secret,
                               discordpp::Client::CreateOrJoinLobbyCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String secret__str{(uint8_t*)(secret.data()), secret.size()};
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_CreateOrJoinLobbyCallback callback__native =
      [](auto result, auto lobbyId, void* userData__) {
          auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          userData__typed->delegate(std::move(result__obj), lobbyId);
      };
    Discord_Client_CreateOrJoinLobby(
      &instance_, secret__str, callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::CreateOrJoinLobbyWithMetadata(
  std::string const& secret,
  std::unordered_map<std::string, std::string> const& lobbyMetadata,
  std::unordered_map<std::string, std::string> const& memberMetadata,
  discordpp::Client::CreateOrJoinLobbyCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String secret__str{(uint8_t*)(secret.data()), secret.size()};
    ConvertedProperties lobbyMetadata__convert(lobbyMetadata);
    ConvertedProperties memberMetadata__convert(memberMetadata);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_CreateOrJoinLobbyCallback callback__native =
      [](auto result, auto lobbyId, void* userData__) {
          auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          userData__typed->delegate(std::move(result__obj), lobbyId);
      };
    Discord_Client_CreateOrJoinLobbyWithMetadata(&instance_,
                                                 secret__str,
                                                 lobbyMetadata__convert.Properties,
                                                 memberMetadata__convert.Properties,
                                                 callback__native,
                                                 Tcallback__UserData::Free,
                                                 callback__userData);
}
void Client::GetGuildChannels(uint64_t guildId, discordpp::Client::GetGuildChannelsCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_GetGuildChannelsCallback cb__native =
      [](auto result, auto guildChannels, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          std::vector<discordpp::GuildChannel> guildChannels__vec;
          guildChannels__vec.reserve(guildChannels.size);
          for (size_t i__ = 0; i__ < guildChannels.size; ++i__) {
              guildChannels__vec.emplace_back(guildChannels.ptr[i__], DiscordObjectState::Owned);
          }
          Discord_Free(guildChannels.ptr);
          userData__typed->delegate(std::move(result__obj), std::move(guildChannels__vec));
      };
    Discord_Client_GetGuildChannels(
      &instance_, guildId, cb__native, Tcb__UserData::Free, cb__userData);
}
std::optional<discordpp::LobbyHandle> Client::GetLobbyHandle(uint64_t lobbyId) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_LobbyHandle returnValueNative__;
    returnIsNonNull__ = Discord_Client_GetLobbyHandle(&instance_, lobbyId, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::LobbyHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::vector<uint64_t> Client::GetLobbyIds() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_UInt64Span returnValueNative__;
    Discord_Client_GetLobbyIds(&instance_, &returnValueNative__);
    std::vector<uint64_t> returnValue__(returnValueNative__.ptr,
                                        returnValueNative__.ptr + returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void Client::GetUserGuilds(discordpp::Client::GetUserGuildsCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_GetUserGuildsCallback cb__native =
      [](auto result, auto guilds, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          std::vector<discordpp::GuildMinimal> guilds__vec;
          guilds__vec.reserve(guilds.size);
          for (size_t i__ = 0; i__ < guilds.size; ++i__) {
              guilds__vec.emplace_back(guilds.ptr[i__], DiscordObjectState::Owned);
          }
          Discord_Free(guilds.ptr);
          userData__typed->delegate(std::move(result__obj), std::move(guilds__vec));
      };
    Discord_Client_GetUserGuilds(&instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::LeaveLobby(uint64_t lobbyId, discordpp::Client::LeaveLobbyCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_LeaveLobbyCallback callback__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_LeaveLobby(
      &instance_, lobbyId, callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::LinkChannelToLobby(uint64_t lobbyId,
                                uint64_t channelId,
                                discordpp::Client::LinkOrUnlinkChannelCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_LinkOrUnlinkChannelCallback callback__native = [](auto result,
                                                                     void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_LinkChannelToLobby(&instance_,
                                      lobbyId,
                                      channelId,
                                      callback__native,
                                      Tcallback__UserData::Free,
                                      callback__userData);
}
void Client::SetLobbyCreatedCallback(discordpp::Client::LobbyCreatedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_LobbyCreatedCallback cb__native = [](auto lobbyId, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate(lobbyId);
    };
    Discord_Client_SetLobbyCreatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetLobbyDeletedCallback(discordpp::Client::LobbyDeletedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_LobbyDeletedCallback cb__native = [](auto lobbyId, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate(lobbyId);
    };
    Discord_Client_SetLobbyDeletedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetLobbyMemberAddedCallback(discordpp::Client::LobbyMemberAddedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_LobbyMemberAddedCallback cb__native =
      [](auto lobbyId, auto memberId, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(lobbyId, memberId);
      };
    Discord_Client_SetLobbyMemberAddedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetLobbyMemberRemovedCallback(discordpp::Client::LobbyMemberRemovedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_LobbyMemberRemovedCallback cb__native =
      [](auto lobbyId, auto memberId, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(lobbyId, memberId);
      };
    Discord_Client_SetLobbyMemberRemovedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetLobbyMemberUpdatedCallback(discordpp::Client::LobbyMemberUpdatedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_LobbyMemberUpdatedCallback cb__native =
      [](auto lobbyId, auto memberId, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(lobbyId, memberId);
      };
    Discord_Client_SetLobbyMemberUpdatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetLobbyUpdatedCallback(discordpp::Client::LobbyUpdatedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_LobbyUpdatedCallback cb__native = [](auto lobbyId, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate(lobbyId);
    };
    Discord_Client_SetLobbyUpdatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::UnlinkChannelFromLobby(uint64_t lobbyId,
                                    discordpp::Client::LinkOrUnlinkChannelCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_LinkOrUnlinkChannelCallback callback__native = [](auto result,
                                                                     void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_UnlinkChannelFromLobby(
      &instance_, lobbyId, callback__native, Tcallback__UserData::Free, callback__userData);
}
void Client::AcceptActivityInvite(discordpp::ActivityInvite invite,
                                  discordpp::Client::AcceptActivityInviteCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_AcceptActivityInviteCallback cb__native =
      [](auto result, auto joinSecret, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          std::string joinSecret__str(reinterpret_cast<char*>(joinSecret.ptr), joinSecret.size);
          userData__typed->delegate(std::move(result__obj), std::move(joinSecret__str));
          Discord_Free(joinSecret.ptr);
      };
    Discord_Client_AcceptActivityInvite(
      &instance_, invite.instance(), cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::ClearRichPresence()
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_Client_ClearRichPresence(&instance_);
}
bool Client::RegisterLaunchCommand(uint64_t applicationId, std::string command)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    Discord_String command__str{(uint8_t*)(command.data()), command.size()};
    returnValue__ = Discord_Client_RegisterLaunchCommand(&instance_, applicationId, command__str);
    return returnValue__;
}
bool Client::RegisterLaunchSteamApplication(uint64_t applicationId, uint32_t steamAppId)
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnValue__;
    returnValue__ =
      Discord_Client_RegisterLaunchSteamApplication(&instance_, applicationId, steamAppId);
    return returnValue__;
}
void Client::SendActivityInvite(uint64_t userId,
                                std::string const& content,
                                discordpp::Client::SendActivityInviteCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String content__str{(uint8_t*)(content.data()), content.size()};
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendActivityInviteCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SendActivityInvite(
      &instance_, userId, content__str, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SendActivityJoinRequest(uint64_t userId,
                                     discordpp::Client::SendActivityInviteCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendActivityInviteCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SendActivityJoinRequest(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SendActivityJoinRequestReply(discordpp::ActivityInvite invite,
                                          discordpp::Client::SendActivityInviteCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendActivityInviteCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SendActivityJoinRequestReply(
      &instance_, invite.instance(), cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetActivityInviteCreatedCallback(discordpp::Client::ActivityInviteCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_ActivityInviteCallback cb__native = [](auto invite, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ActivityInvite invite__obj(*invite, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(invite__obj));
    };
    Discord_Client_SetActivityInviteCreatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetActivityInviteUpdatedCallback(discordpp::Client::ActivityInviteCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_ActivityInviteCallback cb__native = [](auto invite, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ActivityInvite invite__obj(*invite, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(invite__obj));
    };
    Discord_Client_SetActivityInviteUpdatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetActivityJoinCallback(discordpp::Client::ActivityJoinCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_ActivityJoinCallback cb__native = [](auto joinSecret, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        std::string joinSecret__str(reinterpret_cast<char*>(joinSecret.ptr), joinSecret.size);
        userData__typed->delegate(std::move(joinSecret__str));
        Discord_Free(joinSecret.ptr);
    };
    Discord_Client_SetActivityJoinCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetOnlineStatus(discordpp::StatusType status,
                             discordpp::Client::UpdateStatusCallback callback)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_UpdateStatusCallback callback__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SetOnlineStatus(&instance_,
                                   static_cast<Discord_StatusType>(status),
                                   callback__native,
                                   Tcallback__UserData::Free,
                                   callback__userData);
}
void Client::UpdateRichPresence(discordpp::Activity activity,
                                discordpp::Client::UpdateRichPresenceCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRichPresenceCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_UpdateRichPresence(
      &instance_, activity.instance(), cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::AcceptDiscordFriendRequest(uint64_t userId,
                                        discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_AcceptDiscordFriendRequest(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::AcceptGameFriendRequest(uint64_t userId,
                                     discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_AcceptGameFriendRequest(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::BlockUser(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_BlockUser(&instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::CancelDiscordFriendRequest(uint64_t userId,
                                        discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_CancelDiscordFriendRequest(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::CancelGameFriendRequest(uint64_t userId,
                                     discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_CancelGameFriendRequest(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
discordpp::RelationshipHandle Client::GetRelationshipHandle(uint64_t userId) const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_RelationshipHandle returnValueNative__{};
    Discord_Client_GetRelationshipHandle(&instance_, userId, &returnValueNative__);
    discordpp::RelationshipHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
std::vector<discordpp::RelationshipHandle> Client::GetRelationships() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_RelationshipHandleSpan returnValueNative__;
    Discord_Client_GetRelationships(&instance_, &returnValueNative__);
    std::vector<discordpp::RelationshipHandle> returnValue__;
    returnValue__.reserve(returnValueNative__.size);
    for (size_t i__ = 0; i__ < returnValueNative__.size; ++i__) {
        returnValue__.emplace_back(returnValueNative__.ptr[i__], DiscordObjectState::Owned);
    }
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void Client::RejectDiscordFriendRequest(uint64_t userId,
                                        discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_RejectDiscordFriendRequest(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::RejectGameFriendRequest(uint64_t userId,
                                     discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_RejectGameFriendRequest(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::RemoveDiscordAndGameFriend(uint64_t userId,
                                        discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_RemoveDiscordAndGameFriend(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::RemoveGameFriend(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_RemoveGameFriend(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
std::vector<discordpp::UserHandle> Client::SearchFriendsByUsername(std::string searchStr) const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_UserHandleSpan returnValueNative__;
    Discord_String searchStr__str{(uint8_t*)(searchStr.data()), searchStr.size()};
    Discord_Client_SearchFriendsByUsername(&instance_, searchStr__str, &returnValueNative__);
    std::vector<discordpp::UserHandle> returnValue__;
    returnValue__.reserve(returnValueNative__.size);
    for (size_t i__ = 0; i__ < returnValueNative__.size; ++i__) {
        returnValue__.emplace_back(returnValueNative__.ptr[i__], DiscordObjectState::Owned);
    }
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
void Client::SendDiscordFriendRequest(std::string const& username,
                                      discordpp::Client::SendFriendRequestCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String username__str{(uint8_t*)(username.data()), username.size()};
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendFriendRequestCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SendDiscordFriendRequest(
      &instance_, username__str, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SendDiscordFriendRequestById(uint64_t userId,
                                          discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SendDiscordFriendRequestById(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SendGameFriendRequest(std::string const& username,
                                   discordpp::Client::SendFriendRequestCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_String username__str{(uint8_t*)(username.data()), username.size()};
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_SendFriendRequestCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SendGameFriendRequest(
      &instance_, username__str, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SendGameFriendRequestById(uint64_t userId,
                                       discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_SendGameFriendRequestById(
      &instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetRelationshipCreatedCallback(discordpp::Client::RelationshipCreatedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_RelationshipCreatedCallback cb__native =
      [](auto userId, auto isDiscordRelationshipUpdate, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(userId, isDiscordRelationshipUpdate);
      };
    Discord_Client_SetRelationshipCreatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::SetRelationshipDeletedCallback(discordpp::Client::RelationshipDeletedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_RelationshipDeletedCallback cb__native =
      [](auto userId, auto isDiscordRelationshipUpdate, void* userData__) {
          auto userData__typed = static_cast<Tcb__UserData*>(userData__);
          userData__typed->delegate(userId, isDiscordRelationshipUpdate);
      };
    Discord_Client_SetRelationshipDeletedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
void Client::UnblockUser(uint64_t userId, discordpp::Client::UpdateRelationshipCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UpdateRelationshipCallback cb__native = [](auto result, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
        userData__typed->delegate(std::move(result__obj));
    };
    Discord_Client_UnblockUser(&instance_, userId, cb__native, Tcb__UserData::Free, cb__userData);
}
discordpp::UserHandle Client::GetCurrentUser() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_UserHandle returnValueNative__{};
    Discord_Client_GetCurrentUser(&instance_, &returnValueNative__);
    discordpp::UserHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void Client::GetDiscordClientConnectedUser(
  uint64_t applicationId,
  discordpp::Client::GetDiscordClientConnectedUserCallback callback) const
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcallback__UserData = TDelegateUserData<std::remove_reference_t<decltype(callback)>>;
    auto callback__userData = new Tcallback__UserData(callback);
    Discord_Client_GetDiscordClientConnectedUserCallback callback__native =
      [](auto result, auto user, void* userData__) {
          auto userData__typed = static_cast<Tcallback__UserData*>(userData__);
          discordpp::ClientResult result__obj(*result, DiscordObjectState::Owned);
          std::optional<discordpp::UserHandle> user__opt{};
          if (user) {
              user__opt = discordpp::UserHandle(*user, DiscordObjectState::Owned);
          }
          userData__typed->delegate(std::move(result__obj), std::move(user__opt));
      };
    Discord_Client_GetDiscordClientConnectedUser(
      &instance_, applicationId, callback__native, Tcallback__UserData::Free, callback__userData);
}
std::optional<discordpp::UserHandle> Client::GetUser(uint64_t userId) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_UserHandle returnValueNative__;
    returnIsNonNull__ = Discord_Client_GetUser(&instance_, userId, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::UserHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
void Client::SetUserUpdatedCallback(discordpp::Client::UserUpdatedCallback cb)
{
    assert(state_ == DiscordObjectState::Owned);
    using Tcb__UserData = TDelegateUserData<std::remove_reference_t<decltype(cb)>>;
    auto cb__userData = new Tcb__UserData(cb);
    Discord_Client_UserUpdatedCallback cb__native = [](auto userId, void* userData__) {
        auto userData__typed = static_cast<Tcb__UserData*>(userData__);
        userData__typed->delegate(userId);
    };
    Discord_Client_SetUserUpdatedCallback(
      &instance_, cb__native, Tcb__UserData::Free, cb__userData);
}
const CallInfoHandle CallInfoHandle::nullobj{{}, DiscordObjectState::Invalid};
CallInfoHandle::~CallInfoHandle()
{
    if (state_ == DiscordObjectState::Owned) {
        Drop();
        state_ = DiscordObjectState::Invalid;
    }
}
CallInfoHandle::CallInfoHandle(CallInfoHandle&& other) noexcept
  : instance_(other.instance_)
  , state_(other.state_)
{
    other.state_ = DiscordObjectState::Invalid;
}
CallInfoHandle& CallInfoHandle::operator=(CallInfoHandle&& other) noexcept
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
        }
        instance_ = other.instance_;
        state_ = other.state_;
        other.state_ = DiscordObjectState::Invalid;
    }
    return *this;
}
CallInfoHandle::CallInfoHandle(const CallInfoHandle& other)
  : instance_{}
  , state_(DiscordObjectState::Invalid)
{
    if (other.state_ == DiscordObjectState::Owned) {
        Discord_CallInfoHandle_Clone(&instance_, other.instance());

        state_ = DiscordObjectState::Owned;
    }
}
CallInfoHandle& CallInfoHandle::operator=(const CallInfoHandle& other)
{
    if (this != &other) {
        if (state_ == DiscordObjectState::Owned) {
            Drop();
            state_ = DiscordObjectState::Invalid;
        }
        if (other.state_ == DiscordObjectState::Owned) {
            Discord_CallInfoHandle_Clone(&instance_, other.instance());

            state_ = DiscordObjectState::Owned;
        }
    }
    return *this;
}
CallInfoHandle::CallInfoHandle(Discord_CallInfoHandle instance, DiscordObjectState state)
  : instance_(instance)
  , state_(state)
{
}
void CallInfoHandle::Drop()
{
    if (state_ != DiscordObjectState::Owned) {
        return;
    }
    Discord_CallInfoHandle_Drop(&instance_);
    state_ = DiscordObjectState::Invalid;
}
uint64_t CallInfoHandle::ChannelId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_CallInfoHandle_ChannelId(&instance_);
    return returnValue__;
}
std::vector<uint64_t> CallInfoHandle::GetParticipants() const
{
    assert(state_ == DiscordObjectState::Owned);
    Discord_UInt64Span returnValueNative__;
    Discord_CallInfoHandle_GetParticipants(&instance_, &returnValueNative__);
    std::vector<uint64_t> returnValue__(returnValueNative__.ptr,
                                        returnValueNative__.ptr + returnValueNative__.size);
    Discord_Free(returnValueNative__.ptr);
    return returnValue__;
}
std::optional<discordpp::VoiceStateHandle> CallInfoHandle::GetVoiceStateHandle(
  uint64_t userId) const
{
    assert(state_ == DiscordObjectState::Owned);
    bool returnIsNonNull__;
    Discord_VoiceStateHandle returnValueNative__;
    returnIsNonNull__ =
      Discord_CallInfoHandle_GetVoiceStateHandle(&instance_, userId, &returnValueNative__);
    if (!returnIsNonNull__) {
        return {};
    }
    discordpp::VoiceStateHandle returnValue__(returnValueNative__, DiscordObjectState::Owned);
    return returnValue__;
}
uint64_t CallInfoHandle::GuildId() const
{
    assert(state_ == DiscordObjectState::Owned);
    uint64_t returnValue__;
    returnValue__ = Discord_CallInfoHandle_GuildId(&instance_);
    return returnValue__;
}
} // namespace discordpp

#endif // DISCORDPP_IMPLEMENTATION
