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

class QNetworkAccessManager;
class QNetworkReply;

#include "MprisRootAdaptor.h"
#include "MprisPlayerAdaptor.h"

class MprisComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(MprisComponent);

public:
  explicit MprisComponent(QObject* parent = nullptr);
  ~MprisComponent() override;

  const char* componentName() override { return "mpris"; }
  bool componentExport() override { return false; }
  bool componentInitialize() override;
  void componentPostInitialize() override;

  bool canQuit() const { return false; }
  bool canRaise() const { return true; }
  bool canSetFullscreen() const { return true; }
  bool fullscreen() const;
  void setFullscreen(bool value);
  bool hasTrackList() const { return false; }
  QString identity() const { return "Jellyfin"; }
  QString desktopEntry() const { return "org.jellyfin.JellyfinDesktop"; }
  QStringList supportedUriSchemes() const { return QStringList(); }
  QStringList supportedMimeTypes() const { return QStringList(); }

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

  // Notification slots - connected to PlayerComponent signals
  void notifyShuffleChange(bool enabled);
  void notifyRepeatChange(const QString& mode);
  void notifyFullscreenChange(bool isFullscreen);
  void notifyRateChange(double rate);
  void notifyQueueChange(bool canNext, bool canPrevious);
  void notifyPlaybackStop(bool isNavigating);
  void notifyDurationChange(qint64 durationMs);
  void notifyPlaybackState(const QString& state);
  void notifyPosition(qint64 positionMs);
  void notifySeek(qint64 positionMs);
  void notifyMetadata(const QVariantMap& metadata, const QString& baseUrl);
  void notifyVolumeChange(double volume);

public Q_SLOTS:
  // MPRIS D-Bus method handlers - called from D-Bus clients
  void Raise();
  void Quit();

  void Next();
  void Previous();
  void Pause();
  void PlayPause();
  void Stop();
  void Play();
  void Seek(qint64 offset);
  void SetPosition(const QDBusObjectPath& trackId, qint64 position);
  void OpenUri(const QString& uri);

  void setVolume(double volume);
  void setLoopStatus(const QString& value);
  void setRate(double value);
  void setShuffle(bool value);

private Q_SLOTS:
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

  void onAlbumArtDownloaded();

Q_SIGNALS:
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
  QString m_serviceName;
  PlayerComponent* m_player;
  std::unique_ptr<MprisRootAdaptor> m_rootAdaptor;
  std::unique_ptr<MprisPlayerAdaptor> m_playerAdaptor;

  QString m_playbackStatus;
  QVariantMap m_metadata;
  qint64 m_position;
  qint64 m_duration;
  QString m_currentTrackId;
  QString m_currentMediaType;

  QTimer* m_positionTimer;
  bool m_canGoNext;
  bool m_canGoPrevious;

  bool m_shuffle;
  QString m_loopStatus;
  double m_rate;
  double m_volume;

  bool m_seekPending;
  qint64 m_expectedPosition;

  bool m_isNavigating;

  QString m_currentArtDataUri;
  QString m_pendingArtUrl;
  PlayerComponent::State m_playerState;

  QNetworkAccessManager* m_albumArtManager;
  QNetworkReply* m_pendingArtReply;
};

#endif // MPRISCOMPONENT_H