#include "MprisComponent.h"
#include "MprisRootAdaptor.h"
#include "MprisPlayerAdaptor.h"
#include "player/PlayerComponent.h"
#include "input/InputComponent.h"
#include "core/Globals.h"
#include "ui/KonvergoWindow.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusError>
#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QUrlQuery>

#define MPRIS_SERVICE_NAME "org.mpris.MediaPlayer2.jellyfinmediaplayer"
#define MPRIS_OBJECT_PATH "/org/mpris/MediaPlayer2"

MprisComponent::MprisComponent(QObject* parent)
  : ComponentBase(parent)
  , m_enabled(false)
  , m_player(nullptr)
  , m_playbackStatus("Stopped")
  , m_position(0)
  , m_duration(0)
  , m_positionTimer(nullptr)
  , m_canGoNext(false)
  , m_canGoPrevious(false)
  , m_shuffle(false)
  , m_loopStatus("None")
  , m_rate(1.0)
  , m_volume(1.0)
  , m_seekPending(false)
  , m_expectedPosition(0)
  , m_isNavigating(false)
  , m_albumArtDir("/tmp/jellyfin-mpris")
  , m_playerState(PlayerComponent::State::finished)
  , m_albumArtManager(nullptr)
  , m_pendingArtReply(nullptr)
{
  // Create album art directory
  QDir dir;
  dir.mkpath(m_albumArtDir);

  // Initialize network manager for async album art downloads
  m_albumArtManager = new QNetworkAccessManager(this);
}

MprisComponent::~MprisComponent()
{
  if (m_enabled)
  {
    disconnectPlayerSignals();
    cleanupAlbumArt();
    QDBusConnection::sessionBus().unregisterService(MPRIS_SERVICE_NAME);
    QDBusConnection::sessionBus().unregisterObject(MPRIS_OBJECT_PATH);
  }
}

bool MprisComponent::componentInitialize()
{
  // Check if D-Bus session bus is available
  if (!QDBusConnection::sessionBus().isConnected())
  {
    qDebug() << "D-Bus session bus not available, MPRIS disabled";
    return true; // Not an error, just disabled
  }

  // Register the service name
  qDebug() << "Attempting to register MPRIS service:" << MPRIS_SERVICE_NAME;
  if (!QDBusConnection::sessionBus().registerService(MPRIS_SERVICE_NAME))
  {
    qWarning() << "Failed to register MPRIS service:"
               << QDBusConnection::sessionBus().lastError().message();
    return true; // Not fatal, continue without MPRIS
  }
  qDebug() << "MPRIS service registered successfully";

  // Create adaptors
  m_rootAdaptor = std::make_unique<MprisRootAdaptor>(this);
  m_playerAdaptor = std::make_unique<MprisPlayerAdaptor>(this);

  // Register object
  if (!QDBusConnection::sessionBus().registerObject(MPRIS_OBJECT_PATH, this))
  {
    qWarning() << "Failed to register MPRIS object:"
               << QDBusConnection::sessionBus().lastError().message();
    QDBusConnection::sessionBus().unregisterService(MPRIS_SERVICE_NAME);
    return true;
  }

  m_enabled = true;
  qDebug() << "MPRIS interface registered successfully";

  return true;
}

void MprisComponent::componentPostInitialize()
{
  if (!m_enabled)
    return;

  m_player = &PlayerComponent::Get();
  connectPlayerSignals();

  // Set up position update timer (for clients that poll instead of using signals)
  m_positionTimer = new QTimer(this);
  m_positionTimer->setInterval(500);
  connect(m_positionTimer, &QTimer::timeout, [this]() {
    if (m_playbackStatus == "Playing")
    {
      // Position is tracked internally and updated via signals
      // This timer is just for edge cases
    }
  });
}

void MprisComponent::connectPlayerSignals()
{
  if (!m_player)
    return;

  // All MPRIS updates now come from JavaScript business logic via notify* methods
  // This decouples MPRIS from MPV, making it work with any player (MPV, HTML5, etc.)

  // Playback state updates via notifyPlaybackState()
  // connect(m_player, &PlayerComponent::playing,
  //         this, &MprisComponent::onPlayerPlaying);
  // connect(m_player, &PlayerComponent::paused,
  //         this, &MprisComponent::onPlayerPaused);
  // connect(m_player, &PlayerComponent::stopped,
  //         this, &MprisComponent::onPlayerStopped);
  // connect(m_player, &PlayerComponent::finished,
  //         this, &MprisComponent::onPlayerFinished);
  // connect(m_player, &PlayerComponent::stateChanged,
  //         this, &MprisComponent::onPlayerStateChanged);

  // Position updates via notifyPosition()
  // connect(m_player, &PlayerComponent::positionUpdate,
  //         this, &MprisComponent::onPlayerPositionUpdate);

  // Duration updates via notifyDurationChange()
  // connect(m_player, &PlayerComponent::updateDuration,
  //         this, &MprisComponent::onPlayerDurationChanged);

  // Metadata updates via notifyMetadata()
  // connect(m_player, &PlayerComponent::onMetaData,
  //         this, &MprisComponent::onPlayerMetaData);

  // Shuffle/repeat updates via notifyShuffleChange() / notifyRepeatChange()
  // connect(m_player, &PlayerComponent::shuffleModeChanged,
  //         this, &MprisComponent::onShuffleModeChanged);
  // connect(m_player, &PlayerComponent::repeatModeChanged,
  //         this, &MprisComponent::onRepeatModeChanged);
}

