#include "CodecsComponent.h"
#include <QString>
#include <Qt>
#include <QDir>
#include <QDomAttr>
#include <QDomDocument>
#include <QDomNode>
#include <QCoreApplication>
#include <QUuid>
#include <QUrl>
#include <QUrlQuery>
#include <QResource>
#include <QStandardPaths>
#include <QSysInfo>
#include "system/SystemComponent.h"
#include "settings/SettingsComponent.h"
#include "utils/Utils.h"
#include "shared/Paths.h"
#include "PlayerComponent.h"

#include "QsLog.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

// For QVariant. Mysteriously makes Qt happy.
Q_DECLARE_METATYPE(CodecDriver);

#ifdef HAVE_CODEC_MANIFEST
#include "CodecManifest.h"
#else
#define CODEC_VERSION   "dummy"
#define SHLIB_PREFIX    ""
#define SHLIB_EXTENSION "dummy"
// Codec.name is the name of the codec implementation, Codec.codecName the name of the codec
struct Codec {const char* name; const char* codecName; const char* profiles; int external;};
static const Codec Decoders[] = {
    {"dummy", "dummy", nullptr, 1},
};
static const Codec Encoders[] = {
    {"dummy", "dummy", nullptr, 1},
};
#endif

// We might want to use Codec.quality to decide this one day.
// But for now, it's better if we can quickly change these.
static QSet<QString> g_systemVideoDecoderWhitelist = {
  // RPI
  "h264_mmal",
  "mpeg2_mmal",
  "mpeg4_mmal",
  "vc1_mmal",
  // Windows
  "h264_mf",
  "hevc_mf",
  "vc1_mf",
  "wmv1_mf",
  "wmv2_mf",
  "wmv3_mf",
  "mpeg4_mf",
  "msmpeg4v1_mf",
  "msmpeg4v2_mf",
  "msmpeg4v3_mf",
};

static QSet<QString> g_systemAudioDecoderWhitelist = {
  // OSX
  "aac_at",
  "ac3_at",
  "mp1_at",
  "mp2_at",
  "mp3_at",
  // Windows
  "ac3_mf",
  "eac3_mf",
  "aac_mf",
  "mp1_mf",
  "mp2_mf",
  "mp3_mf",
};

static QSet<QString> g_systemAudioEncoderWhitelist = {
};

static QSize g_mediaFoundationH264MaxResolution;

static QString g_codecVersion;
static QList<CodecDriver> g_cachedCodecList;

static QString g_deviceID;

