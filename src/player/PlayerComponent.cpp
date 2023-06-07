#include "PlayerComponent.h"
#include <QString>
#include <Qt>
#include <QDir>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QDebug>
#include "display/DisplayComponent.h"
#include "settings/SettingsComponent.h"
#include "system/SystemComponent.h"
#include "utils/Utils.h"
#include "utils/Log.h"
#include "ComponentManager.h"
#include "settings/SettingsSection.h"

#include "PlayerQuickItem.h"
#include "input/InputComponent.h"

#include <math.h>
#include <string.h>
#include <shared/Paths.h>

#if !defined(Q_OS_WIN)
#include <unistd.h>
#endif

#if !defined(Q_OS_WIN)
#include <unistd.h>
#endif

#ifdef TARGET_RPI
#include <bcm_host.h>
#include <interface/vmcs_host/vcgencmd.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
static void wakeup_cb(void *context)
{
  PlayerComponent *player = (PlayerComponent *)context;

  emit player->onMpvEvents();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerComponent::PlayerComponent(QObject* parent)
  : ComponentBase(parent), m_state(State::finished), m_paused(false), m_playbackActive(false),
  m_windowVisible(false), m_videoPlaybackActive(false), m_inPlayback(false), m_playbackCanceled(false),
  m_bufferingPercentage(100), m_lastBufferingPercentage(-1),
  m_lastPositionUpdate(0.0), m_playbackAudioDelay(0),
  m_window(nullptr), m_mediaFrameRate(0),
  m_restoreDisplayTimer(this), m_reloadAudioTimer(this),
  m_streamSwitchImminent(false), m_doAc3Transcoding(false),
  m_videoRectangle(-1, -1, -1, -1)
{
  qmlRegisterType<PlayerQuickItem>("Konvergo", 1, 0, "MpvVideo"); // deprecated name
  qmlRegisterType<PlayerQuickItem>("Konvergo", 1, 0, "KonvergoVideo");

  m_restoreDisplayTimer.setSingleShot(true);
  connect(&m_restoreDisplayTimer, &QTimer::timeout, this, &PlayerComponent::onRestoreDisplay);

  connect(&DisplayComponent::Get(), &DisplayComponent::refreshRateChanged, this, &PlayerComponent::onRefreshRateChange);

  m_reloadAudioTimer.setSingleShot(true);
  connect(&m_reloadAudioTimer, &QTimer::timeout, this, &PlayerComponent::updateAudioDevice);
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::componentPostInitialize()
{
  InputComponent::Get().registerHostCommand("player", this, "userCommand");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerComponent::~PlayerComponent()
{
  if (m_mpv)
    mpv_set_wakeup_callback(m_mpv, nullptr, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PlayerComponent::componentInitialize()
{
  m_mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
  if (!m_mpv)
    throw FatalException(tr("Failed to load mpv."));

  mpv_request_log_messages(m_mpv, "terminal-default");
  mpv::qt::set_property(m_mpv, "msg-level", "all=v");

  // Configuration properties defined in the mpv.conf will override our
  // hardcoded properties below.
  mpv::qt::set_property(m_mpv, "config", "yes");
  mpv::qt::set_property(m_mpv, "config-dir", Paths::dataDir());

  mpv_set_wakeup_callback(m_mpv, wakeup_cb, this);

  // Disable native OSD if mpv_command_string() is used.
  mpv::qt::set_property(m_mpv, "osd-level", "0");

  // This forces the player not to rebase playback time to 0 with mkv. We
  // require this, because mkv transcoding lets files start at times other
  // than 0, and web-client expects that we return these times unchanged.
  mpv::qt::set_property(m_mpv, "demuxer-mkv-probe-start-time", false);

  // Upstream mpv sets this to "auto", which disables probing for HLS (at least),
  // in order to speed up playback start. The situation is more complex in PMP
  // due to us wanting to use system codecs, so always enable this.
  mpv::qt::set_property(m_mpv, "demuxer-lavf-probe-info", true);

  // Just discard audio output if no audio device could be opened. This gives
  // us better flexibility how to react to such errors (instead of just
  // aborting playback immediately).
  mpv::qt::set_property(m_mpv, "audio-fallback-to-null", "yes");

  // Do not let the decoder downmix (better customization for us).
  mpv::qt::set_property(m_mpv, "ad-lavc-downmix", false);

  // Make it load the hwdec interop, so hwdec can be enabled at runtime.
  mpv::qt::set_property(m_mpv, "gpu-hwdec-interop", "auto");

  // User-visible application name used by some audio APIs (at least PulseAudio).
  mpv::qt::set_property(m_mpv, "audio-client-name", QCoreApplication::applicationName());

  // User-visible stream title used by some audio APIs (at least PulseAudio and wasapi).
  mpv::qt::set_property(m_mpv, "title", QCoreApplication::applicationName());

  // See: https://github.com/plexinc/plex-media-player/issues/736
  mpv::qt::set_property(m_mpv, "cache-seek-min", 5000);

  // Disable ytdl
  mpv::qt::set_property(m_mpv, "ytdl", false);

  if (SettingsComponent::Get().ignoreSSLErrors()) {
    mpv::qt::set_property(m_mpv, "tls-ca-file", "");
    mpv::qt::set_property(m_mpv, "tls-verify", "no");
  } else {
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
    QList<QByteArray> list;
    list << "/etc/ssl/certs/ca-certificates.crt"
        << "/etc/pki/tls/certs/ca-bundle.crt"
        << "/usr/share/ssl/certs/ca-bundle.crt"
        << "/usr/local/share/certs/ca-root-nss.crt"
        << "/etc/ssl/cert.pem"
        << "/usr/share/curl/curl-ca-bundle.crt"
        << "/usr/local/share/curl/curl-ca-bundle.crt"
        << "/var/lib/ca-certificates/ca-bundle.pem";

    bool success = false;

    for (auto path : list)
    {
      if (access(path.data(), R_OK) == 0) {
        
        success = true;
        break;
      }
    }

    if (!success)
      throw FatalException(tr("Failed to locate CA bundle."));
#else
    // We need to not use Shinchiro's personal CA file...
    mpv::qt::set_property(m_mpv, "tls-ca-file", "");
#endif
  }

  // Apply some low-memory settings on RPI, which is relatively memory-constrained.
#ifdef TARGET_RPI
  // The backbuffer makes seeking back faster (without having to do a HTTP-level seek)
  mpv::qt::set_property(m_mpv, "cache-backbuffer", 10 * 1024); // KB
  // The demuxer queue is used for the readahead, and also for dealing with badly
  // interlaved audio/video. Setting it too low increases sensitivity to network
  // issues, and could cause playback failure with "bad" files.
  mpv::qt::set_property(m_mpv, "demuxer-max-bytes", 50 * 1024 * 1024); // bytes
  // Specifically for enabling mpeg4.
  mpv::qt::set_property(m_mpv, "hwdec-codecs", "all");
  // Do not use exact seeks by default. (This affects the start position in the "loadfile"
  // command in particular. We override the seek mode for normal "seek" commands.)
  mpv::qt::set_property(m_mpv, "hr-seek", "no");
  // Force vo_rpi to fullscreen.
  mpv::qt::set_property(m_mpv, "fullscreen", true);
#endif

  if (mpv_initialize(m_mpv) < 0)
    throw FatalException(tr("Failed to initialize mpv."));

  mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
  mpv_observe_property(m_mpv, 0, "core-idle", MPV_FORMAT_FLAG);
  mpv_observe_property(m_mpv, 0, "cache-buffering-state", MPV_FORMAT_INT64);
  mpv_observe_property(m_mpv, 0, "playback-time", MPV_FORMAT_DOUBLE);
  mpv_observe_property(m_mpv, 0, "vo-configured", MPV_FORMAT_FLAG);
  mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
  mpv_observe_property(m_mpv, 0, "audio-device-list", MPV_FORMAT_NODE);
  mpv_observe_property(m_mpv, 0, "video-dec-params", MPV_FORMAT_NODE);

  // Setup a hook with the ID 1, which is run during the file is loaded.
  // Used to delay playback start for display framerate switching.
  // (See handler in handleMpvEvent() for details.)
  // Setup a hook with the ID 2, which is run at a certain stage during loading.
  // We use it to initialize stream selections and to probe the codecs.
#if MPV_CLIENT_API_VERSION < MPV_MAKE_VERSION(1, 100)
  mpv::qt::command(m_mpv, QStringList() << "hook-add" << "on_load" << "1" << "0");
  mpv::qt::command(m_mpv, QStringList() << "hook-add" << "on_preloaded" << "2" << "0");
#else
  mpv_hook_add(m_mpv, 1, "on_load", 0);
  mpv_hook_add(m_mpv, 2, "on_preloaded", 0);
#endif

  updateAudioDeviceList();
  setAudioConfiguration();
  updateSubtitleSettings();
  updateVideoSettings();

  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_VIDEO), &SettingsSection::valuesUpdated,
          this, &PlayerComponent::updateVideoSettings);

  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_SUBTITLES), &SettingsSection::valuesUpdated,
          this, &PlayerComponent::updateSubtitleSettings);

  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO), &SettingsSection::valuesUpdated,
          this, &PlayerComponent::setAudioConfiguration);

  initializeCodecSupport();
  Codecs::initCodecs();

  QString codecInfo;
  for (auto codec : Codecs::getCachedCodecList())
  {
    if (codec.present)
    {
      if (codecInfo.size())
        codecInfo += " ";
      codecInfo += codec.driver;
      if (codec.type == CodecType::Encoder)
        codecInfo += "(enc)";
    }
  }
  qInfo() << "Present codecs:" << qPrintable(codecInfo);

  connect(this, &PlayerComponent::onMpvEvents, this, &PlayerComponent::handleMpvEvents, Qt::QueuedConnection);
  emit onMpvEvents();

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setVideoRectangle(int x, int y, int w, int h)
{
  QRect rc(x, y, w, h);
  if (rc != m_videoRectangle)
  {
    m_videoRectangle = rc;
    emit onVideoRecangleChanged();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setQtQuickWindow(QQuickWindow* window)
{
  PlayerQuickItem* video = window->findChild<PlayerQuickItem*>("video");
  if (!video)
    throw FatalException(tr("Failed to load video element."));

  video->initMpv(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setWindow(QQuickWindow* window)
{
  QString vo = "libmpv";

#ifdef TARGET_RPI
  window->setFlags(Qt::FramelessWindowHint);
  vo = "rpi";
#endif

  m_window = window;
  if (!window)
    return;

  QString forceVo = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "debug.force_vo").toString();
  if (forceVo.size())
    vo = forceVo;

  mpv::qt::set_property(m_mpv, "vo", vo);

  if (vo == "libmpv")
    setQtQuickWindow(window);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PlayerComponent::load(const QString& url, const QVariantMap& options, const QVariantMap &metadata, const QString& audioStream , const QString& subtitleStream)
{
  stop();
  queueMedia(url, options, metadata, audioStream, subtitleStream);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::queueMedia(const QString& url, const QVariantMap& options, const QVariantMap &metadata, const QString& audioStream, const QString& subtitleStream)
{
  InputComponent::Get().cancelAutoRepeat();

  m_mediaFrameRate = metadata["frameRate"].toFloat(); // returns 0 on failure
  m_serverMediaInfo = metadata["media"].toMap();

  updateVideoSettings();

  QUrl qurl = url;
  QString host = qurl.host();

  QVariantList command;
  command << "loadfile" << qurl.toString(QUrl::FullyEncoded);
  command << "append-play"; // if nothing is playing, play it now, otherwise just enqueue it

  QVariantMap extraArgs;

  quint64 startMilliseconds = options["startMilliseconds"].toLongLong();
  if (startMilliseconds != 0)
    extraArgs.insert("start", "+" + QString::number(startMilliseconds / 1000.0));

  // we're going to select these streams later, in the preloaded hook
  extraArgs.insert("aid", "no");
  extraArgs.insert("sid", "no");

  m_currentSubtitleStream = subtitleStream;
  m_currentAudioStream = audioStream;

  if (metadata["type"] == "music")
    extraArgs.insert("vid", "no");

  extraArgs.insert("pause", options["autoplay"].toBool() ? "no" : "yes");

  QString userAgent = metadata["headers"].toMap()["User-Agent"].toString();
  if (userAgent.size())
    extraArgs.insert("user-agent", userAgent);

  // Make sure the list of requested codecs is reset.
  extraArgs.insert("ad", "");
  extraArgs.insert("vd", "");

  command << extraArgs;

  mpv::qt::command(m_mpv, command);

  emit onMetaData(metadata["metadata"].toMap(), qurl.adjusted(QUrl::RemovePath | QUrl::RemoveQuery));
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::streamSwitch()
{
  m_streamSwitchImminent = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PlayerComponent::switchDisplayFrameRate()
{
  qDebug() << "Video framerate:" << m_mediaFrameRate << "fps";

  if (!SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "refreshrate.auto_switch").toBool())
  {
    qDebug() << "Not switching refresh-rate (disabled by settings).";
    return false;
  }

  bool fs = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "fullscreen").toBool();
#if KONVERGO_OPENELEC
  fs = true;
#endif
  if (!fs)
  {
    qDebug() << "Not switching refresh-rate (not in fullscreen mode).";
    return false;
  }

  if (m_mediaFrameRate < 1)
  {
    qDebug() << "Not switching refresh-rate (no known video framerate).";
    return false;
  }

  // Make sure a timer started by the previous file ending isn't accidentally
  // still in-flight. It could switch the display back after we've switched.
  m_restoreDisplayTimer.stop();

  DisplayComponent* display = &DisplayComponent::Get();
  if (!display->switchToBestVideoMode(m_mediaFrameRate))
  {
    qDebug() << "Switching refresh-rate failed or unnecessary.";
    return false;
  }

  // Make sure settings dependent on the display refresh rate are updated properly.
  updateVideoSettings();
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::onRestoreDisplay()
{
  // If the player will in fact start another file (or is playing one), don't restore.
  if (mpv::qt::get_property(m_mpv, "idle-active").toBool())
    DisplayComponent::Get().restorePreviousVideoMode();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::onRefreshRateChange()
{
  // Make sure settings dependent on the display refresh rate are updated properly.
  updateVideoSettings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::updatePlaybackState()
{
  State newState = m_state;

  if (m_inPlayback) {
    if (m_paused)
    {
      newState = State::paused;
    }
    else if (m_playbackActive)
    {
      newState = State::playing;
    }
    else
    {
      // Playback not active, but also not buffering means we're in some "other"
      // waiting state. Pretend to web-client that we're buffering.
      if (m_bufferingPercentage == 100)
        m_bufferingPercentage = 0;
      newState = State::buffering;
    }
  }
  else
  {
    if (!m_playbackError.isEmpty())
      newState = State::error;
    else if (m_playbackCanceled)
      newState = State::canceled;
    else
      newState = State::finished;
  }

  if (newState != m_state)
  {
    switch (newState) {
    case State::paused:
      qInfo() << "Entering state: paused";
      emit paused();
      break;
    case State::playing:
      qInfo() << "Entering state: playing";
      emit playing();
      break;
    case State::buffering:
      qInfo() << "Entering state: buffering";
      m_lastBufferingPercentage = -1; /* force update below */
      break;
    case State::finished:
      qInfo() << "Entering state: finished";
      emit finished();
      emit stopped();
      break;
    case State::canceled:
      qInfo() << "Entering state: canceled";
      emit canceled();
      emit stopped();
      break;
    case State::error:
      qInfo() << ("Entering state: error (" + m_playbackError + ")");
      emit error(m_playbackError);
      break;
    }
    emit stateChanged(newState, m_state);
    m_state = newState;
  }

  if (m_state == State::buffering && m_lastBufferingPercentage != m_bufferingPercentage)
    emit buffering(m_bufferingPercentage);
  m_lastBufferingPercentage = m_bufferingPercentage;

  bool is_videoPlaybackActive = m_state == State::playing && m_windowVisible;
  if (m_videoPlaybackActive != is_videoPlaybackActive)
  {
    m_videoPlaybackActive = is_videoPlaybackActive;
    emit videoPlaybackActive(m_videoPlaybackActive);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::handleMpvEvent(mpv_event *event)
{
  switch (event->event_id)
  {
    case MPV_EVENT_START_FILE:
    {
      m_inPlayback = true;
      break;
    }
    case MPV_EVENT_END_FILE:
    {
      mpv_event_end_file *endFile = (mpv_event_end_file *)event->data;

      m_inPlayback = false;
      m_playbackCanceled = false;
      m_playbackError = "";

      switch (endFile->reason)
      {
        case MPV_END_FILE_REASON_ERROR:
        {
          m_playbackError = mpv_error_string(endFile->error);
          break;
        }
        case MPV_END_FILE_REASON_STOP:
        {
          m_playbackCanceled = true;
          break;
        }
        case MPV_END_FILE_REASON_EOF:
        case MPV_END_FILE_REASON_QUIT:
        case MPV_END_FILE_REASON_REDIRECT:
          break;
      }

      if (!m_streamSwitchImminent)
        m_restoreDisplayTimer.start(0);
      m_streamSwitchImminent = false;
      break;
    }
    case MPV_EVENT_PROPERTY_CHANGE:
    {
      mpv_event_property *prop = (mpv_event_property *)event->data;
      if (strcmp(prop->name, "pause") == 0 && prop->format == MPV_FORMAT_FLAG)
      {
        m_paused = !!*(int *)prop->data;
      }
      else if (strcmp(prop->name, "core-idle") == 0 && prop->format == MPV_FORMAT_FLAG)
      {
        m_playbackActive = !*(int *)prop->data;
      }
      else if (strcmp(prop->name, "cache-buffering-state") == 0)
      {
        m_bufferingPercentage = prop->format == MPV_FORMAT_INT64 ? (int)*(int64_t *)prop->data : 100;
      }
      else if (strcmp(prop->name, "playback-time") == 0 && prop->format == MPV_FORMAT_DOUBLE)
      {
        double pos = *(double*)prop->data;
        if (fabs(pos - m_lastPositionUpdate) > 0.015)
        {
          quint64 ms = (quint64)(qMax(pos * 1000.0, 0.0));
          emit positionUpdate(ms);
          m_lastPositionUpdate = pos;
        }
      }
      else if (strcmp(prop->name, "vo-configured") == 0)
      {
        int state = prop->format == MPV_FORMAT_FLAG ? *(int *)prop->data : 0;
        m_windowVisible = state;
        emit windowVisible(m_windowVisible);
      }
      else if (strcmp(prop->name, "duration") == 0)
      {
        if (prop->format == MPV_FORMAT_DOUBLE)
          emit updateDuration(*(double *)prop->data * 1000.0);
      }
      else if (strcmp(prop->name, "audio-device-list") == 0)
      {
        updateAudioDeviceList();
      }
      else if (strcmp(prop->name, "video-dec-params") == 0)
      {
        // Aspect might be known now (or it changed during playback), so update settings
        // dependent on the aspect ratio.
        updateVideoAspectSettings();
      }
      break;
    }
    case MPV_EVENT_LOG_MESSAGE:
    {
      mpv_event_log_message *msg = (mpv_event_log_message *)event->data;
      // Strip the trailing '\n'
      size_t len = strlen(msg->text);
      if (len > 0 && msg->text[len - 1] == '\n')
        len -= 1;
      QString logline = QString::fromUtf8(msg->prefix) + ": " + QString::fromUtf8(msg->text, (int)len);
      if (msg->log_level >= MPV_LOG_LEVEL_V)
        qDebug() << qPrintable(logline);
      else if (msg->log_level >= MPV_LOG_LEVEL_INFO)
        qInfo() << qPrintable(logline);
      else if (msg->log_level >= MPV_LOG_LEVEL_WARN)
        qWarning() << qPrintable(logline);
      else
        qCritical() << qPrintable(logline);
      break;
    }
    case MPV_EVENT_CLIENT_MESSAGE:
    {
      mpv_event_client_message *msg = (mpv_event_client_message *)event->data;
      if (msg->num_args < 3 || strcmp(msg->args[0], "hook_run") != 0)
        break;
      QString resumeId = QString::fromUtf8(msg->args[2]);
      // Start "on_load" hook.
      // This happens when the player is about to load the file, but no actual loading has taken part yet.
      // We use this to block loading until we explicitly tell it to continue.
      if (!strcmp(msg->args[1], "1"))
      {
        // Calling this lambda will instruct mpv to continue loading the file.
        auto resume = [=] {
          qInfo() << "checking codecs";
          startCodecsLoading([=] {
            qInfo() << "resuming loading";
            mpv::qt::command(m_mpv, QStringList() << "hook-ack" << resumeId);
          });
        };
        if (switchDisplayFrameRate())
        {
          // Now wait for some time for mode change - this is needed because mode changing can take some
          // time, during which the screen is black, and initializing hardware decoding could fail due
          // to various strange OS-related reasons.
          // (Better hope the user doesn't try to exit Konvergo during mode change.)
          int pause = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "refreshrate.delay").toInt() * 1000;
          qInfo() << "waiting" << pause << "msec after rate switch before loading";
          QTimer::singleShot(pause, resume);
        }
        else
        {
          resume();
        }
        break;
      }
      // Start "on_preloaded" hook.
      // Used initialize stream selections and to probe codecs.
      if (!strcmp(msg->args[1], "2"))
      {
        reselectStream(m_currentSubtitleStream, MediaType::Subtitle);
        reselectStream(m_currentAudioStream, MediaType::Audio);
        startCodecsLoading([=] {
          mpv::qt::command(m_mpv, QStringList() << "hook-ack" << resumeId);
        });
        break;
      }
      break;
    }
#if MPV_CLIENT_API_VERSION >= MPV_MAKE_VERSION(1, 100)
    case MPV_EVENT_HOOK:
    {
      mpv_event_hook *hook = (mpv_event_hook *)event->data;
      uint64_t id = hook->id;

      if (!strcmp(hook->name, "on_load"))
      {
        // Calling this lambda will instruct mpv to continue loading the file.
        auto resume = [=] {
          qInfo() << "checking codecs";
          startCodecsLoading([=] {
            qInfo() << "resuming loading";
            mpv_hook_continue(m_mpv, id);
          });
        };
        if (switchDisplayFrameRate())
        {
          // Now wait for some time for mode change - this is needed because mode changing can take some
          // time, during which the screen is black, and initializing hardware decoding could fail due
          // to various strange OS-related reasons.
          // (Better hope the user doesn't try to exit Konvergo during mode change.)
          int pause = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "refreshrate.delay").toInt() * 1000;
          qInfo() << "waiting" << pause << "msec after rate switch before loading";
          QTimer::singleShot(pause, resume);
        }
        else
        {
          resume();
        }
        break;
      }
      if (!strcmp(hook->name, "on_preloaded"))
      {
        reselectStream(m_currentSubtitleStream, MediaType::Subtitle);
        reselectStream(m_currentAudioStream, MediaType::Audio);
        startCodecsLoading([=] {
          mpv_hook_continue(m_mpv, id);
        });
        break;
      }
      break;
    }
#endif

    default:; /* ignore */
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::handleMpvEvents()
{
  // Process all events, until the event queue is empty.
  while (1)
  {
    mpv_event *event = mpv_wait_event(m_mpv, 0);
    if (event->event_id == MPV_EVENT_NONE)
      break;
    handleMpvEvent(event);
  }
  // Once we got all status updates, determine the new canonical state.
  updatePlaybackState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setVideoOnlyMode(bool enable)
{
  if (m_window)
  {
    QQuickItem *web = m_window->findChild<QQuickItem *>("web");
    if (web)
      web->setVisible(!enable);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::play()
{
  QStringList args = (QStringList() << "set" << "pause" << "no");
  mpv::qt::command(m_mpv, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::stop()
{
  QStringList args("stop");
  mpv::qt::command(m_mpv, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::clearQueue()
{
  QStringList args("playlist_clear");
  mpv::qt::command(m_mpv, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::pause()
{
  QStringList args = (QStringList() << "set" << "pause" << "yes");
  mpv::qt::command(m_mpv, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::seekTo(qint64 ms)
{
  double timeSecs = ms / 1000.0;
  QVariantList args = (QVariantList() << "seek" << timeSecs << "absolute+exact");
  mpv::qt::command(m_mpv, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant PlayerComponent::getAudioDeviceList()
{
  return mpv::qt::get_property(m_mpv, "audio-device-list");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setAudioDevice(const QString& name)
{
  mpv::qt::set_property(m_mpv, "audio-device", name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setVolume(int volume)
{
  // Will fail if no audio output opened (i.e. no file playing)
  mpv::qt::set_property(m_mpv, "volume", volume);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int PlayerComponent::volume()
{
  QVariant volume = mpv::qt::get_property(m_mpv, "volume");
  if (volume.isValid())
    return volume.toInt();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setMuted(bool muted)
{
  // Will fail if no audio output opened (i.e. no file playing)
  mpv::qt::set_property(m_mpv, "mute", muted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PlayerComponent::muted()
{
  QVariant mute = mpv::qt::get_property(m_mpv, "mute");
  if (mute.isValid())
    return mute.toBool();
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariantList PlayerComponent::findStreamsForURL(const QString &url)
{
  bool isExternal = !url.isEmpty();
  QVariantList res;

  auto tracks = mpv::qt::get_property(m_mpv, "track-list");
  for (auto track : tracks.toList())
  {
    QVariantMap map = track.toMap();

    if (map["external"].toBool() != isExternal)
      continue;

    if (!isExternal || map["external-filename"].toString() == url)
      res += map;
  }

  return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::reselectStream(const QString &streamSelection, MediaType target)
{
  QString streamIdPropertyName;
  QString streamAddCommandName;
  QString mpvStreamTypeName;

  switch (target)
  {
  case MediaType::Subtitle:
    mpvStreamTypeName = "sub";
    streamIdPropertyName = "sid";
    streamAddCommandName = "sub-add";
    break;
  case MediaType::Audio:
    mpvStreamTypeName = "audio";
    streamIdPropertyName = "aid";
    streamAddCommandName = "audio-add";
    break;
  }

  QString streamName;
  QString streamID;

  if (streamSelection.startsWith("#"))
  {
    int splitPos = streamSelection.indexOf(",");
    if (splitPos < 0)
    {
      // Stream from the main file
      streamID = streamSelection.mid(1);
      streamName = "";
    }
    else
    {
      // Stream from an external file
      streamID = streamSelection.mid(1, splitPos - 1);
      streamName = streamSelection.mid(splitPos + 1);
    }
  }
  else if (streamSelection.isEmpty())
  {
    mpv::qt::set_property(m_mpv, streamIdPropertyName, "no");
    return;
  }

  if (!streamName.isEmpty())
  {
    auto streams = findStreamsForURL(streamName);
    if (streams.isEmpty())
    {
      QStringList args = (QStringList() << streamAddCommandName << streamName);
      mpv::qt::command(m_mpv, args);
    }
  }

  QString selection = "no";

  if (!streamID.isEmpty())
  {
    selection = streamID;
  } else {
    for (auto stream : findStreamsForURL(streamName))
    {
      auto map = stream.toMap();

      if (map["type"].toString() != mpvStreamTypeName)
      {
        continue;
      } else if (map["external-filename"].toString() == streamName) {
        selection = map["id"].toString();
        break;
      }
    }
  }

  // Fallback to the first stream if none could be found.
  // Useful if web-client uses wrong stream IDs when e.g. transcoding.
  if ((target == MediaType::Audio || !streamID.isEmpty()) && selection == "no")
    selection = "1";

  mpv::qt::set_property(m_mpv, streamIdPropertyName, selection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setSubtitleStream(const QString &subtitleStream)
{
  m_currentSubtitleStream = subtitleStream;
  reselectStream(m_currentSubtitleStream, MediaType::Subtitle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setAudioStream(const QString &audioStream)
{
  m_currentAudioStream = audioStream;
  reselectStream(m_currentAudioStream, MediaType::Audio);
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setAudioDelay(qint64 milliseconds)
{
  m_playbackAudioDelay = milliseconds;

  double displayFps = DisplayComponent::Get().currentRefreshRate();
  const char *audioDelaySetting = "audio_delay.normal";
  if (fabs(displayFps - 24) < 0.5) // cover 24Hz, 23.976Hz, and values very close
    audioDelaySetting = "audio_delay.24hz";
  else if (fabs(displayFps - 25) < 0.5)
    audioDelaySetting = "audio_delay.25hz";
  else if (fabs(displayFps - 50) < 0.5)
    audioDelaySetting = "audio_delay.50hz";

  double fixedDelay = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, audioDelaySetting).toFloat();
  mpv::qt::set_property(m_mpv, "audio-delay", (fixedDelay + m_playbackAudioDelay) / 1000.0);
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setSubtitleDelay(qint64 milliseconds)
{
  mpv::qt::set_property(m_mpv, "sub-delay", milliseconds / 1000.0);
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setPlaybackRate(int rate)
{
  mpv::qt::set_property(m_mpv, "speed", rate / 1000.0);
}

/////////////////////////////////////////////////////////////////////////////////////////
qint64 PlayerComponent::getPosition()
{
  QVariant time = mpv::qt::get_property(m_mpv, "playback-time");
  if (time.canConvert<double>())
    return time.toDouble();
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
qint64 PlayerComponent::getDuration()
{
  QVariant time = mpv::qt::get_property(m_mpv, "duration");
  if (time.canConvert<double>())
    return time.toDouble();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// This is called with the set of previous audio devices that were detected, and the set of current
// audio devices. From this we guess whether we should reopen the audio device. If the user-selected
// device went away previously, and now comes back, reinitializing the player's audio output will
// force the player and/or the OS to move audio output back to the user-selected device.
void PlayerComponent::checkCurrentAudioDevice(const QSet<QString>& old_devs, const QSet<QString>& new_devs)
{
  QString userDevice = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "device").toString();

  QSet<QString> removed = old_devs - new_devs;
  QSet<QString> added = new_devs - old_devs;

  qDebug() << "Audio devices removed:" << removed;
  qDebug() << "Audio devices added:" << added;
  qDebug() << "Audio device selected:" << userDevice;

  if (userDevice.length())
  {
    if (added.contains(userDevice) || removed.contains(userDevice))
    {
      // The timer is for debouncing the reload. Several change notifications could
      // come in quick succession. Also, it's possible that trying to open the
      // reappeared audio device immediately can fail.
      m_reloadAudioTimer.start(500);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::updateAudioDeviceList()
{
  QString userDevice = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "device").toString();
  bool userDeviceFound = false;

  QVariantList settingList;
  QVariant list = getAudioDeviceList();
  QSet<QString> devices;
  for(const QVariant& d : list.toList())
  {
    Q_ASSERT(d.typeId() == QMetaType::QVariantMap);
    QVariantMap dmap = d.toMap();

    QString device = dmap["name"].toString();
    QString description = dmap["description"].toString();

    devices.insert(device);

    if (userDevice == device)
      userDeviceFound = true;

    QVariantMap entry;
    entry["value"] = device;
    entry["title"] = description;

    settingList << entry;
  }

  if (!userDeviceFound)
  {
    QVariantMap entry;
    entry["value"] = userDevice;
    entry["title"] = "[Disconnected device: " + userDevice + "]";

    settingList << entry;
  }

  SettingsComponent::Get().updatePossibleValues(SETTINGS_SECTION_AUDIO, "device", settingList);

  checkCurrentAudioDevice(m_audioDevices, devices);
  m_audioDevices = devices;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::updateAudioDevice()
{
  QString device = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "device").toString();

  if (!m_audioDevices.contains(device))
  {
    qWarning() << "Not using audio device" << device << "because it's not present.";
    device = "auto";
  }

  mpv::qt::set_property(m_mpv, "audio-device", device);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setAudioConfiguration()
{
  QString deviceType = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "devicetype").toString();

  mpv::qt::set_property(m_mpv, "audio-exclusive", SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "exclusive").toBool());

  updateAudioDevice();

  bool normalize = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "normalize").toBool();
  mpv::qt::set_property(m_mpv, "audio-normalize-downmix", normalize ? "yes" : "no");

  // Make downmix more similar to PHT.
  mpv::qt::set_property(m_mpv, "audio-swresample-o", "surround_mix_level=1");

  m_passthroughCodecs.clear();

  // passthrough doesn't make sense with basic type
  if (deviceType != AUDIO_DEVICE_TYPE_BASIC)
  {
    SettingsSection* audioSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO);

    QStringList codecs;
    if (deviceType == AUDIO_DEVICE_TYPE_SPDIF)
      codecs = AudioCodecsSPDIF();
    else if (deviceType == AUDIO_DEVICE_TYPE_HDMI)
      codecs = AudioCodecsAll();

    for(const QString& key : codecs)
    {
      if (audioSection->value("passthrough." + key).toBool())
        m_passthroughCodecs << key;
    }

    // dts-hd includes dts, but listing dts before dts-hd may disable dts-hd.
    if (m_passthroughCodecs.indexOf("dts-hd") != -1)
      m_passthroughCodecs.removeAll("dts");
  }

  QString passthroughCodecs = m_passthroughCodecs.join(",");
  mpv::qt::set_property(m_mpv, "audio-spdif", passthroughCodecs);

  // set the channel layout
  QVariant layout = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "channels");

  // always force either stereo or transcoding
  if (deviceType == AUDIO_DEVICE_TYPE_SPDIF)
    layout = "2.0";

  mpv::qt::set_property(m_mpv, "audio-channels", layout);

  // if the user has indicated that PCM only works for stereo, and that
  // the receiver supports AC3, set this extra option that allows us to transcode
  // 5.1 audio into a usable format, note that we only support AC3
  // here for now. We might need to add support for DTS transcoding
  // if we see user requests for it.
  //
  m_doAc3Transcoding =
  (deviceType == AUDIO_DEVICE_TYPE_SPDIF &&
   SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "passthrough.ac3").toBool());
  if (m_doAc3Transcoding)
  {
    QString filterArgs = "";
    mpv::qt::command(m_mpv, QStringList() << "af" << "add" << ("lavcac3enc" + filterArgs));
  }
  else
  {
    mpv::qt::command(m_mpv, QStringList() << "af" << "remove" << "lavcac3enc");
  }

  QVariant device = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "device");

  // Make a informational log message.
  QString audioConfig = QString(QString("Audio Config - device: %1, ") +
                                        "channel layout: %2, " +
                                        "passthrough codecs: %3, " +
                                        "ac3 transcoding: %4").arg(device.toString(),
                                                                   layout.toString(),
                                                                   passthroughCodecs.isEmpty() ? "none" : passthroughCodecs,
                                                                   m_doAc3Transcoding ? "yes" : "no");
  qInfo() << qPrintable(audioConfig);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::updateSubtitleSettings()
{
  QVariant size = SettingsComponent::Get().value(SETTINGS_SECTION_SUBTITLES, "size");
  mpv::qt::set_property(m_mpv, "sub-scale", size.toInt() / 32.0);

  QVariant colorsString = SettingsComponent::Get().value(SETTINGS_SECTION_SUBTITLES, "color");
  auto colors = colorsString.toString().split(",");
  if (colors.length() == 2)
  {
    mpv::qt::set_property(m_mpv, "sub-color", colors[0]);
    mpv::qt::set_property(m_mpv, "sub-border-color", colors[1]);
  }

  QVariant subposString = SettingsComponent::Get().value(SETTINGS_SECTION_SUBTITLES, "placement");
  auto subpos = subposString.toString().split(",");
  if (subpos.length() == 2)
  {
    mpv::qt::set_property(m_mpv, "sub-align-x", subpos[0]);
    mpv::qt::set_property(m_mpv, "sub-pos", subpos[1] == "bottom" ? 100 : 10);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::updateVideoAspectSettings()
{
  QVariant mode = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "aspect").toString();
  bool disableScaling = false;
  bool keepAspect = true;
  QString forceAspect = "-1";
  double panScan = 0.0;
  if (mode == "custom")
  {
    // in particular, do not restore anything - the intention is not to touch the user's mpv.conf settings, or whatever
    return;
  }
  else if (mode == "zoom")
  {
    panScan = 1.0;
  }
  else if (mode == "force_4_3")
  {
    forceAspect = "4:3";
  }
  else if (mode == "force_16_9")
  {
    forceAspect = "16:9";
  }
  else if (mode == "force_16_9_if_4_3")
  {
    auto params = mpv::qt::get_property(m_mpv, "video-dec-params").toMap();
    auto aspect = params["aspect"].toFloat();
    if (fabs(aspect - 4.0/3.0) < 0.1)
      forceAspect = "16:9";
  }
  else if (mode == "stretch")
  {
    keepAspect = false;
  }
  else if (mode == "noscaling")
  {
    disableScaling = true;
  }

  mpv::qt::set_property(m_mpv, "video-unscaled", disableScaling);
  mpv::qt::set_property(m_mpv, "video-aspect-override", forceAspect);
  mpv::qt::set_property(m_mpv, "keepaspect", keepAspect);
  mpv::qt::set_property(m_mpv, "panscan", panScan);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::updateVideoSettings()
{
  if (!m_mpv)
    return;

  QVariant syncMode = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "sync_mode");
  mpv::qt::set_property(m_mpv, "video-sync", syncMode);

  QString hardwareDecodingMode = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "hardwareDecoding").toString();
  QString hwdecMode = "no";
  QString hwdecVTFormat = "nv12";
  if (hardwareDecodingMode == "enabled")
    hwdecMode = "auto";
  else if (hardwareDecodingMode == "osx_compat")
  {
    hwdecMode = "auto";
    hwdecVTFormat = "uyvy422";
  }
  else if (hardwareDecodingMode == "copy")
  {
    hwdecMode = "auto-copy";
  }
  mpv::qt::set_property(m_mpv, "hwdec", hwdecMode);
  mpv::qt::set_property(m_mpv, "hwdec-image-format", hwdecVTFormat);

  QVariant deinterlace = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "deinterlace");
  mpv::qt::set_property(m_mpv, "deinterlace", deinterlace.toBool() ? "yes" : "no");

#ifndef TARGET_RPI
  double displayFps = DisplayComponent::Get().currentRefreshRate();
  mpv::qt::set_property(m_mpv, "override-display-fps", displayFps);
#endif

  setAudioDelay(m_playbackAudioDelay);

  QVariant cache = SettingsComponent::Get().value(SETTINGS_SECTION_VIDEO, "cache");
  mpv::qt::set_property(m_mpv, "demuxer-max-bytes", cache.toInt() * 1024);

  updateVideoAspectSettings();
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::userCommand(QString command)
{
  QByteArray cmdUtf8 = command.toUtf8();
  mpv_command_string(m_mpv, cmdUtf8.data());
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::initializeCodecSupport()
{
  QMap<QString, QString> all = { {"vc1", "WVC1"}, {"mpeg2video", "MPG2"} };
  for (auto name : all.keys())
  {
    bool ok = true;
#ifdef TARGET_RPI
    char res[100] = "";
    bcm_host_init();
    if (vc_gencmd(res, sizeof(res), "codec_enabled %s", all[name].toUtf8().data()))
      res[0] = '\0'; // error
    ok = !!strstr(res, "=enabled");
#endif
    m_codecSupport[name] = ok;
    qInfo() << "Codec" << name << (ok ? "present" : "disabled");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
bool PlayerComponent::checkCodecSupport(const QString& codec)
{
  if (m_codecSupport.contains(codec))
    return m_codecSupport[codec];
  return true; // doesn't matter if unknown codecs are reported as "ok"
}

/////////////////////////////////////////////////////////////////////////////////////////
QList<CodecDriver> convertCodecList(QVariant list, CodecType type)
{
  QList<CodecDriver> codecs;

  foreach (const QVariant& e, list.toList())
  {
    QVariantMap map = e.toMap();

    QString family = map["family"].toString();
    QString codec = map["codec"].toString();
    QString driver = map["driver"].toString();

    // Only include FFmpeg codecs; exclude pseudo-codecs like spdif (on mpv versions where those were exposed).
    if (family != "" && family != "lavc")
      continue;

    CodecDriver ncodec = {};
    ncodec.type = type;
    ncodec.format = codec;
    ncodec.driver = driver;
    ncodec.present = true;

    codecs.append(ncodec);
  }

  return codecs;
}

/////////////////////////////////////////////////////////////////////////////////////////
QList<CodecDriver> PlayerComponent::installedCodecDrivers()
{
  QList<CodecDriver> codecs;

  codecs.append(convertCodecList(mpv::qt::get_property(m_mpv, "decoder-list"), CodecType::Decoder));
  codecs.append(convertCodecList(mpv::qt::get_property(m_mpv, "encoder-list"), CodecType::Encoder));

  return codecs;
}

/////////////////////////////////////////////////////////////////////////////////////////
QStringList PlayerComponent::installedDecoderCodecs()
{
  QStringList formats;
  bool hasPcm = false;

  for (auto driver : installedCodecDrivers())
  {
    if (driver.type == CodecType::Decoder && checkCodecSupport(driver.format))
    {
      QString name = Codecs::plexNameFromFF(driver.format);
      if (name.startsWith("pcm_") && name != "pcm_bluray" && name != "pcm_dvd")
      {
        if (hasPcm)
          continue;
        hasPcm = true;
        name = "pcm";
      }
      formats.append(name);
    }
  }

  return formats;
}

/////////////////////////////////////////////////////////////////////////////////////////
PlaybackInfo PlayerComponent::getPlaybackInfo()
{
  PlaybackInfo info = {};

  for (auto codec : m_passthroughCodecs)
  {
    // Normalize back to canonical codec names.
    if (codec == "dts-hd")
      codec = "dts";
    info.audioPassthroughCodecs.insert(codec);
  }

  info.enableAC3Transcoding = m_doAc3Transcoding;

  auto tracks = mpv::qt::get_property(m_mpv, "track-list");
  for (auto track : tracks.toList())
  {
    QVariantMap map = track.toMap();
    QString type = map["type"].toString();

    StreamInfo stream = {};
    stream.isVideo = type == "video";
    stream.isAudio = type == "audio";
    stream.codec = map["codec"].toString();
    stream.audioChannels = map["demux-channel-count"].toInt();
    stream.audioSampleRate = map["demux-samplerate"].toInt();
    stream.videoResolution = QSize(map["demux-w"].toInt(), map["demux-h"].toInt());

    // Get the profile from the server, because mpv can't determine it yet.
    if (stream.isVideo)
    {
      int index = map["ff-index"].toInt();
      for (auto partInfo : m_serverMediaInfo["Part"].toList())
      {
        for (auto streamInfo : partInfo.toMap()["Stream"].toList())
        {
          auto streamInfoMap = streamInfo.toMap();
          bool ok = false;
          if (streamInfoMap["index"].toInt(&ok) == index && ok)
          {
            stream.profile = streamInfoMap["profile"].toString();
            qDebug() << "h264profile:" << stream.profile;
          }
        }
      }
    }

    info.streams.append(stream);
  }

  // If we're in an early stage where we don't have streams yet, try to get the
  // info from the PMS metadata.
  if (!info.streams.size())
  {
    for (auto partInfo : m_serverMediaInfo["Part"].toList())
    {
      for (auto streamInfo : partInfo.toMap()["Stream"].toList())
      {
        auto streamInfoMap = streamInfo.toMap();

        StreamInfo stream = {};
        stream.isVideo = streamInfoMap["width"].isValid();
        stream.isAudio = streamInfoMap["channels"].isValid();
        stream.codec = Codecs::plexNameToFF(streamInfoMap["codec"].toString());
        stream.audioChannels = streamInfoMap["channels"].toInt();
        stream.videoResolution = QSize(streamInfoMap["width"].toInt(), streamInfoMap["height"].toInt());
        stream.profile = streamInfoMap["profile"].toString();

        info.streams.append(stream);
      }
    }
  }

  return info;
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::setPreferredCodecs(const QList<CodecDriver>& codecs)
{
  QStringList items;
  for (auto codec : codecs)
  {
    if (codec.type == CodecType::Decoder)
    {
      items << codec.driver;
    }
  }
  QString opt = items.join(",");
  // For simplicity, we don't distinguish between audio and video. The player
  // will ignore entries with mismatching media type.
  mpv::qt::set_property(m_mpv, "ad", opt);
  mpv::qt::set_property(m_mpv, "vd", opt);
}

// For QVariant.
Q_DECLARE_METATYPE(std::function<void()>);

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::startCodecsLoading(std::function<void()> resume)
{
  auto fetcher = new CodecsFetcher();
  fetcher->userData = QVariant::fromValue(resume);
  connect(fetcher, &CodecsFetcher::done, this, &PlayerComponent::onCodecsLoadingDone);
  Codecs::updateCachedCodecList();
  QList<CodecDriver> codecs = Codecs::determineRequiredCodecs(getPlaybackInfo());
  setPreferredCodecs(codecs);
  fetcher->installCodecs(codecs);
}

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::onCodecsLoadingDone(CodecsFetcher* sender)
{
  sender->deleteLater();
  sender->userData.value<std::function<void()>>()();
}

/////////////////////////////////////////////////////////////////////////////////////////
static QString get_mpv_osd(mpv_handle *ctx, const QString& property)
{
  char *s = mpv_get_property_osd_string(ctx, property.toUtf8().data());
  if (!s)
    return "-";
  QString r = QString::fromUtf8(s);
  mpv_free(s);
  if (r.size() > 400)
    r = r.mid(0, 400) + "...";
  Log::CensorAuthTokens(r);
  return r;
}

#define MPV_PROPERTY(p) get_mpv_osd(m_mpv, p)
#define MPV_PROPERTY_BOOL(p) (mpv::qt::get_property(m_mpv, p).toBool())

/////////////////////////////////////////////////////////////////////////////////////////
void PlayerComponent::appendAudioFormat(QTextStream& info, const QString& property) const
{
  // Guess if it's a passthrough format. Don't show the channel layout in this
  // case, because it's confusing.
  QString audioFormat = MPV_PROPERTY(property + "/format");
  if (audioFormat.startsWith("spdif-"))
  {
    info << "passthrough (" << audioFormat.mid(6) << ")";
  }
  else
  {
    QString hr = MPV_PROPERTY(property + "/hr-channels");
    QString full = MPV_PROPERTY(property + "/channels");
    info << hr;
    if (hr != full)
      info << " (" << full << ")";
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
QString PlayerComponent::videoInformation() const
{
  QString infoStr;
  QTextStream info(&infoStr);

  // check if video is playing
  if (mpv::qt::get_property(m_mpv, "idle-active").toBool())
    return "";

  info << "File:\n";
  info << "URL: " << MPV_PROPERTY("path") << "\n";
  info << "Container: " << MPV_PROPERTY("file-format") << "\n";
  info << "Native seeking: " << ((MPV_PROPERTY_BOOL("seekable") &&
                                  !MPV_PROPERTY_BOOL("partially-seekable"))
                                 ? "yes" : "no") << "\n";
  info << "\n";
  info << "Video:\n";
  info << "Codec: " << MPV_PROPERTY("video-codec") << "\n";
  info << "Size: " << MPV_PROPERTY("video-params/dw") << "x"
                   << MPV_PROPERTY("video-params/dh") << "\n";
  info << "FPS (container): " << MPV_PROPERTY("container-fps") << "\n";
  info << "FPS (filters): " << MPV_PROPERTY("estimated-vf-fps") << "\n";
  info << "Aspect: " << MPV_PROPERTY("video-params/aspect") << "\n";
  info << "Bitrate: " << MPV_PROPERTY("video-bitrate") << "\n";
  double displayFps = DisplayComponent::Get().currentRefreshRate();
  info << "Display FPS: " << MPV_PROPERTY("override-display-fps")
                          << " (" << displayFps << ")" << "\n";
  info << "Hardware Decoding: " << MPV_PROPERTY("hwdec-current")
                                << " (" << MPV_PROPERTY("hwdec-interop") << ")\n";
  info << "\n";
  info << "Audio:\n";
  info << "Codec: " << MPV_PROPERTY("audio-codec") << "\n";
  info << "Bitrate: " << MPV_PROPERTY("audio-bitrate") << "\n";
  info << "Channels: ";
  appendAudioFormat(info, "audio-params");
  info << " -> ";
  appendAudioFormat(info, "audio-out-params");
  info << "\n";
  info << "Output driver: " << MPV_PROPERTY("current-ao") << "\n";
  info << "\n";
  info << "Performance:\n";
  info << "A/V: " << MPV_PROPERTY("avsync") << "\n";
  info << "Dropped frames: " << MPV_PROPERTY("vo-drop-frame-count") << "\n";
  bool dispSync = MPV_PROPERTY_BOOL("display-sync-active");
  info << "Display Sync: ";
  if (!dispSync)
  {
     info << "no\n";
  }
  else
  {
    info << "yes (ratio " << MPV_PROPERTY("vsync-ratio") << ")\n";
    info << "Mistimed frames: " << MPV_PROPERTY("mistimed-frame-count")
                                << "/" << MPV_PROPERTY("vo-delayed-frame-count") << "\n";
    info << "Measured FPS: " << MPV_PROPERTY("estimated-display-fps")
                             << " (" << MPV_PROPERTY("vsync-jitter") << ")\n";
    info << "V. speed corr.: " << MPV_PROPERTY("video-speed-correction") << "\n";
    info << "A. speed corr.: " << MPV_PROPERTY("audio-speed-correction") << "\n";
  }
  info << "\n";
  info << "Cache:\n";
  info << "Seconds: " << MPV_PROPERTY("demuxer-cache-duration") << "\n";
  info << "Extra readahead: " << MPV_PROPERTY("cache-used") << "\n";
  info << "Buffering: " << MPV_PROPERTY("cache-buffering-state") << "\n";
  info << "Speed: " << MPV_PROPERTY("cache-speed") << "\n";
  info << "\n";
  info << "Misc:\n";
  info << "Time: " << MPV_PROPERTY("playback-time") << " / "
                   << MPV_PROPERTY("duration")
                   << " (" << MPV_PROPERTY("percent-pos") << "%)\n";
  info << "State: " << (MPV_PROPERTY_BOOL("pause") ? "paused " : "")
                    << (MPV_PROPERTY_BOOL("paused-for-cache") ? "buffering " : "")
                    << (MPV_PROPERTY_BOOL("core-idle") ? "waiting " : "playing ")
                    << (MPV_PROPERTY_BOOL("seeking") ? "seeking " : "")
                    << "\n";

  info.flush();
  return infoStr;
}