void MprisComponent::disconnectPlayerSignals()
{
  if (!m_player)
    return;

  disconnect(m_player, nullptr, this, nullptr);
}

QString MprisComponent::playbackStatus() const
{
  return m_playbackStatus;
}

double MprisComponent::volume() const
{
  return m_volume;
}

qint64 MprisComponent::position() const
{
  return m_position;
}

QString MprisComponent::loopStatus() const
{
  return m_loopStatus;
}

double MprisComponent::rate() const
{
  return m_rate;
}

bool MprisComponent::shuffle() const
{
  // Only expose shuffle for music content
  return (m_currentMediaType == "Audio") ? m_shuffle : false;
}

bool MprisComponent::canGoNext() const
{
  return m_canGoNext && m_playbackStatus != "Stopped";
}

bool MprisComponent::canGoPrevious() const
{
  return m_canGoPrevious && m_playbackStatus != "Stopped";
}

bool MprisComponent::canPlay() const
{
  // Cannot play when stopped (no active media)
  if (m_playbackStatus == "Stopped")
    return false;

  return m_playbackStatus != "Playing";
}

bool MprisComponent::canPause() const
{
  return m_playbackStatus == "Playing";
}

bool MprisComponent::canSeek() const
{
  // Cannot seek when stopped
  if (m_playbackStatus == "Stopped")
    return false;

  bool result = m_duration > 0;
  qDebug() << "MPRIS: canSeek() called - duration:" << m_duration << "result:" << result;
  return result;
}

bool MprisComponent::canControl() const
{
  // Cannot control when stopped
  if (m_playbackStatus == "Stopped")
    return false;

  // Match Apple's logic: visible for paused/playing/buffering, hidden for finished/canceled/error
  return m_playerState == PlayerComponent::State::paused ||
         m_playerState == PlayerComponent::State::playing ||
         m_playerState == PlayerComponent::State::buffering;
}

// MPRIS Root interface methods
void MprisComponent::Raise()
{
  KonvergoWindow* window = Globals::MainWindow();
  if (window)
  {
    window->raise();
    window->requestActivate();
  }
}

void MprisComponent::Quit()
{
  // We don't allow quit via MPRIS for now
  // qApp->quit();
}

bool MprisComponent::fullscreen() const
{
  KonvergoWindow* window = Globals::MainWindow();
  if (window)
    return window->isFullScreen();
  return false;
}

void MprisComponent::setFullscreen(bool value)
{
  KonvergoWindow* window = Globals::MainWindow();
  if (window)
  {
    bool currentState = window->isFullScreen();
    if (currentState != value)
    {
      window->setFullScreen(value);
      qDebug() << "MPRIS: Fullscreen changed to:" << value;
      emitPropertyChange("org.mpris.MediaPlayer2", "Fullscreen", value);
    }
  }
}

// MPRIS Player interface methods
void MprisComponent::Next()
{
  qDebug() << "MPRIS: Next track requested";
  // Metadata will be updated by onPlayerMetaData when queueMedia is called
  InputComponent::Get().sendAction("next");
}

void MprisComponent::Previous()
{
  qDebug() << "MPRIS: Previous track requested";
  // Metadata will be updated by onPlayerMetaData when queueMedia is called
  InputComponent::Get().sendAction("previous");
}

void MprisComponent::Pause()
{
  if (m_player && canPause())
  {
    m_player->pause();
  }
}

void MprisComponent::PlayPause()
{
  if (m_player)
  {
    if (m_playbackStatus == "Playing")
      m_player->pause();
    else
      m_player->play();
  }
}

void MprisComponent::Stop()
{
  if (m_player)
  {
    m_player->stop();
  }
}

void MprisComponent::Play()
{
  if (m_player && canPlay())
  {
    m_player->play();
  }
}

void MprisComponent::Seek(qint64 offset)
{
  if (!canSeek())
    return;

  // offset is in microseconds
  qint64 newPos = m_position + offset;
  if (newPos < 0)
    newPos = 0;
  if (newPos > m_duration)
    newPos = m_duration;

  // Track the expected position for Seeked signal
  m_seekPending = true;
  m_expectedPosition = newPos;

  // Send seek command via signal to JavaScript (convert to milliseconds)
  qDebug() << "MPRIS: Seek requested to" << (newPos / 1000) << "ms";
  emit seekRequested(newPos / 1000);
}

void MprisComponent::SetPosition(const QDBusObjectPath& trackId, qint64 position)
{
  if (!canSeek())
    return;

  // Verify track ID matches current track
  if (trackId.path() != m_currentTrackId)
    return;

  // position is in microseconds
  if (position >= 0 && position <= m_duration)
  {
    // Track the expected position for Seeked signal
    m_seekPending = true;
    m_expectedPosition = position;

    // Send seek command via signal to JavaScript (convert to milliseconds)
    qDebug() << "MPRIS: SetPosition requested to" << (position / 1000) << "ms";
    emit seekRequested(position / 1000);
  }
}

void MprisComponent::OpenUri(const QString& uri)
{
  // Not implemented - would need to handle Jellyfin URLs
}

void MprisComponent::setVolume(double volume)
{
  // Update local volume state
  m_volume = qBound(0.0, volume, 1.0);

  // Emit signal for JavaScript to handle
  // Convert from 0.0-1.0 to 0-100
  int vol = static_cast<int>(m_volume * 100);
  emit volumeChangeRequested(vol);

  // Don't emit property change here - wait for JS to confirm via notifyVolumeChange
}

