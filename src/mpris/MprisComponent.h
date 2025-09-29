#ifndef MPRISCOMPONENT_H
#define MPRISCOMPONENT_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QVariantMap>
#include <QTimer>
#include <memory>

#include "ComponentManager.h"
#include "player/PlayerComponent.h"
class MprisRootAdaptor;
class MprisPlayerAdaptor;
class QNetworkAccessManager;
class QNetworkReply;

// MPRIS2 Component for D-Bus media player integration on Linux
// Exposes Jellyfin Media Player to desktop environments via the
// Media Player Remote Interfacing Specification (MPRIS)
class MprisComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(MprisComponent);

public:
  explicit MprisComponent(QObject* parent = nullptr);
  ~MprisComponent() override;

  // Component interface
  const char* componentName() override { return "mpris"; }
  bool componentExport() override { return true; }
  bool componentInitialize() override;
  void componentPostInitialize() override;

  // MPRIS Root interface properties
  bool canQuit() const { return false; }
  bool canRaise() const { return true; }
  bool canSetFullscreen() const { return true; }
  bool fullscreen() const;
  void setFullscreen(bool value);
  bool hasTrackList() const { return false; }
  QString identity() const { return "Jellyfin Media Player"; }
  QString desktopEntry() const { return "jellyfinmediaplayer"; }
  QStringList supportedUriSchemes() const { return QStringList(); }
  QStringList supportedMimeTypes() const { return QStringList(); }

  // MPRIS Player interface properties
  QString playbackStatus() const;
  QString loopStatus() const;
  double rate() const;
  bool shuffle() const;
  QVariantMap metadata() const { return m_metadata; }
  double volume() const;
  qint64 position() const;
  double minimumRate() const { return 0.25; }
  double maximumRate() const { return 2.0; }
  bool canGoNext() const;
  bool canGoPrevious() const;
  bool canPlay() const;
  bool canPause() const;
  bool canSeek() const;
  bool canControl() const;

Q_SIGNALS:
  void volumeChangeRequested(int volume); // 0-100
  void seekRequested(qint64 positionMs);

public Q_SLOTS:
  // MPRIS Root interface methods
  void Raise();
  void Quit();

  // MPRIS Player interface methods
  void Next();
  void Previous();
  void Pause();
  void PlayPause();
  void Stop();
  void Play();
  void Seek(qint64 offset);
  void SetPosition(const QDBusObjectPath& trackId, qint64 position);
  void OpenUri(const QString& uri);

  // Property setters
  void setVolume(double volume);
  void setLoopStatus(const QString& value);
  void setRate(double value);
  void setShuffle(bool value);

  // Invokable methods for JS to notify mode changes
  Q_INVOKABLE void notifyShuffleChange(bool enabled);
  Q_INVOKABLE void notifyRepeatChange(const QString& mode);
  Q_INVOKABLE void notifyFullscreenChange(bool isFullscreen);
  Q_INVOKABLE void notifyRateChange(double rate);
  Q_INVOKABLE void notifyQueueChange(bool canNext, bool canPrevious);
  Q_INVOKABLE void notifyPlaybackStop(bool isNavigating);
  Q_INVOKABLE void notifyDurationChange(qint64 durationMs);
  Q_INVOKABLE void notifyPlaybackState(const QString& state);
  Q_INVOKABLE void notifyPosition(qint64 positionMs);
  Q_INVOKABLE void notifySeek(qint64 positionMs);
  Q_INVOKABLE void notifyMetadata(const QVariantMap& metadata, const QString& baseUrl);
  Q_INVOKABLE void notifyVolumeChange(double volume);

private Q_SLOTS:
  // PlayerComponent signal handlers
  void onPlayerPlaying();
  void onPlayerPaused();
  void onPlayerStopped();
  void onPlayerFinished();
  void onPlayerStateChanged(PlayerComponent::State newState, PlayerComponent::State oldState);
  void onPlayerPositionUpdate(quint64 position);
  void onPlayerDurationChanged(qint64 duration);
  void onPlayerMetaData(const QVariantMap& metadata, const QUrl& baseUrl);
  void onPlayerVolumeChanged();
  void onShuffleModeChanged(bool shuffleEnabled);
  void onRepeatModeChanged(const QString& repeatMode);

  // Album art download handler
  void onAlbumArtDownloaded();

Q_SIGNALS:
  // For D-Bus property change notifications
  void propertiesChanged(const QString& interface,
                         const QVariantMap& changedProperties,
                         const QStringList& invalidatedProperties);

private:
  void updatePlaybackStatus(const QString& status);
  void updateMetadata(const QVariantMap& jellyfinMeta, const QUrl& baseUrl = QUrl());
  void emitPropertyChange(const QString& interface, const QString& property, const QVariant& value);
  void emitMultiplePropertyChanges(const QString& interface, const QVariantMap& properties);
  void connectPlayerSignals();
  void disconnectPlayerSignals();
  QString generateTrackId() const;
  QString handleAlbumArt(const QString& artUrl);
  void cleanupAlbumArt();
  QString extractArtworkUrl(const QVariantMap& metadata, const QUrl& baseUrl);
  void updateNavigationCapabilities();

  bool m_enabled;
  PlayerComponent* m_player;
  std::unique_ptr<MprisRootAdaptor> m_rootAdaptor;
  std::unique_ptr<MprisPlayerAdaptor> m_playerAdaptor;

  QString m_playbackStatus;
  QVariantMap m_metadata;
  qint64 m_position; // in microseconds
  qint64 m_duration; // in microseconds
  QString m_currentTrackId;
  QString m_currentMediaType; // "Audio", "Video", etc.

  QTimer* m_positionTimer;
  bool m_canGoNext;
  bool m_canGoPrevious;

  // Music-specific controls
  bool m_shuffle;
  QString m_loopStatus; // "None", "Track", "Playlist"
  double m_rate; // Playback rate
  double m_volume; // Volume 0.0-1.0

  // Track seek operations for Seeked signal
  bool m_seekPending;
  qint64 m_expectedPosition;

  bool m_isNavigating; // True if stopping for navigation, false if truly exiting

  QString m_currentArtPath;
  QString m_albumArtDir;
  QString m_pendingArtPath; // Destination path for current download
  QString m_pendingArtUrl; // URL being downloaded
  PlayerComponent::State m_playerState;

  QNetworkAccessManager* m_albumArtManager;
  QNetworkReply* m_pendingArtReply; // Active download to abort if needed
};

#endif // MPRISCOMPONENT_H