#ifndef MPRISCOMPONENT_H
#define MPRISCOMPONENT_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QVariantMap>
#include <QTimer>
#include <memory>

#include "ComponentManager.h"

class PlayerComponent;
class MprisRootAdaptor;
class MprisPlayerAdaptor;

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
  bool componentExport() override { return false; }
  bool componentInitialize() override;
  void componentPostInitialize() override;

  // MPRIS Root interface properties
  bool canQuit() const { return false; }
  bool canRaise() const { return true; }
  bool canSetFullscreen() const { return false; }
  bool fullscreen() const { return false; }
  bool hasTrackList() const { return false; }
  QString identity() const { return "Jellyfin Media Player"; }
  QString desktopEntry() const { return "jellyfinmediaplayer"; }
  QStringList supportedUriSchemes() const { return QStringList(); }
  QStringList supportedMimeTypes() const { return QStringList(); }

  // MPRIS Player interface properties
  QString playbackStatus() const;
  QString loopStatus() const { return "None"; }
  double rate() const { return 1.0; }
  bool shuffle() const { return false; }
  QVariantMap metadata() const { return m_metadata; }
  double volume() const;
  qint64 position() const;
  double minimumRate() const { return 1.0; }
  double maximumRate() const { return 1.0; }
  bool canGoNext() const;
  bool canGoPrevious() const;
  bool canPlay() const;
  bool canPause() const;
  bool canSeek() const;
  bool canControl() const { return true; }

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

private Q_SLOTS:
  // PlayerComponent signal handlers
  void onPlayerPlaying();
  void onPlayerPaused();
  void onPlayerStopped();
  void onPlayerFinished();
  void onPlayerStateChanged(int newState, int oldState);
  void onPlayerPositionUpdate(quint64 position);
  void onPlayerDurationChanged(qint64 duration);
  void onPlayerMetaData(const QVariantMap& metadata, const QUrl& baseUrl);
  void onPlayerVolumeChanged();

Q_SIGNALS:
  // For D-Bus property change notifications
  void propertiesChanged(const QString& interface,
                         const QVariantMap& changedProperties,
                         const QStringList& invalidatedProperties);

private:
  void updatePlaybackStatus(const QString& status);
  void updateMetadata(const QVariantMap& jellyfinMeta, const QUrl& baseUrl = QUrl());
  void emitPropertyChange(const QString& interface, const QString& property, const QVariant& value);
  void connectPlayerSignals();
  void disconnectPlayerSignals();
  QString generateTrackId() const;
  QString handleAlbumArt(const QString& artUrl);
  void cleanupAlbumArt();
  QString extractArtworkUrl(const QVariantMap& metadata, const QUrl& baseUrl);

  bool m_enabled;
  PlayerComponent* m_player;
  std::unique_ptr<MprisRootAdaptor> m_rootAdaptor;
  std::unique_ptr<MprisPlayerAdaptor> m_playerAdaptor;

  QString m_playbackStatus;
  QVariantMap m_metadata;
  qint64 m_position; // in microseconds
  qint64 m_duration; // in microseconds
  QString m_currentTrackId;

  QTimer* m_positionTimer;
  bool m_canGoNext;
  bool m_canGoPrevious;

  QString m_currentArtPath;
  QString m_albumArtDir;
};

#endif // MPRISCOMPONENT_H