void MprisComponent::setLoopStatus(const QString& value)
{
  // Valid MPRIS values: "None", "Track", "Playlist"
  if (value == "None" || value == "Track" || value == "Playlist")
  {
    // Map MPRIS values to Jellyfin web client actions
    QString action;
    if (value == "None")
      action = "repeatnone";
    else if (value == "Track")
      action = "repeatone";
    else if (value == "Playlist")
      action = "repeatall";

    // Send action to web client
    InputComponent::Get().sendAction(action);

    qDebug() << "MPRIS: Loop status change requested:" << value << "via action:" << action;
    // Don't emit here - wait for JavaScript confirmation via notifyRepeatChange
  }
}

void MprisComponent::setRate(double value)
{
  if (!m_player)
    return;

  // Clamp to supported range (0.25 to 2.0)
  value = qBound(0.25, value, 2.0);

  qDebug() << "MPRIS: Setting playback rate to:" << value;
  m_rate = value;
  m_player->setPlaybackRate(static_cast<int>(value * 1000));
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "Rate", value);
}

void MprisComponent::setShuffle(bool value)
{
  // Only support for music content
  if (m_currentMediaType != "Audio")
    return;

  // Send appropriate action to web client
  QString action = value ? "shuffle" : "sorted";
  InputComponent::Get().sendAction(action);

  qDebug() << "MPRIS: Shuffle change requested:" << value << "via action:" << action;
  // Don't emit here - wait for JavaScript confirmation via notifyShuffleChange
}

/////////////////////////////////////////////////////////////////////////////////////////
void MprisComponent::notifyShuffleChange(bool enabled)
{
  qDebug() << "MPRIS: Received shuffle change notification from web client:" << enabled;
  if (m_shuffle != enabled)
  {
    m_shuffle = enabled;
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "Shuffle", shuffle());
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void MprisComponent::notifyRepeatChange(const QString& mode)
{
  qDebug() << "MPRIS: Received repeat change notification from web client:" << mode;

  // Convert Jellyfin repeat mode to MPRIS LoopStatus
  QString loopStatus;
  if (mode == "RepeatAll")
    loopStatus = "Playlist";
  else if (mode == "RepeatOne")
    loopStatus = "Track";
  else
    loopStatus = "None";

  if (m_loopStatus != loopStatus)
  {
    m_loopStatus = loopStatus;
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "LoopStatus", loopStatus);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void MprisComponent::notifyFullscreenChange(bool isFullscreen)
{
  qDebug() << "MPRIS: Received fullscreen change notification from web client:" << isFullscreen;
  emitPropertyChange("org.mpris.MediaPlayer2", "Fullscreen", isFullscreen);
}

void MprisComponent::notifyRateChange(double rate)
{
  qDebug() << "MPRIS: Received playback rate change notification from web client:" << rate;
  if (m_rate != rate)
  {
    m_rate = rate;
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "Rate", rate);
  }
}

void MprisComponent::notifyPlaybackStop(bool isNavigating)
{
  qDebug() << "MPRIS: Playback stop notification - isNavigating:" << isNavigating;
  m_isNavigating = isNavigating;

  // Trigger the stopped state immediately with the correct navigation flag
  // This ensures the flag is set before any other stop handlers run
  notifyPlaybackState("Stopped");
}

void MprisComponent::notifyDurationChange(qint64 durationMs)
{
  qDebug() << "MPRIS: Duration change notification from JS:" << durationMs << "ms";

  // Convert milliseconds to microseconds
  qint64 durationMicroseconds = durationMs * 1000;

  if (m_duration != durationMicroseconds)
  {
    m_duration = durationMicroseconds;
    m_metadata["mpris:length"] = QVariant::fromValue(m_duration);

    // Only emit if already playing
    if (m_playbackStatus == "Playing")
    {
      QVariantMap properties;
      properties["Metadata"] = m_metadata;
      properties["CanSeek"] = canSeek();
      emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
    }
  }
}

void MprisComponent::notifyQueueChange(bool canNext, bool canPrevious)
{
  qDebug() << "MPRIS: Queue change - canNext:" << canNext << "canPrevious:" << canPrevious
           << "status:" << m_playbackStatus << "state:" << (int)m_playerState;

  bool changed = false;

  if (m_canGoNext != canNext)
  {
    m_canGoNext = canNext;
    changed = true;
  }

  if (m_canGoPrevious != canPrevious)
  {
    m_canGoPrevious = canPrevious;
    changed = true;
  }

  if (changed)
  {
    QVariantMap properties;
    properties["CanGoNext"] = m_canGoNext;
    properties["CanGoPrevious"] = m_canGoPrevious;

    // If queue is now empty and we're stopped, clear metadata
    if (!canNext && !canPrevious && m_playbackStatus == "Stopped")
    {
      qDebug() << "MPRIS: Queue empty and stopped, clearing metadata";
      m_metadata.clear();
      m_currentTrackId.clear();
      cleanupAlbumArt();
      properties["Metadata"] = m_metadata;
      properties["Position"] = (qint64)0;
    }

    emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
  }
}

void MprisComponent::notifyPlaybackState(const QString& state)
{
  qDebug() << "MPRIS: Playback state change from JS:" << state;

  if (state == "Playing")
  {
    m_playbackStatus = "Playing";
    m_playerState = PlayerComponent::State::playing;
    updateNavigationCapabilities();
    bool seekable = canSeek();

    QVariantMap properties;
    properties["PlaybackStatus"] = "Playing";
    properties["CanPlay"] = canPlay();
    properties["CanPause"] = canPause();
    properties["CanSeek"] = seekable;
    properties["CanControl"] = canControl();
    properties["Rate"] = m_rate;
    properties["Metadata"] = m_metadata;
    emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);

    if (m_positionTimer)
      m_positionTimer->start();
  }
  else if (state == "Paused")
  {
    m_playbackStatus = "Paused";
    m_playerState = PlayerComponent::State::paused;
    updateNavigationCapabilities();

    QVariantMap properties;
    properties["PlaybackStatus"] = "Paused";
    properties["CanPlay"] = canPlay();
    properties["CanPause"] = canPause();
    properties["CanControl"] = canControl();
    emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);

    if (m_positionTimer)
      m_positionTimer->stop();
  }
  else if (state == "Stopped")
  {
    if (m_positionTimer)
      m_positionTimer->stop();

    m_position = 0;
    m_duration = 0;

    m_playbackStatus = "Stopped";
    m_playerState = PlayerComponent::State::finished;

    // Clear metadata if not navigating (true exit like back button)
    if (!m_isNavigating)
    {
      qDebug() << "MPRIS: Stopped without navigation, clearing metadata";
      m_metadata.clear();
      m_currentTrackId.clear();
      cleanupAlbumArt();
    }
    else
    {
      qDebug() << "MPRIS: Stopped during navigation, keeping metadata for transition";
    }

    // Reset navigation flag
    m_isNavigating = false;

    updateNavigationCapabilities();
    bool seekable = canSeek();

    QVariantMap properties;
    properties["PlaybackStatus"] = "Stopped";
    properties["CanPlay"] = canPlay();
    properties["CanPause"] = canPause();
    properties["CanSeek"] = seekable;
    properties["CanControl"] = canControl();
    properties["Position"] = (qint64)0;
    properties["Metadata"] = m_metadata;  // Include metadata (empty if cleared)
    emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
  }
}

