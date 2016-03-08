#include "KonvergoWindow.h"
#include <QTimer>
#include <QJsonObject>
#include <QScreen>
#include <QQuickItem>
#include <QGuiApplication>

#include "input/InputKeyboard.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "system/SystemComponent.h"
#include "player/PlayerComponent.h"
#include "player/PlayerQuickItem.h"
#include "display/DisplayComponent.h"
#include "QsLog.h"
#include "power/PowerComponent.h"
#include "utils/Utils.h"
#include "KonvergoEngine.h"
#include "EventFilter.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
KonvergoWindow::KonvergoWindow(QWindow* parent) : QQuickWindow(parent), m_debugLayer(false), m_lastScale(1.0)
{
  // NSWindowCollectionBehaviorFullScreenPrimary is only set on OSX if Qt::WindowFullscreenButtonHint is set on the window.
  setFlags(flags() | Qt::WindowFullscreenButtonHint);

  m_infoTimer = new QTimer(this);
  m_infoTimer->setInterval(1000);

  installEventFilter(new EventFilter(this));

  connect(m_infoTimer, &QTimer::timeout, this, &KonvergoWindow::updateDebugInfo);

  InputComponent::Get().registerHostCommand("close", this, "close");
  InputComponent::Get().registerHostCommand("toggleDebug", this, "toggleDebug");
  InputComponent::Get().registerHostCommand("reload", this, "reloadWeb");
  InputComponent::Get().registerHostCommand("fullscreen", this, "toggleFullscreen");

#ifdef TARGET_RPI
  // On RPI, we use dispmanx layering - the video is on a layer below Konvergo,
  // and during playback the Konvergo window is partially transparent. The OSD
  // will be visible on top of the video as part of the Konvergo window.
  setColor(QColor("transparent"));
#else
  setColor(QColor("#111111"));
#endif

  loadGeometry();
  m_lastScale = CalculateScale(size());

  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_MAIN), &SettingsSection::valuesUpdated,
          this, &KonvergoWindow::updateMainSectionSettings);

  connect(this, &KonvergoWindow::visibilityChanged,
          this, &KonvergoWindow::onVisibilityChanged);

  connect(this, &KonvergoWindow::enableVideoWindowSignal,
          this, &KonvergoWindow::enableVideoWindow, Qt::QueuedConnection);

//  connect(QGuiApplication::desktop(), &QDesktopWidget::screenCountChanged,
//              this, &KonvergoWindow::onScreenCountChanged);

  connect(&PlayerComponent::Get(), &PlayerComponent::windowVisible,
          this, &KonvergoWindow::playerWindowVisible);

  connect(&PlayerComponent::Get(), &PlayerComponent::playbackStarting,
          this, &KonvergoWindow::playerPlaybackStarting);

  // this is using old syntax because ... reasons. QQuickCloseEvent is not public class
  connect(this, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(closingWindow()));

  connect(qApp, &QCoreApplication::aboutToQuit, this, &KonvergoWindow::saveGeometry);

  if (!SystemComponent::Get().isOpenELEC())
  {
    // this is such a hack. But I could not get it to enter into fullscreen
    // mode if I didn't trigger this after a while.
    //
    QTimer::singleShot(500, [=]() {
        updateFullscreenState();
    });
  }
  else
  {
    setWindowState(Qt::WindowFullScreen);
  }

  emit enableVideoWindowSignal();
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::closingWindow()
{
  if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "fullscreen").toBool() == false)
    saveGeometry();

  qApp->quit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
