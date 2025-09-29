#include "MprisComponent.h"
#include "MprisRootAdaptor.h"
#include "MprisPlayerAdaptor.h"
#include "player/PlayerComponent.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusError>
#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
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
  , m_albumArtDir("/tmp/jellyfin-mpris")
{
  // Create album art directory
  QDir dir;
  dir.mkpath(m_albumArtDir);
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
  if (!QDBusConnection::sessionBus().registerService(MPRIS_SERVICE_NAME))
  {
    qWarning() << "Failed to register MPRIS service:"
               << QDBusConnection::sessionBus().lastError().message();
    return true; // Not fatal, continue without MPRIS
  }

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

  connect(m_player, &PlayerComponent::playing,
          this, &MprisComponent::onPlayerPlaying);
  connect(m_player, &PlayerComponent::paused,
          this, &MprisComponent::onPlayerPaused);
  connect(m_player, &PlayerComponent::stopped,
          this, &MprisComponent::onPlayerStopped);
  connect(m_player, &PlayerComponent::finished,
          this, &MprisComponent::onPlayerFinished);
  connect(m_player, &PlayerComponent::positionUpdate,
          this, &MprisComponent::onPlayerPositionUpdate);
  connect(m_player, &PlayerComponent::updateDuration,
          this, &MprisComponent::onPlayerDurationChanged);
  connect(m_player, &PlayerComponent::onMetaData,
          this, &MprisComponent::onPlayerMetaData);
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
  if (!m_player)
    return 0.0;

  // Convert from 0-100 to 0.0-1.0
  return m_player->volume() / 100.0;
}

qint64 MprisComponent::position() const
{
  return m_position;
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
  return m_playbackStatus != "Playing";
}

bool MprisComponent::canPause() const
{
  return m_playbackStatus == "Playing";
}

bool MprisComponent::canSeek() const
{
  return m_duration > 0 && m_playbackStatus != "Stopped";
}

// MPRIS Root interface methods
void MprisComponent::Raise()
{
  // TODO: Implement window raising when we have access to the main window
  // For now, this is a no-op as KonvergoWindow doesn't expose a singleton
}

void MprisComponent::Quit()
{
  // We don't allow quit via MPRIS for now
  // qApp->quit();
}

// MPRIS Player interface methods
void MprisComponent::Next()
{
  // PlayerComponent doesn't have built-in next/previous
  // This would need to be implemented via web UI integration
  // For now, we report as unsupported
}

void MprisComponent::Previous()
{
  // PlayerComponent doesn't have built-in next/previous
  // This would need to be implemented via web UI integration
  // For now, we report as unsupported
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
  if (!m_player || !canSeek())
    return;

  // offset is in microseconds, seekTo expects milliseconds
  qint64 newPos = m_position + offset;
  if (newPos < 0)
    newPos = 0;
  if (newPos > m_duration)
    newPos = m_duration;

  m_player->seekTo(newPos / 1000);
}

void MprisComponent::SetPosition(const QDBusObjectPath& trackId, qint64 position)
{
  if (!m_player || !canSeek())
    return;

  // Verify track ID matches current track
  if (trackId.path() != m_currentTrackId)
    return;

  // position is in microseconds, seekTo expects milliseconds
  if (position >= 0 && position <= m_duration)
  {
    m_player->seekTo(position / 1000);
  }
}

void MprisComponent::OpenUri(const QString& uri)
{
  // Not implemented - would need to handle Jellyfin URLs
}

void MprisComponent::setVolume(double volume)
{
  if (!m_player)
    return;

  // Convert from 0.0-1.0 to 0-100
  int vol = qBound(0, static_cast<int>(volume * 100), 100);
  m_player->setVolume(vol);

  emitPropertyChange("org.mpris.MediaPlayer2.Player", "Volume", volume);
}

void MprisComponent::setLoopStatus(const QString& value)
{
  // Not supported
}

void MprisComponent::setRate(double value)
{
  // Not supported - playback rate is always 1.0
}

void MprisComponent::setShuffle(bool value)
{
  // Not supported yet
}

// PlayerComponent signal handlers
void MprisComponent::onPlayerPlaying()
{
  updatePlaybackStatus("Playing");
  if (m_positionTimer)
    m_positionTimer->start();
}

void MprisComponent::onPlayerPaused()
{
  updatePlaybackStatus("Paused");
  if (m_positionTimer)
    m_positionTimer->stop();
}

void MprisComponent::onPlayerStopped()
{
  updatePlaybackStatus("Stopped");
  if (m_positionTimer)
    m_positionTimer->stop();

  m_position = 0;
  m_duration = 0;
  m_metadata.clear();
  m_currentTrackId.clear();

  cleanupAlbumArt();

  emitPropertyChange("org.mpris.MediaPlayer2.Player", "Metadata", m_metadata);
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "Position", m_position);
}