void MprisComponent::notifyPosition(qint64 positionMs)
{
  // Convert milliseconds to microseconds
  m_position = positionMs * 1000;

  // Check if this position update is from a pending seek (MPRIS-initiated)
  if (m_seekPending && m_playerAdaptor) {
    m_seekPending = false;
    qDebug() << "MPRIS: MPRIS-initiated seek completed, emitting Seeked signal at" << m_position << "us";
    Q_EMIT m_playerAdaptor->Seeked(m_position);
  }
}

void MprisComponent::notifySeek(qint64 positionMs)
{
  // Convert milliseconds to microseconds
  m_position = positionMs * 1000;

  // Always emit Seeked signal for player-initiated seeks
  if (m_playerAdaptor) {
    qDebug() << "MPRIS: Player-initiated seek, emitting Seeked signal at" << m_position << "us";
    Q_EMIT m_playerAdaptor->Seeked(m_position);
  }

  // Clear any pending MPRIS seek flag
  m_seekPending = false;
}

void MprisComponent::notifyMetadata(const QVariantMap& metadata, const QString& baseUrl)
{
  qDebug() << "MPRIS: Metadata update from JS";
  updateMetadata(metadata, QUrl(baseUrl));
}

void MprisComponent::notifyVolumeChange(double volume)
{
  double newVolume = qBound(0.0, volume, 1.0);
  if (m_volume != newVolume)
  {
    m_volume = newVolume;
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "Volume", m_volume);
  }
}


// PlayerComponent signal handlers
void MprisComponent::onPlayerPlaying()
{
  qDebug() << "MPRIS: Player playing";

  // Update playback status and emit with metadata
  m_playbackStatus = "Playing";
  updateNavigationCapabilities();
  bool seekable = canSeek();

  QVariantMap properties;
  properties["PlaybackStatus"] = "Playing";
  properties["CanPlay"] = canPlay();
  properties["CanPause"] = canPause();
  properties["CanSeek"] = seekable;
  properties["CanControl"] = canControl();
  properties["Rate"] = m_rate;
  properties["Metadata"] = m_metadata;
  emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);

  if (m_positionTimer)
    m_positionTimer->start();
}

void MprisComponent::onPlayerPaused()
{
  qDebug() << "MPRIS: Player paused";
  updatePlaybackStatus("Paused");
  if (m_positionTimer)
    m_positionTimer->stop();
}

void MprisComponent::onPlayerStopped()
{
  qDebug() << "MPRIS: Player stopped";
  if (m_positionTimer)
    m_positionTimer->stop();

  m_position = 0;
  m_duration = 0;

  // Emit all stopped state changes together
  m_playbackStatus = "Stopped";
  updateNavigationCapabilities();
  bool seekable = canSeek();

  QVariantMap properties;
  properties["PlaybackStatus"] = "Stopped";
  properties["CanPlay"] = canPlay();
  properties["CanPause"] = canPause();
  properties["CanSeek"] = seekable;
  properties["CanControl"] = canControl();
  properties["Position"] = (qint64)0;
  emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
}

void MprisComponent::onPlayerFinished()
{
  updatePlaybackStatus("Stopped");
  if (m_positionTimer)
    m_positionTimer->stop();

  // Playback finished - clear metadata and reset state
  qDebug() << "MPRIS: Playback finished, clearing metadata";
  m_position = 0;
  m_duration = 0;
  m_metadata.clear();
  m_currentTrackId.clear();
  m_canGoNext = false;
  m_canGoPrevious = false;

  cleanupAlbumArt();

  QVariantMap properties;
  properties["Metadata"] = m_metadata;
  properties["Position"] = (qint64)m_position;
  properties["CanGoNext"] = false;
  properties["CanGoPrevious"] = false;
  properties["CanControl"] = canControl();
  emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
}