KonvergoWindow::~KonvergoWindow()
{
  DisplayComponent::Get().setApplicationWindow(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::saveGeometry()
{
  QRect rc = geometry();
  QJsonObject obj;
  obj.insert("x", rc.x());
  obj.insert("y", rc.y());
  obj.insert("width", rc.width());
  obj.insert("height", rc.height());
  SettingsComponent::Get().setValue(SETTINGS_SECTION_STATE, "geometry", obj.toVariantMap());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::loadGeometry()
{
  QRect rc = loadGeometryRect();
  if (rc.isValid())
    setGeometry(rc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QRect KonvergoWindow::loadGeometryRect()
{
  QJsonObject obj = QJsonObject::fromVariantMap(SettingsComponent::Get().value(SETTINGS_SECTION_STATE, "geometry").toMap());
  if (obj.isEmpty())
    return QRect();

  QRect rc(obj["x"].toInt(), obj["y"].toInt(), obj["width"].toInt(), obj["height"].toInt());

  if (rc.width() < 1280)
    rc.setWidth(1280);

  if (rc.height() < 720)
    rc.setHeight(720);

  // make sure our poisition is contained in one of the current screens
  foreach (QScreen *screen, QGuiApplication::screens())
  {
    if (screen->availableGeometry().contains(rc))
      return rc;
  }

 // otherwise default to center of current screen
  return QRect((screen()->geometry().width() - geometry().width()) / 2,
               (screen()->geometry().height() - geometry().height()) / 2,
               geometry().width(),
               geometry().height());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::enableVideoWindow()
{
  PlayerComponent::Get().setWindow(this);
  DisplayComponent::Get().setApplicationWindow(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::setFullScreen(bool enable)
{
  QLOG_DEBUG() << "setting fullscreen = " << enable;

  SettingsComponent::Get().setValue(SETTINGS_SECTION_MAIN, "fullscreen", enable);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::playerWindowVisible(bool visible)
{
  // adjust webengineview transparecy depending on player visibility
  QQuickItem *web = findChild<QQuickItem *>("web");
  if (web)
    web->setProperty("backgroundColor", visible ? "transparent" : "#111111");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::updateMainSectionSettings(const QVariantMap& values)
{
  // update mouse visibility if needed
  if (values.find("disablemouse") != values.end())
  {
    SystemComponent::Get().setCursorVisibility(!SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "disablemouse").toBool());
  }

  if (values.find("fullscreen") == values.end())
    return;

  updateFullscreenState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::updateFullscreenState()
{
  if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "fullscreen").toBool() || SystemComponent::Get().isOpenELEC())
  {
    // if we were go from windowed to fullscreen
    // we want to stor our current windowed position
    if (!isFullScreen())
      saveGeometry();

      setVisibility(QWindow::FullScreen);
  }
  else
  {
    setVisibility(QWindow::Windowed);
    loadGeometry();
  }
  
  
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::onVisibilityChanged(QWindow::Visibility visibility)
{
  QLOG_DEBUG() << (visibility == QWindow::FullScreen ? "FullScreen" : "Windowed") << "visbility set to " << visibility;

  if (visibility == QWindow::Windowed)
    loadGeometry();

  if (visibility == QWindow::FullScreen)
    PowerComponent::Get().setFullscreenState(true);
  else if (visibility == QWindow::Windowed)
    PowerComponent::Get().setFullscreenState(false);

  notifyScale(size());
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::focusOutEvent(QFocusEvent * ev)
{
#ifdef Q_OS_WIN32
  // Do this to workaround DWM compositor bugs with fullscreened OpenGL applications.
  // The compositor will not properly redraw anything when focusing other windows.
  if (visibility() == QWindow::FullScreen && SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "minimizeOnDefocus").toBool())
  {
    QLOG_DEBUG() << "minimizing window";
    showMinimized();
  }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::playerPlaybackStarting()
{
#if defined(Q_OS_MAC)
  // On OSX, initializing VideoTooolbox (hardware decoder API) will mysteriously
  // show the hidden mouse pointer again. The VTDecompressionSessionCreate API
  // function does this, and we have no influence over its behavior. To make sure
  // the cursor is gone again when starting playback, listen to the player's
  // playbackStarting signal, at which point decoder initialization is guaranteed
  // to be completed. Then we just have to set the cursor again on the Cocoa level.
  if (QGuiApplication::overrideCursor())
    QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::RegisterClass()
{
  qmlRegisterType<KonvergoWindow>("Konvergo", 1, 0, "KonvergoWindow");
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::onScreenCountChanged(int newCount)
{
  updateFullscreenState();
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::updateDebugInfo()
{
  if (m_systemDebugInfo.size() == 0)
    m_systemDebugInfo = SystemComponent::Get().debugInformation();
  m_debugInfo = m_systemDebugInfo;
  m_debugInfo += DisplayComponent::Get().debugInformation();
  PlayerQuickItem* video = findChild<PlayerQuickItem*>("video");
  if (video)
    m_debugInfo += video->debugInfo();
  m_videoInfo = PlayerComponent::Get().videoInformation();
  emit debugInfoChanged();
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::toggleDebug()
{
  if (property("showDebugLayer").toBool())
  {
    m_infoTimer->stop();
    setProperty("showDebugLayer", false);
  }
  else
  {
    m_infoTimer->start();
    updateDebugInfo();
    setProperty("showDebugLayer", true);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::notifyScale(const QSize& size)
{
  qreal scale = CalculateScale(size);
  if (scale != m_lastScale)
  {
    QLOG_DEBUG() << "windowScale updated to:" << scale;
    m_lastScale = scale;

    emit SystemComponent::Get().scaleChanged(CalculateWebScale(size, devicePixelRatio()));
  }
  emit webScaleChanged();
}


/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::resizeEvent(QResizeEvent* event)
{
  notifyScale(event->size());
  QQuickWindow::resizeEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////////
#define ROUND(x) (qRound(x * 1000) / 1000.0)

/////////////////////////////////////////////////////////////////////////////////////////
qreal KonvergoWindow::CalculateScale(const QSize& size)
{
  qreal horizontalScale = (qreal)size.width() / (qreal)WEBUI_SIZE.width();
  qreal verticalScale = (qreal)size.height() / (qreal)WEBUI_SIZE.height();
  return ROUND(qMin(horizontalScale, verticalScale));
}

/////////////////////////////////////////////////////////////////////////////////////////
qreal KonvergoWindow::CalculateWebScale(const QSize& size, qint32 devicePixelRatio)
{
  qreal horizontalScale = (qreal)size.width() / (qreal)WEBUI_SIZE.width();
  qreal verticalScale = (qreal)size.height() / (qreal)WEBUI_SIZE.height();

  qreal minScale = qMin(horizontalScale, qMin(verticalScale, (qreal)(WEBUI_MAX_HEIGHT / devicePixelRatio) / (qreal)WEBUI_SIZE.height()));
  return ROUND(qMax(1.0, minScale));
}