void MprisComponent::onPlayerFinished()
{
  updatePlaybackStatus("Stopped");
  if (m_positionTimer)
    m_positionTimer->stop();
}

void MprisComponent::onPlayerStateChanged(int newState, int oldState)
{
  // Additional state handling if needed
}

void MprisComponent::onPlayerPositionUpdate(quint64 position)
{
  // position is in milliseconds, MPRIS uses microseconds
  m_position = position * 1000;

  // MPRIS spec says don't emit position changes via PropertiesChanged
  // Clients should use Seeked signal or poll the property
}

void MprisComponent::onPlayerDurationChanged(qint64 duration)
{
  // duration is in milliseconds, MPRIS uses microseconds
  m_duration = duration * 1000;

  if (m_metadata.contains("mpris:length"))
  {
    m_metadata["mpris:length"] = QVariant::fromValue(m_duration);
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "Metadata", m_metadata);
  }
}

void MprisComponent::onPlayerMetaData(const QVariantMap& metadata, const QUrl& baseUrl)
{
  updateMetadata(metadata, baseUrl);
}

void MprisComponent::onPlayerVolumeChanged()
{
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "Volume", volume());
}

void MprisComponent::updatePlaybackStatus(const QString& status)
{
  if (m_playbackStatus != status)
  {
    m_playbackStatus = status;
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "PlaybackStatus", status);

    // Update can* properties
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "CanPlay", canPlay());
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "CanPause", canPause());
    emitPropertyChange("org.mpris.MediaPlayer2.Player", "CanSeek", canSeek());
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

  // Common fields
  if (jellyfinMeta.contains("Name"))
    mprisMeta["xesam:title"] = jellyfinMeta["Name"];

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
    int season = jellyfinMeta.value("ParentIndexNumber", 0).toInt();
    int episode = jellyfinMeta.value("IndexNumber", 0).toInt();

    if (!series.isEmpty())
    {
      mprisMeta["xesam:album"] = series;
      if (season > 0 && episode > 0)
      {
        title = QString("S%1E%2 - %3").arg(season, 2, 10, QChar('0'))
                                      .arg(episode, 2, 10, QChar('0'))
                                      .arg(title);
      }
    }
    mprisMeta["xesam:title"] = title;
  }
  else if (itemType == "Movie")
  {
    // Movie metadata
    if (jellyfinMeta.contains("ProductionYear"))
    {
      QString year = jellyfinMeta["ProductionYear"].toString();
      QString title = jellyfinMeta.value("Name").toString();
      mprisMeta["xesam:title"] = QString("%1 (%2)").arg(title).arg(year);
    }
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

  m_metadata = mprisMeta;
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "Metadata", m_metadata);

  // Update navigation capabilities based on queue status
  // PlayerComponent doesn't expose queue navigation, so we report as unavailable
  m_canGoNext = false;
  m_canGoPrevious = false;
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "CanGoNext", canGoNext());
  emitPropertyChange("org.mpris.MediaPlayer2.Player", "CanGoPrevious", canGoPrevious());
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

QString MprisComponent::generateTrackId() const
{
  static int trackCounter = 0;
  return QString("/org/mpris/MediaPlayer2/Track/%1").arg(++trackCounter);
}

QString MprisComponent::handleAlbumArt(const QString& artUrl)
{
  if (artUrl.isEmpty())
    return QString();

  // Clean up previous album art
  cleanupAlbumArt();

  // For local files, just return the file URL
  if (artUrl.startsWith("file://"))
    return artUrl;

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
      m_currentArtPath = filename;
      return QUrl::fromLocalFile(filename).toString();
    }

    // Download the image
    QNetworkAccessManager manager;
    QNetworkRequest request;
    request.setUrl(QUrl(artUrl));
    request.setRawHeader("User-Agent", "JellyfinMediaPlayer/1.0");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkReply* reply = manager.get(request);

    // Use event loop for synchronous download with timeout
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(5000); // 5 second timeout

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start();
    loop.exec();

    if (reply->error() == QNetworkReply::NoError && timer.isActive())
    {
      QByteArray imageData = reply->readAll();

      // Basic validation - check if it looks like an image
      if (imageData.size() > 0 && imageData.size() < 10 * 1024 * 1024) // Max 10MB
      {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly))
        {
          file.write(imageData);
          file.close();

          // Set restrictive permissions (owner read/write only)
          file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);

          m_currentArtPath = filename;
          reply->deleteLater();
          return QUrl::fromLocalFile(filename).toString();
        }
      }
    }

    reply->deleteLater();
  }

  return QString();
}

void MprisComponent::cleanupAlbumArt()
{
  if (!m_currentArtPath.isEmpty())
  {
    QFile::remove(m_currentArtPath);
    m_currentArtPath.clear();
  }
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