void MprisComponent::onPlayerStateChanged(PlayerComponent::State newState, PlayerComponent::State oldState)
{
  qDebug() << "MPRIS: State changed from" << (int)oldState << "to" << (int)newState;
  m_playerState = newState;

  // Handle different terminal states
  if (newState == PlayerComponent::State::finished)
  {
    // Finished means EOF of current item - check if there's a next item
    if (!m_canGoNext)
    {
      qDebug() << "MPRIS: Finished with no next item, clearing metadata";
      m_metadata.clear();
      m_currentTrackId.clear();
      cleanupAlbumArt();

      QVariantMap properties;
      properties["Metadata"] = m_metadata;
      emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
    }
    else
    {
      qDebug() << "MPRIS: Finished but next item available, keeping metadata";
      // CanControl already emitted by updatePlaybackStatus
    }
  }
  else if (newState == PlayerComponent::State::canceled)
  {
    // Canceled can happen during navigation (next/prev) or when exiting playback
    // JavaScript tells us via notifyPlaybackStop() which case it is
    if (!m_isNavigating)
    {
      qDebug() << "MPRIS: Canceled (exiting playback), clearing metadata";
      m_metadata.clear();
      m_currentTrackId.clear();
      m_canGoNext = false;
      m_canGoPrevious = false;
      cleanupAlbumArt();

      QVariantMap properties;
      properties["Metadata"] = m_metadata;
      properties["CanGoNext"] = false;
      properties["CanGoPrevious"] = false;
      emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
    }
    else
    {
      qDebug() << "MPRIS: Canceled during navigation, keeping metadata (will be updated by queueMedia)";
      // Metadata will be updated by onPlayerMetaData when queueMedia is called
    }

    // Reset navigation flag
    m_isNavigating = false;
  }
  else if (newState == PlayerComponent::State::error)
  {
    qDebug() << "MPRIS: Clearing metadata due to error state";
    m_metadata.clear();
    m_currentTrackId.clear();
    cleanupAlbumArt();

    QVariantMap properties;
    properties["Metadata"] = m_metadata;
    emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
  }
  // CanControl is now always emitted by updatePlaybackStatus
}

void MprisComponent::onPlayerPositionUpdate(quint64 position)
{
  // position is in milliseconds, MPRIS uses microseconds
  m_position = position * 1000;

  // Check if this position update is from a pending seek
  if (m_seekPending && m_playerAdaptor) {
    m_seekPending = false;
    Q_EMIT m_playerAdaptor->Seeked(m_position);
  }

  // MPRIS spec says don't emit position changes via PropertiesChanged
  // Clients should use Seeked signal or poll the property
}

void MprisComponent::onPlayerDurationChanged(qint64 duration)
{
  // duration is in milliseconds, MPRIS uses microseconds
  qint64 mpvDuration = duration * 1000;

  qDebug() << "MPRIS: MPV duration:" << mpvDuration << "Jellyfin duration:" << m_duration;

  // Update to MPV's precise duration
  m_duration = mpvDuration;

  // Update length in metadata
  m_metadata["mpris:length"] = QVariant::fromValue(m_duration);

  qDebug() << "MPRIS: Duration updated, CanSeek:" << canSeek();

  // Only emit if already playing - otherwise wait for onPlayerPlaying to emit everything together
  if (m_playbackStatus == "Playing")
  {
    QVariantMap properties;
    properties["Metadata"] = m_metadata;
    properties["CanSeek"] = canSeek();
    emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
  }
}

void MprisComponent::onPlayerMetaData(const QVariantMap& metadata, const QUrl& baseUrl)
{
  qDebug() << "MPRIS: Received metadata from PlayerComponent:" << metadata.value("Name").toString();
  updateMetadata(metadata, baseUrl);
}

void MprisComponent::onPlayerVolumeChanged()
{
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "Volume", volume());
}

void MprisComponent::onShuffleModeChanged(bool shuffleEnabled)
{
  // Only update for music content
  if (m_currentMediaType != "Audio")
    return;

  m_shuffle = shuffleEnabled;
  qDebug() << "MPRIS: Shuffle mode changed from web client to:" << shuffleEnabled;
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "Shuffle", shuffle());
}

void MprisComponent::onRepeatModeChanged(const QString& repeatMode)
{
  // Only update for music content
  if (m_currentMediaType != "Audio")
    return;

  // Map Jellyfin repeat modes to MPRIS LoopStatus values
  QString mprisLoopStatus;
  if (repeatMode == "RepeatNone")
    mprisLoopStatus = "None";
  else if (repeatMode == "RepeatOne")
    mprisLoopStatus = "Track";
  else if (repeatMode == "RepeatAll")
    mprisLoopStatus = "Playlist";
  else {
    qWarning() << "MPRIS: Unknown repeat mode from web client:" << repeatMode;
    return;
  }

  m_loopStatus = mprisLoopStatus;
  qDebug() << "MPRIS: Repeat mode changed from web client:" << repeatMode << "-> MPRIS:" << mprisLoopStatus;
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "LoopStatus", loopStatus());
}

void MprisComponent::updatePlaybackStatus(const QString& status)
{
  if (m_playbackStatus != status)
  {
    m_playbackStatus = status;

    // Update navigation capabilities when playback status changes
    updateNavigationCapabilities();
    bool seekable = canSeek();
    qDebug() << "MPRIS: Emitting CanSeek on status change:" << seekable << "status:" << status << "duration:" << m_duration;

    // Emit all properties in single message
    QVariantMap properties;
    properties["PlaybackStatus"] = status;
    properties["CanPlay"] = canPlay();
    properties["CanPause"] = canPause();
    properties["CanSeek"] = seekable;
    properties["CanControl"] = canControl();
    properties["Rate"] = m_rate;
    emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
  }
}

