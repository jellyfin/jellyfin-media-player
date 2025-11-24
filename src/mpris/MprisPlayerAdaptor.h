#ifndef MPRISPLAYERADAPTOR_H
#define MPRISPLAYERADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QVariantMap>

class MprisComponent;

class MprisPlayerAdaptor : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

  // Properties
  Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
  Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
  Q_PROPERTY(double Rate READ rate WRITE setRate)
  Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
  Q_PROPERTY(QVariantMap Metadata READ metadata)
  Q_PROPERTY(double Volume READ volume WRITE setVolume)
  Q_PROPERTY(qint64 Position READ position)
  Q_PROPERTY(double MinimumRate READ minimumRate)
  Q_PROPERTY(double MaximumRate READ maximumRate)
  Q_PROPERTY(bool CanGoNext READ canGoNext)
  Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
  Q_PROPERTY(bool CanPlay READ canPlay)
  Q_PROPERTY(bool CanPause READ canPause)
  Q_PROPERTY(bool CanSeek READ canSeek)
  Q_PROPERTY(bool CanControl READ canControl)

public:
  explicit MprisPlayerAdaptor(MprisComponent* parent);
  ~MprisPlayerAdaptor() override = default;

  // Properties
  QString playbackStatus() const;
  QString loopStatus() const;
  void setLoopStatus(const QString& value);
  double rate() const;
  void setRate(double value);
  bool shuffle() const;
  void setShuffle(bool value);
  QVariantMap metadata() const;
  double volume() const;
  void setVolume(double value);
  qint64 position() const;
  double minimumRate() const;
  double maximumRate() const;
  bool canGoNext() const;
  bool canGoPrevious() const;
  bool canPlay() const;
  bool canPause() const;
  bool canSeek() const;
  bool canControl() const;

public Q_SLOTS:
  // Methods
  void Next();
  void Previous();
  void Pause();
  void PlayPause();
  void Stop();
  void Play();
  void Seek(qint64 offset);
  void SetPosition(const QDBusObjectPath& trackId, qint64 position);
  void OpenUri(const QString& uri);

Q_SIGNALS:
  // Signals
  void Seeked(qint64 position);

private:
  MprisComponent* m_component;
};

#endif // MPRISPLAYERADAPTOR_H