///////////////////////////////////////////////////////////////////////////////////////////////////
static QString getBuildType()
{
#ifdef Q_OS_MAC
  return "darwin-x86_64";
#elif defined(TARGET_RPI)
  return "openelec-armv7";
#else
  return SystemComponent::Get().getPlatformTypeString() + "-" +
         SystemComponent::Get().getPlatformArchString();
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString Codecs::plexNameToFF(QString plex)
{
  if (plex == "dca")
    return "dts";
  return plex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString Codecs::plexNameFromFF(QString ffname)
{
  if (ffname == "dts")
    return "dca";
  return ffname;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static QString codecsRootPath()
{
  return Paths::dataDir("Codecs") + QDir::separator();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static QString codecsPath()
{
  return codecsRootPath() + g_codecVersion + "-" + getBuildType() + QDir::separator();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static int indexOfCodecInList(const QList<CodecDriver>& list, const CodecDriver& codec)
{
  for (int n = 0; n < list.size(); n++)
  {
    if (Codecs::sameCodec(list[n], codec))
      return n;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Codecs::updateCachedCodecList()
{
  g_cachedCodecList.clear();

  for (CodecType type : {CodecType::Decoder, CodecType::Encoder})
  {
    const Codec* list = (type == CodecType::Decoder) ? Decoders : Encoders;
    size_t count = (type == CodecType::Decoder) ? countof(Decoders) : countof(Encoders);

    for (size_t i = 0; i < count; i++)
    {
      CodecDriver codec = {};
      codec.type = type;
      codec.format = Codecs::plexNameToFF(list[i].codecName);
      codec.driver = list[i].name;
      codec.external = list[i].external;
      if (!codec.isSystemCodec())
        g_cachedCodecList.append(codec);
    }
  }

  // Set present flag for the installed codecs. Also, there could be codecs not
  // on the CodecManifest.h list (system codecs, or when compiled  without
  // codec loading).

  QList<CodecDriver> installed = PlayerComponent::Get().installedCodecDrivers();

  // Surely O(n^2) won't be causing trouble, right?
  for (const CodecDriver& installedCodec : installed)
  {
    int index = indexOfCodecInList(g_cachedCodecList, installedCodec);
    if (index >= 0)
      g_cachedCodecList[index].present = true;
    else
      g_cachedCodecList.append(installedCodec);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const QList<CodecDriver>& Codecs::getCachedCodecList()
{
  return g_cachedCodecList;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QList<CodecDriver> Codecs::findCodecsByFormat(const QList<CodecDriver>& list, CodecType type, const QString& format)
{
  QList<CodecDriver> result;
  for (const CodecDriver& codec : list)
  {
    if (codec.type == type && codec.format == format)
      result.append(codec);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString CodecDriver::getMangledName() const
{
  return driver + (type == CodecType::Decoder ? "_decoder" : "_encoder");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString CodecDriver::getFileName() const
{
  return SHLIB_PREFIX + getMangledName() + "." + SHLIB_EXTENSION;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString CodecDriver::getPath() const
{
  return QDir(codecsPath()).absoluteFilePath(getFileName());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CodecDriver::isSystemCodec() const
{
  // MS Windows
  if (driver.endsWith("_mf"))
    return true;
  // OSX
  if (driver.endsWith("_at"))
    return true;
  // Linux on RPI
  if (driver.endsWith("_mmal"))
    return true;
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CodecDriver::isWhitelistedSystemAudioCodec() const
{
  if (type == CodecType::Decoder)
    return g_systemAudioDecoderWhitelist.contains(driver);
  else
    return g_systemAudioEncoderWhitelist.contains(driver);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CodecDriver::isWhitelistedSystemVideoCodec() const
{
  if (type == CodecType::Decoder)
    return g_systemVideoDecoderWhitelist.contains(driver);
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static bool useSystemAudioDecoders()
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static bool useSystemVideoDecoders()
{
  return SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "useSystemVideoCodecs").toBool();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load the device ID, do some minimal verification, return "" on error.
static QString loadDeviceID(QString filename)
{
  QFile path(filename);
  if (!path.open(QFile::ReadOnly))
    return "";
  auto res = QString::fromLatin1(path.readAll());
  if (res.size() < 32 || res.size() > 512)
    res = ""; // mark as invalid
  return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
static QString findOldDeviceID()
{
  // First we try to reuse the ID from other Plex products (i.e. PMS) or older paths.
  QStringList candidates = {
#ifdef Q_OS_MAC
    QDir::home().path() + "/Library/Application Support/Plex/Codecs/.device-id",
    QDir::home().path() + "/Library/Application Support/Plex Media Server/Codecs/.device-id",
#endif
    QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Plex/Codecs/.device-id",
    QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Plex/codecs/.device-id",
    QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Plex Media Server/Codecs/.device-id",
    Paths::dataDir() + "/codecs/.device-id",
  };

  for (auto candidate : candidates)
  {
    auto id = loadDeviceID(candidate);
    if (!id.isEmpty())
      return id;
  }

  return "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Returns "" on error.
static QString loadDeviceID()
{
  QString deviceIDFilename = QDir(codecsRootPath()).absoluteFilePath(".device-id");

  QString id = loadDeviceID(deviceIDFilename);
  if (id.isEmpty())
  {
    id = findOldDeviceID();
    if (id.isEmpty())
    {
      id = QUuid::createUuid().toString();
      // The UUID should be e.g. "8f6ad954-0cb9-4dbb-a5e5-e0b085f07cf8"
      if (id.startsWith("{"))
        id = id.mid(1);
      if (id.endsWith("}"))
        id = id.mid(0, id.size() - 1);
    }

    Utils::safelyWriteFile(deviceIDFilename, id.toLatin1());

    // We load it again to make sure writing it succeeded. If it doesn't, we'll
    // error out at a later point.
    id = loadDeviceID(deviceIDFilename);
  }

  return id;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static QString getFFmpegVersion()
{
  auto mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
  if (!mpv || mpv_initialize(mpv) < 0)
    return "";
  return mpv::qt::get_property_variant(mpv, "ffmpeg-version").toString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Codecs::preinitCodecs()
{
  // Extract the CI codecs version we set with --extra-version when compiling FFmpeg.
  QString ffmpegVersion = getFFmpegVersion();
  int sep = ffmpegVersion.indexOf(',');
  if (sep >= 0)
    g_codecVersion = ffmpegVersion.mid(sep + 1);
  else
    g_codecVersion = CODEC_VERSION;

  QString path = codecsPath();

  QDir("").mkpath(path);

  // Follows the convention used by av_get_token().
  QString escapedPath = path.replace("\\", "\\\\").replace(":", "\\:");
  // This must be run before any threads are started etc. (for safety).
#ifdef Q_OS_WIN
  SetEnvironmentVariableW(L"FFMPEG_EXTERNAL_LIBS", escapedPath.toStdWString().c_str());
#else
  qputenv("FFMPEG_EXTERNAL_LIBS", escapedPath.toUtf8().data());
#endif

  g_deviceID = loadDeviceID();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static bool probeDecoder(QString decoder, QString resourceName)
{
  QResource resource(resourceName);

  QLOG_DEBUG() << "Testing decoding of" << resource.fileName();

  if (!resource.isValid())
    return false;

  auto mpv = mpv::qt::Handle::FromRawHandle(mpv_create());

  // Disable any output.
  mpv::qt::set_option_variant(mpv, "vo", "null");

  if (!mpv || mpv_initialize(mpv) < 0)
    return false;

  // Force the decoder. The ",-" means that if the first entry fails, the next codec in the global
  // codec list will not be tried, and decoding fails.
  mpv::qt::set_option_variant(mpv, "vd", "lavc:" + decoder + ",-");

  // Attempt decoding, and return success.
  auto data = QByteArray::fromRawData((const char *)resource.data(), resource.size());
  if (resource.isCompressed())
    data = qUncompress(data);
  auto hex = data.toHex();
  mpv::qt::command_variant(mpv, QVariantList{"loadfile", "hex://" + QString::fromLatin1(hex)});
  bool result = false;
  while (1) {
    mpv_event *event = mpv_wait_event(mpv, 0);
    if (event->event_id == MPV_EVENT_SHUTDOWN)
      break;
    if (event->event_id == MPV_EVENT_END_FILE)
    {
      mpv_event_end_file *endFile = (mpv_event_end_file *)event->data;
      result = endFile->reason == MPV_END_FILE_REASON_EOF;
      break;
    }
  }

  QLOG_DEBUG() << "Result:" << result;

  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Codecs::probeCodecs()
{
  if (g_deviceID.isEmpty())
    throw FatalException("Could not read device-id.");

#ifdef Q_OS_WIN32
  if (useSystemVideoDecoders())
  {
    if (probeDecoder("h264_mf", ":/testmedia/high_4096x2304.h264"))
      g_mediaFoundationH264MaxResolution = QSize(4096, 2304);
    else if (probeDecoder("h264_mf", ":/testmedia/high_4096x2160.h264"))
      g_mediaFoundationH264MaxResolution = QSize(4096, 2160);
    else if (probeDecoder("h264_mf", ":/testmedia/high_4096x1080.h264"))
      g_mediaFoundationH264MaxResolution = QSize(4096, 1080);
    else
      g_systemVideoDecoderWhitelist.remove("h264_mf");

    QLOG_DEBUG() << "h264_mf max. resolution:" << g_mediaFoundationH264MaxResolution;
  }
#endif

#ifdef Q_OS_MAC
  // Unsupported, but avoid picking up broken Perian decoders.
  if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_10)
    g_systemAudioDecoderWhitelist.remove("ac3_at");
#endif

  Codecs::updateCachedCodecList();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CodecsFetcher::codecNeedsDownload(const CodecDriver& codec)
{
  if (codec.present)
    return false;
  if (!codec.external)
  {
    QLOG_ERROR() << "Codec" << codec.driver << "does not exist and is not downloadable.";
    return false;
  }
  for (int n = 0; n < m_Codecs.size(); n++)
  {
    if (Codecs::sameCodec(codec, m_Codecs[n]))
      return false;
  }
  QFile codecFile(codec.getPath());
  if (codecFile.exists())
  {
    QLOG_ERROR() << "Codec" << codec.driver << "exists on disk as" << codec.getPath()
                 << "but is not known as installed - broken codec?";
    if (!codecFile.remove())
      return false;
    QLOG_ERROR() << "Retrying download.";
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CodecsFetcher::installCodecs(const QList<CodecDriver>& codecs)
{
  foreach (CodecDriver codec, codecs)
  {
    if (codecNeedsDownload(codec))
      m_Codecs.enqueue(codec);
  }
  startNext();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static Downloader::HeaderList getPlexHeaders()
{
  Downloader::HeaderList headers;
  QString auth = SystemComponent::Get().authenticationToken();
  if (auth.size())
    headers.append({"X-Plex-Token", auth});
  headers.append({"X-Plex-Product", "Plex Media Player"});
  headers.append({"X-Plex-Platform", "Konvergo"});
  return headers;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CodecsFetcher::startNext()
{
  if (m_Codecs.isEmpty())
  {
    emit done(this);
    return;
  }

  CodecDriver codec = m_Codecs.dequeue();

  QString host = "https://plex.tv";
  QUrl url = QUrl(host + "/api/codecs/" + codec.getMangledName());
  QUrlQuery query;
  query.addQueryItem("deviceId", g_deviceID);
  query.addQueryItem("version", g_codecVersion);
  query.addQueryItem("build", getBuildType());
  query.addQueryItem("oldestPreviousVersion", SettingsComponent::Get().oldestPreviousVersion());
  url.setQuery(query);

  Downloader *downloader = new Downloader(QVariant::fromValue(codec), url, getPlexHeaders(), this);
  connect(downloader, &Downloader::done, this, &CodecsFetcher::codecInfoDownloadDone);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CodecsFetcher::processCodecInfoReply(const QByteArray& data, const CodecDriver& codec)
{
  QLOG_INFO() << "Got reply:" << QString::fromUtf8(data);

  QDomDocument dom;
  if (!dom.setContent(data))
  {
    QLOG_ERROR() << "XML parsing error.";
    return false;
  }

  QDomNodeList list = dom.elementsByTagName("MediaContainer");
  if (list.count() != 1)
  {
    QLOG_ERROR() << "MediaContainer XML element not found.";
    return false;
  }
  list = dom.elementsByTagName("Codec");
  if (list.count() != 1)
  {
    QLOG_ERROR() << "Codec XML element not found.";
    return false;
  }

  QDomNamedNodeMap attrs = list.at(0).attributes();
  QString url = attrs.namedItem("url").toAttr().value();
  if (!url.size())
  {
    QLOG_ERROR() << "No URL found.";
    return false;
  }

  QString hash = attrs.namedItem("fileSha").toAttr().value();
  m_currentHash = QByteArray::fromHex(hash.toUtf8());
  // it's hardcoded to SHA-1
  if (!m_currentHash.size()) {
    QLOG_ERROR() << "Hash value in unexpected format or missing:" << hash;
    return false;
  }

  Downloader *downloader = new Downloader(QVariant::fromValue(codec), url, getPlexHeaders(), this);
  connect(downloader, &Downloader::done, this, &CodecsFetcher::codecDownloadDone);

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CodecsFetcher::codecInfoDownloadDone(QVariant userData, bool success, const QByteArray& data)
{
  CodecDriver codec = userData.value<CodecDriver>();
  if (!success || !processCodecInfoReply(data, codec))
  {
    QLOG_ERROR() << "Codec download failed.";
    startNext();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CodecsFetcher::processCodecDownloadDone(const QByteArray& data, const CodecDriver& codec)
{
  QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha1);

  if (hash != m_currentHash)
  {
    QLOG_ERROR() << "Checksum mismatch: got" << hash.toHex() << "expected" << m_currentHash.toHex();
    return;
  }

  QLOG_INFO() << "Storing codec as" << codec.getPath();

  if (!Utils::safelyWriteFile(codec.getPath(), data))
  {
    QLOG_ERROR() << "Writing codec file failed.";
    return;
  }

  // This causes libmpv and eventually libavcodec to rescan and load new codecs.
  Codecs::updateCachedCodecList();
  for (const CodecDriver& item : Codecs::getCachedCodecList())
  {
    if (Codecs::sameCodec(item, codec) && !item.present)
    {
      QLOG_ERROR() << "Codec could not be loaded after installing it.";
      return;
    }
  }

  QLOG_INFO() << "Codec download and installation succeeded.";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CodecsFetcher::codecDownloadDone(QVariant userData, bool success, const QByteArray& data)
{
  CodecDriver codec = userData.value<CodecDriver>();
  QLOG_INFO() << "Codec" << codec.driver << "request finished.";
  if (success)
  {
    processCodecDownloadDone(data, codec);
  }
  else
  {
    QLOG_ERROR() << "Codec download HTTP request failed.";
  }
  startNext();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Downloader::Downloader(QVariant userData, const QUrl& url, const HeaderList& headers, QObject* parent)
  : QObject(parent), m_userData(userData), m_lastProgress(-1)
{
  QLOG_INFO() << "HTTP request:" << url.toDisplayString();
  m_currentStartTime.start();

  connect(&m_WebCtrl, &QNetworkAccessManager::finished, this, &Downloader::networkFinished);

  QNetworkRequest request(url);
  for (int n = 0; n < headers.size(); n++)
    request.setRawHeader(headers[n].first.toUtf8(), headers[n].second.toUtf8());
  QNetworkReply *reply = m_WebCtrl.get(request);
  if (reply)
  {
    connect(reply, &QNetworkReply::downloadProgress, this, &Downloader::downloadProgress);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Downloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  if (bytesTotal > 0)
  {
    int progress = bytesReceived * 100 / bytesTotal;
    if (m_lastProgress < 0 || progress > m_lastProgress + 10)
    {
      m_lastProgress = progress;
      QLOG_INFO() << "HTTP request at" << progress << "% (" << bytesReceived << "/" << bytesTotal << ")";
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Downloader::networkFinished(QNetworkReply* pReply)
{
  QLOG_INFO() << "HTTP finished after" << (m_currentStartTime.elapsed() + 500) / 1000
              << "seconds for a request of" << pReply->size() << "bytes.";

  if (pReply->error() == QNetworkReply::NoError)
  {
    emit done(m_userData, true, pReply->readAll());
  }
  else
  {
    QLOG_ERROR() << "HTTP download error:" << pReply->errorString();
    emit done(m_userData, false, QByteArray());
  }
  pReply->deleteLater();
  m_WebCtrl.clearAccessCache(); // make sure the TCP connection is closed
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static CodecDriver selectBestDecoder(const StreamInfo& stream)
{
  QList<CodecDriver> codecs = Codecs::findCodecsByFormat(Codecs::getCachedCodecList(), CodecType::Decoder, stream.codec);
  CodecDriver best = {};
  int bestScore = -1;
  for (auto codec : codecs)
  {
    int score = -1;

    if (codec.isSystemCodec())
    {
      // we always want to avoid using non-whitelisted system codecs
      // on the other hand, always prefer whitelisted system codecs
      if ((codec.isWhitelistedSystemAudioCodec() && useSystemAudioDecoders()) ||
          (codec.isWhitelistedSystemVideoCodec() && useSystemVideoDecoders()))
        score = 10;
      if (codec.format == "h264")
      {
        // Avoid using system video decoders for h264 profiles usually not supported.
        if (stream.profile != "" && stream.profile != "main" && stream.profile != "baseline" && stream.profile != "high")
          score = 1;
      }
      if (codec.driver == "h264_mf")
      {
        if (!stream.videoResolution.isEmpty())
        {
          QSize res = stream.videoResolution;
          if (res.width() > g_mediaFoundationH264MaxResolution.width() ||
              res.height() > g_mediaFoundationH264MaxResolution.height())
            score = 1;
        }
      }
      if (codec.driver == "aac_mf")
      {
        // Arbitrary but documented and enforced 6 channel limit by MS.
        if (stream.audioChannels > 6)
          score = 1;
      }
    }
    else
    {
      // prefer codecs which do not have to be downloaded over others
      if (codec.present)
        score = 15;
      else
        score = 5;
    }

    if (score > bestScore)
    {
      best = codec;
      bestScore = score;
    }
  }
  return best;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QList<CodecDriver> Codecs::determineRequiredCodecs(const PlaybackInfo& info)
{
  QList<CodecDriver> result;

  bool needAC3Encoder = false;

  QLOG_INFO() << "Using system audio decoders:" << useSystemAudioDecoders();
  QLOG_INFO() << "Using system video decoders:" << useSystemVideoDecoders();

#if !defined(HAVE_CODEC_MANIFEST)
  QLOG_INFO() << "Not using on-demand codecs.";
#endif

  for (auto stream : info.streams)
  {
    if (!stream.isVideo && !stream.isAudio)
      continue;
    if (!stream.codec.size())
    {
      QLOG_ERROR() << "unidentified codec";
      continue;
    }

    // We could do this if we'd find a nice way to enable passthrough by default:
#if 0
    // Can passthrough be used? If so, don't request a codec.
    if (info.audioPassthroughCodecs.contains(stream.codec))
      continue;
#endif

    // (Would be nice to check audioChannels here to not request the encoder
    // when playing stereo - but unfortunately, the ac3 encoder is loaded first,
    // and only removed when detecting stereo input)
    if (info.enableAC3Transcoding)
      needAC3Encoder = true;

    CodecDriver best = selectBestDecoder(stream);
    if (best.valid())
    {
      result.append(best);
    }
    else
    {
      QLOG_ERROR() << "no decoder for" << stream.codec;
    }
  }

  if (needAC3Encoder)
  {
    QList<CodecDriver> codecs = Codecs::findCodecsByFormat(Codecs::getCachedCodecList(), CodecType::Encoder, "ac3");
    CodecDriver encoder = {};
    for (auto codec : codecs)
    {
      if (codec.present && (!codec.isSystemCodec() || codec.isWhitelistedSystemAudioCodec()))
      {
        encoder = codec;
        break;
      }
      if (codec.external)
        encoder = codec; // fallback
    }
    if (encoder.valid())
    {
      result.append(encoder);
    }
    else
    {
      QLOG_ERROR() << "no AC3 encoder available";
    }
  }

  return result;
}