void MprisComponent::updateMetadata(const QVariantMap& jellyfinMeta, const QUrl& baseUrl)
{
  QVariantMap mprisMeta;

  // Generate track ID
  m_currentTrackId = generateTrackId();
  mprisMeta["mpris:trackid"] = QVariant::fromValue(QDBusObjectPath(m_currentTrackId));

  // Map Jellyfin metadata to MPRIS format
  QString mediaType = jellyfinMeta.value("MediaType").toString();
  QString itemType = jellyfinMeta.value("Type").toString();

  // Track current media type for shuffle/repeat availability
  m_currentMediaType = mediaType;

  // Common fields
  if (jellyfinMeta.contains("Name"))
    mprisMeta["xesam:title"] = jellyfinMeta["Name"];

  // Try to get duration from Jellyfin metadata first (RunTimeTicks)
  if (jellyfinMeta.contains("RunTimeTicks"))
  {
    // RunTimeTicks is in 100-nanosecond units, convert to microseconds
    qint64 runTimeTicks = jellyfinMeta["RunTimeTicks"].toLongLong();
    qint64 durationMicroseconds = runTimeTicks / 10;
    m_duration = durationMicroseconds;
    qDebug() << "MPRIS: Got duration from Jellyfin metadata:" << m_duration << "us";
  }

  if (m_duration > 0)
    mprisMeta["mpris:length"] = QVariant::fromValue(m_duration);

  // Handle different media types
  if (mediaType == "Audio" || itemType == "Audio")
  {
    // Music metadata
    if (jellyfinMeta.contains("Artists"))
    {
      QStringList artists = jellyfinMeta["Artists"].toStringList();
      if (!artists.isEmpty())
        mprisMeta["xesam:artist"] = QVariant::fromValue(artists);
    }

    if (jellyfinMeta.contains("AlbumArtist"))
      mprisMeta["xesam:albumArtist"] = QVariant::fromValue(QStringList{jellyfinMeta["AlbumArtist"].toString()});

    if (jellyfinMeta.contains("Album"))
      mprisMeta["xesam:album"] = jellyfinMeta["Album"];

    if (jellyfinMeta.contains("IndexNumber"))
      mprisMeta["xesam:trackNumber"] = jellyfinMeta["IndexNumber"];

    if (jellyfinMeta.contains("ProductionYear"))
      mprisMeta["xesam:contentCreated"] = jellyfinMeta["ProductionYear"].toString();
  }
  else if (itemType == "Episode")
  {
    // TV Show metadata
    QString title = jellyfinMeta.value("Name").toString();
    QString series = jellyfinMeta.value("SeriesName").toString();
    int episode = jellyfinMeta.value("IndexNumber", 0).toInt();

    mprisMeta["xesam:title"] = title;

    if (!series.isEmpty())
      mprisMeta["xesam:artist"] = QVariant::fromValue(QStringList{series});

    if (jellyfinMeta.contains("SeasonName"))
      mprisMeta["xesam:album"] = jellyfinMeta["SeasonName"].toString();

    if (episode > 0)
      mprisMeta["xesam:trackNumber"] = episode;

    if (jellyfinMeta.contains("ProductionYear"))
      mprisMeta["xesam:contentCreated"] = jellyfinMeta["ProductionYear"].toString();
  }
  else if (itemType == "Movie")
  {
    // Movie metadata
    if (jellyfinMeta.contains("ProductionYear"))
      mprisMeta["xesam:contentCreated"] = jellyfinMeta["ProductionYear"].toString();
  }

  // Handle artwork (album art, movie poster, series poster)
  QString artUrl = extractArtworkUrl(jellyfinMeta, baseUrl);
  if (!artUrl.isEmpty())
  {
    QString localArtUrl = handleAlbumArt(artUrl);
    if (!localArtUrl.isEmpty())
    {
      mprisMeta["mpris:artUrl"] = localArtUrl;
    }
  }

  // Always include duration
  mprisMeta["mpris:length"] = QVariant::fromValue(m_duration);

  m_metadata = mprisMeta;

  // Update navigation capabilities based on web playlist
  updateNavigationCapabilities();

  // Emit metadata and properties that changed
  bool seekable = canSeek();
  qDebug() << "MPRIS: Emitting metadata, CanSeek:" << seekable << "duration:" << m_duration;

  QVariantMap properties;
  properties["Metadata"] = m_metadata;

  // Only emit CanSeek if it changed
  static bool lastCanSeek = false;
  if (seekable != lastCanSeek)
  {
    properties["CanSeek"] = seekable;
    lastCanSeek = seekable;
  }

  // Include shuffle/repeat for music content (only if changed)
  if (m_currentMediaType == "Audio") {
    static bool lastShuffle = false;
    static QString lastLoopStatus = "";

    bool currentShuffle = shuffle();
    QString currentLoopStatus = loopStatus();

    if (currentShuffle != lastShuffle)
    {
      properties["Shuffle"] = currentShuffle;
      lastShuffle = currentShuffle;
    }

    if (currentLoopStatus != lastLoopStatus)
    {
      properties["LoopStatus"] = currentLoopStatus;
      lastLoopStatus = currentLoopStatus;
    }
  }

  emitMultiplePropertyChanges("org.mpris.MediaPlayer2.Player", properties);
}

