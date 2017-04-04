#ifndef CODECS_H
#define CODECS_H

#include <QObject>
#include <QtCore/qglobal.h>
#include <QList>
#include <QSize>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>
#include <QUrl>
#include <QVariant>
#include <QTime>
#include <QSet>

///////////////////////////////////////////////////////////////////////////////////////////////////
enum class CodecType {
  Decoder,
  Encoder,
};

///////////////////////////////////////////////////////////////////////////////////////////////////
struct CodecDriver {
  CodecType type; // encoder/decoder
  QString format; // e.g. "h264", the canonical FFmpeg name of the codec
  QString driver; // specific implementation, e.g. "h264" (native) or "h264_mf" (MediaFoundation)
  bool present;   // if false, it's a not-installed installable codec
  bool external;  // marked as external in CodecManifest.h

  // Driver name decorated with additional attributes, e.g. "h264_mf_decoder".
  QString getMangledName() const;

  // Filename of the DLL/SO including build version/type, without path.
  // Only applies to external drivers.
  QString getFileName() const;

  // Like getFileName(), but includes full path.
  // Only applies to external drivers.
  QString getPath() const;

  // Return whether the codec is provided by the OS or the hardware. They are
  // distinct from FFmpeg-native codecs and usually have weaker capabilities.
  // While they are also always available, they might fail in various ways
  // depending on input media and OS version.
  bool isSystemCodec() const;

  // Return "mf" for Windows codecs, "at" for OSX audio, "mmal" for RPI video, "eae" for EAE, "" otherwise
  QString getSystemCodecType() const;

  bool isWhitelistedSystemAudioCodec() const;
  bool isWhitelistedSystemVideoCodec() const;

  bool valid() { return format.size() > 0; }
};

struct StreamInfo {
  bool isVideo, isAudio;
  QString codec;
  QString profile;
  int audioChannels;
  int audioSampleRate;
  QSize videoResolution;
};

struct PlaybackInfo {
  QList<StreamInfo> streams;            // information for _all_ streams the file has
                                        // (even if not selected)
  QSet<QString> audioPassthroughCodecs; // list of audio formats to pass through
  bool enableAC3Transcoding;            // encode non-stereo to AC3
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class Downloader : public QObject
{
  Q_OBJECT
public:
  typedef QList<QPair<QString, QString>> HeaderList;
  explicit Downloader(QVariant userData, const QUrl& url, const HeaderList& headers, QObject* parent);
Q_SIGNALS:
  void done(QVariant userData, bool success, const QByteArray& data);

private Q_SLOTS:
  void networkFinished(QNetworkReply* pReply);
  void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
  QNetworkAccessManager m_WebCtrl;
  QByteArray m_DownloadedData;
  QVariant m_userData;
  QTime m_currentStartTime;
  int m_lastProgress;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class CodecsFetcher : public QObject
{
  Q_OBJECT
public:
  CodecsFetcher()
  : startCodecs(true), m_eaeNeeded(false), m_fetchEAE(false)
  {
  }

  // Download the given list of codecs (skip download for codecs already
  // installed). Then call done(userData), regardless of success.
  void installCodecs(const QList<CodecDriver>& codecs);

  // For free use by the user of this object.
  QVariant userData;

  bool startCodecs;

Q_SIGNALS:
  void done(CodecsFetcher* sender);

private Q_SLOTS:
  void codecInfoDownloadDone(QVariant userData, bool success, const QByteArray& data);
  void codecDownloadDone(QVariant userData, bool success, const QByteArray& data);

private:
  bool codecNeedsDownload(const CodecDriver& codec);
  bool processCodecInfoReply(const QVariant& context, const QByteArray& data);
  void processCodecDownloadDone(const QVariant& context, const QByteArray& data);
  void startNext();
  void startEAE();

  QQueue<CodecDriver> m_Codecs;
  QByteArray m_currentHash;
  bool m_eaeNeeded;
  bool m_fetchEAE;
};

class Codecs
{
public:
  static void preinitCodecs();

  static void initCodecs();

  static QString plexNameToFF(QString plex);

  static QString plexNameFromFF(QString ffname);

  static inline bool sameCodec(const CodecDriver& a, const CodecDriver& b)
  {
    return a.type == b.type && a.format == b.format && a.driver == b.driver;
  }

  static void updateCachedCodecList();

  static void Uninit();

  static const QList<CodecDriver>& getCachedCodecList();

  static QList<CodecDriver> findCodecsByFormat(const QList<CodecDriver>& list, CodecType type, const QString& format);
  static QList<CodecDriver> determineRequiredCodecs(const PlaybackInfo& info);
};

#endif // CODECS_H