void MprisComponent::emitPropertyChange(const QString& interface,
                                        const QString& property,
                                        const QVariant& value)
{
  QVariantMap changedProps;
  changedProps[property] = value;

  QDBusMessage signal = QDBusMessage::createSignal(
    MPRIS_OBJECT_PATH,
    "org.freedesktop.DBus.Properties",
    "PropertiesChanged"
  );

  signal << interface << changedProps << QStringList();
  QDBusConnection::sessionBus().send(signal);
}

void MprisComponent::emitMultiplePropertyChanges(const QString& interface,
                                                 const QVariantMap& properties)
{
  QDBusMessage signal = QDBusMessage::createSignal(
    MPRIS_OBJECT_PATH,
    "org.freedesktop.DBus.Properties",
    "PropertiesChanged"
  );

  signal << interface << properties << QStringList();
  QDBusConnection::sessionBus().send(signal);
}

QString MprisComponent::generateTrackId() const
{
  static int trackCounter = 0;
  return QString("/org/mpris/MediaPlayer2/Track/%1").arg(++trackCounter);
}

QString MprisComponent::handleAlbumArt(const QString& artUrl)
{
  if (artUrl.isEmpty())
    return QString();

  // For local files, just return the file URL
  if (artUrl.startsWith("file://"))
  {
    // Clean up cached HTTP art if we're switching to local file
    if (!m_currentArtPath.isEmpty())
      cleanupAlbumArt();
    return artUrl;
  }

  // For HTTP URLs, we need to download
  if (artUrl.startsWith("http://") || artUrl.startsWith("https://"))
  {
    // Generate a unique filename based on URL hash
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(artUrl.toUtf8());
    QString filename = QString("%1/%2.jpg").arg(m_albumArtDir)
                                           .arg(hash.result().toHex().constData());

    // Check if already cached
    if (QFile::exists(filename))
    {
      // Only cleanup old art if it's different from this one
      if (m_currentArtPath != filename && !m_currentArtPath.isEmpty())
        cleanupAlbumArt();

      m_currentArtPath = filename;
      return QUrl::fromLocalFile(filename).toString();
    }

    // Check if already downloading this same URL - reuse existing download
    if (m_pendingArtReply && m_pendingArtUrl == artUrl)
    {
      // Same URL already downloading, just wait for it
      return QString();
    }

    // Different URL, cleanup old cached art and abort any pending download
    if (!m_currentArtPath.isEmpty() || m_pendingArtReply)
      cleanupAlbumArt();

    // Start async download
    m_pendingArtPath = filename;
    m_pendingArtUrl = artUrl;

    QNetworkRequest request;
    request.setUrl(QUrl(artUrl));
    request.setRawHeader("User-Agent", "JellyfinMediaPlayer/1.0");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    m_pendingArtReply = m_albumArtManager->get(request);

    // Connect finished signal to handler
    connect(m_pendingArtReply, &QNetworkReply::finished, this, &MprisComponent::onAlbumArtDownloaded);

    // Return empty string for now - metadata will be updated when download completes
    return QString();
  }

  return QString();
}

void MprisComponent::cleanupAlbumArt()
{
  // Abort any pending download
  if (m_pendingArtReply)
  {
    m_pendingArtReply->abort();
    m_pendingArtReply->deleteLater();
    m_pendingArtReply = nullptr;
    m_pendingArtPath.clear();
    m_pendingArtUrl.clear();
  }

  // Remove cached file
  if (!m_currentArtPath.isEmpty())
  {
    QFile::remove(m_currentArtPath);
    m_currentArtPath.clear();
  }
}

void MprisComponent::onAlbumArtDownloaded()
{
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  // Clear the pending reply pointer
  if (reply == m_pendingArtReply)
    m_pendingArtReply = nullptr;

  // Ensure reply is cleaned up
  reply->deleteLater();

  // Check for errors or abort
  if (reply->error() != QNetworkReply::NoError)
  {
    // Don't log if manually aborted (cleanup was called)
    if (reply->error() != QNetworkReply::OperationCanceledError)
      qDebug() << "MPRIS: Album art download failed:" << reply->errorString();
    m_pendingArtPath.clear();
    return;
  }

  QByteArray imageData = reply->readAll();

  // Basic validation - check if we got data
  if (imageData.size() > 0)
  {
    QFile file(m_pendingArtPath);
    if (file.open(QIODevice::WriteOnly))
    {
      file.write(imageData);
      file.close();

      // Set restrictive permissions (owner read/write only)
      file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);

      m_currentArtPath = m_pendingArtPath;
      QString artUrl = QUrl::fromLocalFile(m_currentArtPath).toString();

      // Update metadata with the new album art
      if (!m_metadata.isEmpty())
      {
        m_metadata["mpris:artUrl"] = artUrl;
        emitPropertyChange("org.mpris.MediaPlayer2.Player", "Metadata", m_metadata);
      }

      qDebug() << "MPRIS: Album art downloaded successfully:" << m_currentArtPath;
    }
    else
    {
      qDebug() << "MPRIS: Failed to write album art to:" << m_pendingArtPath;
    }
  }
  else
  {
    qDebug() << "MPRIS: Album art validation failed - empty data";
  }

  m_pendingArtPath.clear();
}

QString MprisComponent::extractArtworkUrl(const QVariantMap& metadata, const QUrl& baseUrl)
{
  if (baseUrl.isEmpty())
    return QString();

  QString mediaType = metadata.value("MediaType").toString();
  QString itemType = metadata.value("Type").toString();

  QUrl artUrl = baseUrl;
  QUrlQuery query;

  // Handle different media types with priority fallback chains
  if (mediaType == "Audio" || itemType == "Audio")
  {
    // MUSIC: Priority order - Album art, then track art
    // 1. Try album art first (most common for music)
    if (metadata.contains("AlbumId") && metadata.contains("AlbumPrimaryImageTag"))
    {
      QString albumId = metadata["AlbumId"].toString();
      QString imageTag = metadata["AlbumPrimaryImageTag"].toString();

      if (!albumId.isEmpty() && !imageTag.isEmpty())
      {
        artUrl.setPath(QString("/Items/%1/Images/Primary").arg(albumId));
        query.addQueryItem("tag", imageTag);
        query.addQueryItem("maxWidth", "512");  // Optimize size for desktop notifications
        artUrl.setQuery(query);
        return artUrl.toString();
      }
    }

    // 2. Fallback to track's own primary image
    auto imageTags = metadata["ImageTags"].toMap();
    if (imageTags.contains("Primary") && metadata.contains("Id"))
    {
      QString itemId = metadata["Id"].toString();
      QString imageTag = imageTags["Primary"].toString();

      if (!itemId.isEmpty() && !imageTag.isEmpty())
      {
        artUrl.setPath(QString("/Items/%1/Images/Primary").arg(itemId));
        query.addQueryItem("tag", imageTag);
        query.addQueryItem("maxWidth", "512");
        artUrl.setQuery(query);
        return artUrl.toString();
      }
    }
  }
  else if (itemType == "Episode")
  {
    // TV EPISODES: Priority order - Series poster, Season poster, Episode thumbnail
    // 1. Try series poster (most recognizable)
    if (metadata.contains("SeriesId") && metadata.contains("SeriesPrimaryImageTag"))
    {
      QString seriesId = metadata["SeriesId"].toString();
      QString imageTag = metadata["SeriesPrimaryImageTag"].toString();

      if (!seriesId.isEmpty() && !imageTag.isEmpty())
      {
        artUrl.setPath(QString("/Items/%1/Images/Primary").arg(seriesId));
        query.addQueryItem("tag", imageTag);
        query.addQueryItem("maxWidth", "512");
        artUrl.setQuery(query);
        return artUrl.toString();
      }
    }

    // 2. Try season poster
    if (metadata.contains("SeasonId") && metadata.contains("SeasonPrimaryImageTag"))
    {
      QString seasonId = metadata["SeasonId"].toString();
      QString imageTag = metadata["SeasonPrimaryImageTag"].toString();

      if (!seasonId.isEmpty() && !imageTag.isEmpty())
      {
        artUrl.setPath(QString("/Items/%1/Images/Primary").arg(seasonId));
        query.addQueryItem("tag", imageTag);
        query.addQueryItem("maxWidth", "512");
        artUrl.setQuery(query);
        return artUrl.toString();
      }
    }

    // 3. Fallback to episode thumbnail
    auto imageTags = metadata["ImageTags"].toMap();
    if (imageTags.contains("Primary") && metadata.contains("Id"))
    {
      QString itemId = metadata["Id"].toString();
      QString imageTag = imageTags["Primary"].toString();

      if (!itemId.isEmpty() && !imageTag.isEmpty())
      {
        artUrl.setPath(QString("/Items/%1/Images/Primary").arg(itemId));
        query.addQueryItem("tag", imageTag);
        query.addQueryItem("maxWidth", "512");
        artUrl.setQuery(query);
        return artUrl.toString();
      }
    }
  }
  else if (itemType == "Movie")
  {
    // MOVIES: Primary poster, fallback to backdrop
    // 1. Movie poster (Primary image)
    auto imageTags = metadata["ImageTags"].toMap();
    if (imageTags.contains("Primary") && metadata.contains("Id"))
    {
      QString itemId = metadata["Id"].toString();
      QString imageTag = imageTags["Primary"].toString();

      if (!itemId.isEmpty() && !imageTag.isEmpty())
      {
        artUrl.setPath(QString("/Items/%1/Images/Primary").arg(itemId));
        query.addQueryItem("tag", imageTag);
        query.addQueryItem("maxWidth", "512");
        artUrl.setQuery(query);
        return artUrl.toString();
      }
    }

    // 2. Fallback to backdrop image
    if (imageTags.contains("Backdrop") && metadata.contains("Id"))
    {
      QString itemId = metadata["Id"].toString();
      QString imageTag = imageTags["Backdrop"].toString();

      if (!itemId.isEmpty() && !imageTag.isEmpty())
      {
        artUrl.setPath(QString("/Items/%1/Images/Backdrop/0").arg(itemId));
        query.addQueryItem("tag", imageTag);
        query.addQueryItem("maxWidth", "512");
        artUrl.setQuery(query);
        return artUrl.toString();
      }
    }
  }
  else
  {
    // GENERIC FALLBACK: Any media type - try primary image
    auto imageTags = metadata["ImageTags"].toMap();
    if (imageTags.contains("Primary") && metadata.contains("Id"))
    {
      QString itemId = metadata["Id"].toString();
      QString imageTag = imageTags["Primary"].toString();

      if (!itemId.isEmpty() && !imageTag.isEmpty())
      {
        artUrl.setPath(QString("/Items/%1/Images/Primary").arg(itemId));
        query.addQueryItem("tag", imageTag);
        query.addQueryItem("maxWidth", "512");
        artUrl.setQuery(query);
        return artUrl.toString();
      }
    }
  }

  return QString(); // No suitable image found
}

void MprisComponent::updateNavigationCapabilities()
{
  // Navigation capabilities are now managed by the web client via notifyQueueChange()
  // This function is kept for compatibility but does nothing
  // The queue state is tracked in JavaScript and reported to MPRIS dynamically
}