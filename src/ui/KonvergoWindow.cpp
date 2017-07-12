#include "KonvergoWindow.h"
#include <QTimer>
#include <QJsonObject>
#include <QScreen>
#include <QQuickItem>
#include <QGuiApplication>
#include <QMessageBox>
#include <QPushButton>

#include "core/Version.h"
#include "system/UpdaterComponent.h"
#include "input/InputKeyboard.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "system/SystemComponent.h"
#include "player/PlayerComponent.h"
#include "player/PlayerQuickItem.h"
#include "display/DisplayComponent.h"
#include "QsLog.h"
#include "utils/Utils.h"
#include "Globals.h"
#include "EventFilter.h"

#define MAX_RECURSION_DEPTH 50

///////////////////////////////////////////////////////////////////////////////////////////////////
class ScopedDecrementer
{
  Q_DISABLE_COPY(ScopedDecrementer)

  int* m_value;

public:
  ScopedDecrementer(int* value) : m_value(value) {}
  ~ScopedDecrementer() { (*m_value)--; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
KonvergoWindow::KonvergoWindow(QWindow* parent) :
  QQuickWindow(parent),
  m_debugLayer(false),
  m_ignoreFullscreenSettingsChange(0),
  m_showedUpdateDialog(false),
  m_osxPresentationOptions(0),
  m_eventRecursionDepth(0)
{
  // NSWindowCollectionBehaviorFullScreenPrimary is only set on OSX if Qt::WindowFullscreenButtonHint is set on the window.
  setFlags(flags() | Qt::WindowFullscreenButtonHint);

  m_infoTimer = new QTimer(this);
  m_infoTimer->setInterval(1000);
  m_webDesktopMode = (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "webMode").toString() == "desktop");

  installEventFilter(new EventFilter(this));

  connect(m_infoTimer, &QTimer::timeout, this, &KonvergoWindow::updateDebugInfo);

  InputComponent::Get().registerHostCommand("close", this, "close");
  InputComponent::Get().registerHostCommand("toggleDebug", this, "toggleDebug");
  InputComponent::Get().registerHostCommand("reload", this, "reloadWeb");
  InputComponent::Get().registerHostCommand("fullscreen", this, "toggleFullscreen");
  InputComponent::Get().registerHostCommand("minimize", this, "minimizeWindow");
  InputComponent::Get().registerHostCommand("fullscreenCurrentMode", this, "toggleFullscreenNoSwitch");

#ifdef TARGET_RPI
  // On RPI, we use dispmanx layering - the video is on a layer below Konvergo,
  // and during playback the Konvergo window is partially transparent. The OSD
  // will be visible on top of the video as part of the Konvergo window.
  setColor(QColor("transparent"));
#else
  setColor(QColor("#000000"));
#endif

  QRect loadedGeo = loadGeometry();

  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_MAIN), &SettingsSection::valuesUpdated,
          this, &KonvergoWindow::updateMainSectionSettings);

  connect(this, &KonvergoWindow::visibilityChanged,
          this, &KonvergoWindow::onVisibilityChanged);

  connect(this, &KonvergoWindow::screenChanged,
          this, &KonvergoWindow::updateCurrentScreen);
  connect(this, &KonvergoWindow::xChanged,
          this, &KonvergoWindow::updateCurrentScreen);
  connect(this, &KonvergoWindow::yChanged,
          this, &KonvergoWindow::updateCurrentScreen);
  connect(this, &KonvergoWindow::visibilityChanged,
          this, &KonvergoWindow::updateCurrentScreen);
  connect(this, &KonvergoWindow::windowStateChanged,
          this, &KonvergoWindow::updateCurrentScreen);

  connect(this, &KonvergoWindow::enableVideoWindowSignal,
          this, &KonvergoWindow::enableVideoWindow, Qt::QueuedConnection);

  connect(&PlayerComponent::Get(), &PlayerComponent::windowVisible,
          this, &KonvergoWindow::playerWindowVisible);

  // this is using old syntax because ... reasons. QQuickCloseEvent is not public class
  connect(this, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(closingWindow()));

  connect(qApp, &QCoreApplication::aboutToQuit, this, &KonvergoWindow::closingWindow);

  connect(&UpdaterComponent::Get(), &UpdaterComponent::downloadComplete,
          this, &KonvergoWindow::showUpdateDialog);

#ifdef Q_OS_MAC
  m_osxPresentationOptions = 0;
#endif

#ifdef KONVERGO_OPENELEC
  setVisibility(QWindow::FullScreen);
#else
  updateWindowState(false);
#endif

  updateScreens();

  connect(qApp, &QGuiApplication::screenAdded, this, &KonvergoWindow::onScreenAdded);
  connect(qApp, &QGuiApplication::screenRemoved, this, &KonvergoWindow::onScreenRemoved);

  emit enableVideoWindowSignal();
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::showUpdateDialog()
{
  if (m_webDesktopMode && !m_showedUpdateDialog)
  {
    QVariantHash updateInfo = UpdaterComponent::Get().updateInfo();

    QString currentVersion = Version::GetCanonicalVersionString().split("-")[0];
    QString newVersion = updateInfo["version"].toString().split("-")[0];

    QMessageBox* message = new QMessageBox(nullptr);
    message->setIcon(QMessageBox::Information);
    message->setWindowModality(Qt::ApplicationModal);
    message->setWindowTitle("Update found!");
    message->setText("An update to Plex Media Player was found");
    auto infoText = QString("You are currently running version %0\nDo you wish to install version %1 now?")
      .arg(currentVersion)
      .arg(newVersion);
    message->setInformativeText(infoText);

    auto details = QString("ChangeLog for version %0\n\nNew:\n%1\n\nFixed:\n%2")
      .arg(newVersion)
      .arg(updateInfo["new"].toString())
      .arg(updateInfo["fixed"].toString());

    message->setDetailedText(details);

    auto updateNow = message->addButton("Install Now", QMessageBox::AcceptRole);
    auto updateLater = message->addButton("Install on Next Restart", QMessageBox::RejectRole);

    message->setDefaultButton(updateNow);

    m_showedUpdateDialog = true;
    connect(message, &QMessageBox::buttonClicked, [=](QAbstractButton* button)
    {
      if (button == updateNow)
        UpdaterComponent::Get().doUpdate();
      else if (button == updateLater)
        message->close();

      message->deleteLater();
    });

    message->show();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::closingWindow()
{
  if (!SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "fullscreen").toBool())
    saveGeometry();

  qApp->quit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
KonvergoWindow::~KonvergoWindow()
{
  DisplayComponent::Get().setApplicationWindow(nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool KonvergoWindow::fitsInScreens(const QRect& rc)
{
  for(QScreen *screen : QGuiApplication::screens())
  {
    if (screen->virtualGeometry().isValid() && screen->virtualGeometry().contains(rc))
      return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::saveGeometry()
{
  QLOG_DEBUG() << "Window state when saving geometry:" << visibility();

  QRect rc = geometry();

  // lets make sure we are not saving something craycray
  if (rc.size().width() < windowMinSize().width() || rc.size().height() < windowMinSize().height())
    return;

  if (!fitsInScreens(rc))
    return;

  QLOG_DEBUG() << "Saving window geometry:" << rc;

  QVariantMap map = {{"x", rc.x()}, {"y", rc.y()},
                     {"width", rc.width()}, {"height", rc.height()}};
  SettingsComponent::Get().setValue(SETTINGS_SECTION_STATE, "geometry", map);
  QScreen *curScreen = screen();
  SettingsComponent::Get().setValue(SETTINGS_SECTION_STATE, "lastUsedScreen", curScreen ? curScreen->name() : "");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QRect KonvergoWindow::loadGeometry()
{
  QRect rc = loadGeometryRect();
  QScreen* myScreen = loadLastScreen();
  if (!myScreen)
    myScreen = screen();

  QRect nsize = rc;

  if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "fullscreen").toBool())
  {
    QLOG_DEBUG() << "Load FullScreen geo...";

    if (myScreen)
    {
      // On OSX we need to set the geometry to the size we want when we
      // return from fullscreen otherwise when we exit fullscreen it
      // will stay small or big. On Windows we need to set it to max
      // resolution for the screen (i.e. fullscreen) otherwise it will
      // just scale the webcontent to the minimum size we have defined
      //
  #ifndef Q_OS_MAC
      nsize = myScreen->geometry();
  #endif
      setGeometry(nsize);

      setScreen(myScreen);
    }
  }
  else
  {
    setGeometry(nsize);
    saveGeometry();
  }

  return nsize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QRect KonvergoWindow::loadGeometryRect()
{
  // if we dont have anything, default to 720p in the middle of the screen
  QScreen *curScreen = screen();
  QRect defaultRect = QRect(0, 0, WEBUI_SIZE.width(), WEBUI_SIZE.height());
  if (curScreen)
  {
    defaultRect = QRect((curScreen->geometry().width() - WEBUI_SIZE.width()) / 2,
                        (curScreen->geometry().height() - WEBUI_SIZE.height()) / 2,
                        WEBUI_SIZE.width(), WEBUI_SIZE.height());
  }

  QVariantMap map = SettingsComponent::Get().value(SETTINGS_SECTION_STATE, "geometry").toMap();
  if (map.isEmpty())
    return defaultRect;

  QRect rc(map["x"].toInt(), map["y"].toInt(), map["width"].toInt(), map["height"].toInt());

  QLOG_DEBUG() << "Restoring geo:" << rc;

  if (!rc.isValid() || rc.isEmpty())
  {
    QLOG_DEBUG() << "Geo bad, going for defaults";
    return defaultRect;
  }

  QSize minsz = windowMinSize();

  // Clamp to min size if we have really small values in there
  if (rc.size().width() < minsz.width())
    rc.setWidth(minsz.width());
  if (rc.size().height() < minsz.height())
    rc.setHeight(minsz.height());

  // also make sure we are not putting windows outside the screen somewhere
  if (!fitsInScreens(rc))
  {
    QLOG_DEBUG() << "Could not fit stored geo into current screens";
    return defaultRect;
  }

  return rc;
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
void KonvergoWindow::toggleFullscreen(bool noSwitchMode)
{
  bool switchMode = (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "layout").toString() == "auto" && !noSwitchMode);

  if (switchMode && !m_webDesktopMode && isFullScreen())
    SettingsComponent::Get().setValue(SETTINGS_SECTION_MAIN, "webMode", "desktop");
  else if (switchMode && m_webDesktopMode && !isFullScreen())
    SettingsComponent::Get().setValue(SETTINGS_SECTION_MAIN, "webMode", "tv");
  else
    setFullScreen(!isFullScreen());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::setAlwaysOnTop(bool enable)
{
  QLOG_DEBUG() << "setting always on top = " << enable;

  // Update the settings value.
  SettingsComponent::Get().setValue(SETTINGS_SECTION_MAIN, "alwaysOnTop", enable);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::playerWindowVisible(bool visible)
{
  // adjust webengineview transparecy depending on player visibility
  QQuickItem *web = findChild<QQuickItem *>("web");
  if (web)
    web->setProperty("backgroundColor", visible ? "transparent" : "#000000");

#ifdef Q_OS_MAC
  // On OSX, initializing VideoTooolbox (hardware decoder API) will mysteriously
  // show the hidden mouse pointer again. The VTDecompressionSessionCreate API
  // function does this, and we have no influence over its behavior.
  if (visible && !SystemComponent::Get().cursorVisible())
  {
    // "Refresh" it. (There doesn't seem to be a nicer way, and we have to do
    // this on the Cocoa level too.)
    SystemComponent::Get().setCursorVisibility(true);
    SystemComponent::Get().setCursorVisibility(false);
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::updateMainSectionSettings(const QVariantMap& values)
{
  m_eventRecursionDepth++;
  ScopedDecrementer decrement(&m_eventRecursionDepth);
  if (m_eventRecursionDepth > MAX_RECURSION_DEPTH)
  {
    QLOG_ERROR() << "Maximum recursion depth reached! (updateMainSectionSettings)";
    return;
  }

  // update mouse visibility if needed
  if (values.find("disablemouse") != values.end())
  {
    SystemComponent::Get().setCursorVisibility(!SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "disablemouse").toBool());
  }

  if (values.contains("alwaysOnTop"))
    updateWindowState();

  if (values.contains("fullscreen") && !m_ignoreFullscreenSettingsChange)
    updateWindowState();

  if (values.contains("webMode"))
  {
    InputComponent::Get().cancelAutoRepeat();
    bool oldDesktopMode = m_webDesktopMode;
    bool newDesktopMode = (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "webMode").toString() == "desktop");

    if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "layout").toString() != "auto")
    {
      if (oldDesktopMode != newDesktopMode)
      {
        PlayerComponent::Get().stop();
        m_webDesktopMode = newDesktopMode;
        emit webDesktopModeChanged();
        emit webUrlChanged();
      }
    }
    else
    {
      bool oldFullscreen = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "fullscreen").toBool();
      bool newFullscreen = oldFullscreen;

      if (oldDesktopMode && !newDesktopMode)
        newFullscreen = true;
      else if (!oldDesktopMode && newDesktopMode)
        newFullscreen = false;

      if (oldFullscreen != newFullscreen)
        SettingsComponent::Get().setValue(SETTINGS_SECTION_MAIN, "fullscreen", newFullscreen);

      if (oldDesktopMode != newDesktopMode)
      {
        QTimer::singleShot(0, [this, newDesktopMode]
        {
          PlayerComponent::Get().stop();
          m_webDesktopMode = newDesktopMode;
          emit webDesktopModeChanged();
          emit webUrlChanged();

          if (m_webDesktopMode)
            SystemComponent::Get().setCursorVisibility(true);
          updateWindowState();
        });
      }
    }
  }

  if (values.contains("startupurl"))
    emit webUrlChanged();

  if (values.contains("forceFSScreen"))
    updateForcedScreen();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::updateForcedScreen()
{
  QString screenName = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceFSScreen").toString();

  for (QScreen* scr : QGuiApplication::screens())
  {
    if (scr->name() == screenName)
    {
      QLOG_DEBUG() << "Forcing screen to" << scr->name();
      setScreen(scr);
      setGeometry(scr->geometry());
      setVisibility(QWindow::FullScreen);
      InputComponent::Get().cancelAutoRepeat();
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::updateWindowState(bool saveGeo)
{
  if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "fullscreen").toBool() || SystemComponent::Get().isOpenELEC() || SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceAlwaysFS").toBool())
  {
    // if we were go from windowed to fullscreen
    // we want to store our current windowed position
    if (!isFullScreen() && saveGeo)
      saveGeometry();

    setVisibility(QWindow::FullScreen);

    // When fullscreening explicitly, we might have to move the window to a
    // different screen, as Qt will fullscreen to the current screen.
    QTimer::singleShot(200, [=]
    {
      updateForcedScreen();
    });
  }
  else
  {
    setVisibility(QWindow::Windowed);
    loadGeometry();

    Qt::WindowFlags forceOnTopFlags = Qt::WindowStaysOnTopHint;
#ifdef Q_WS_X11
    forceOnTopFlags = forceOnTopFlags | Qt::X11BypassWindowManagerHint;
#endif

    if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "alwaysOnTop").toBool())
      setFlags(flags() | forceOnTopFlags);
    else
      setFlags(flags() &~ forceOnTopFlags);
  }

  InputComponent::Get().cancelAutoRepeat();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QScreen* KonvergoWindow::findCurrentScreen()
{
  // Return the screen that contains most of the window. Quite possible that
  // screen() would be sufficient, at least once the Qt bug returning a wrong
  // QScreen on Windows is fixed.
  QScreen *best = nullptr;
  qint64 bestArea = 0;
  for(QScreen* screen : qApp->screens())
  {
    QRect areaRC = screen->geometry().intersected(geometry());
    qint64 area = areaRC.width() * (qint64)areaRC.height();
    if (!best || area > bestArea)
    {
      best = screen;
      bestArea = area;
    }
  }
  return best ? best : screen();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::onVisibilityChanged(QWindow::Visibility visibility)
{
  QLOG_DEBUG() << "QWindow visibility set to" << visibility;

  m_eventRecursionDepth++;
  ScopedDecrementer decrement(&m_eventRecursionDepth);
  if (m_eventRecursionDepth > MAX_RECURSION_DEPTH)
  {
    QLOG_ERROR() << "Maximum recursion depth reached! (onVisibilityChanged)";
    return;
  }

#ifdef Q_OS_WIN32
  if (visibility == QWindow::Windowed)
  {
    QScreen* realScreen = findCurrentScreen();
    if (realScreen && realScreen->geometry() == geometry())
    {
      QLOG_DEBUG() << "winging it!";
      setScreen(realScreen);
      setVisibility(QWindow::FullScreen);
      return;
    }
  }
#endif

  if (visibility == QWindow::Windowed && SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceAlwaysFS").toBool())
  {
    QLOG_WARN() << "Forcing re-entering fullscreen because of forceAlwaysFS setting!";
    updateForcedScreen(); // if a specific screen is forced, try to move the window there
    setVisibility(QWindow::FullScreen);
    return;
  }

  if (visibility == QWindow::FullScreen || visibility == QWindow::Windowed)
  {
    m_ignoreFullscreenSettingsChange++;
    ScopedDecrementer decrement(&m_ignoreFullscreenSettingsChange);

    bool fs = visibility == QWindow::FullScreen;
    SettingsComponent::Get().setValue(SETTINGS_SECTION_MAIN, "fullscreen", fs);

    SystemComponent::Get().setCursorVisibility(false);
  }

#ifdef Q_OS_MAC
  if (visibility == QWindow::Windowed)
  {
    QTimer::singleShot(1 * 1000, [this] { OSXUtils::SetPresentationOptions(m_osxPresentationOptions); });
  }
  else if (visibility == QWindow::FullScreen)
  {
    QTimer::singleShot(1 * 1000, [this] {
      OSXUtils::SetPresentationOptions(m_osxPresentationOptions | OSXUtils::GetPresentationOptionsForFullscreen(!m_webDesktopMode));
    });
  }
#endif

  InputComponent::Get().cancelAutoRepeat();
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

  QQuickWindow::focusOutEvent(ev);
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::RegisterClass()
{
  qmlRegisterType<KonvergoWindow>("Konvergo", 1, 0, "KonvergoWindow");
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
  QString infoString;
  QDebug info(&infoString);
  info << "Qt windowing info:\n";
  info << "  FS: " << visibility() << "\n";
  info << "  Geo: " << geometry() << "\n";
  for (QScreen* scr : QGuiApplication::screens())
  {
    info << "  Screen" << scr->name() << scr->geometry() << "\n";
  }

#ifdef Q_OS_WIN32
  HMONITOR mon = MonitorFromWindow((HWND)winId(), MONITOR_DEFAULTTONEAREST);
  MONITORINFO moninfo = {};
  moninfo.cbSize = sizeof(moninfo);
  RECT winrc;
  if (GetMonitorInfo(mon, &moninfo) &&GetWindowRect((HWND)winId(), &winrc))
  {
    RECT rc = moninfo.rcMonitor;
    info << "  Win32 window" << QString("%1/%2 %3x%4").arg(rc.left).arg(rc.top).arg(rc.right).arg(rc.bottom) << QString("%1/%2 %3x%4").arg(winrc.left).arg(winrc.top).arg(winrc.right).arg(winrc.bottom) << "\n";
  }
#endif

  info << "\n";
  m_debugInfo += infoString;
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
void KonvergoWindow::resizeEvent(QResizeEvent* event)
{
  QLOG_DEBUG() << "resize event:" << event->size();

  // This next block was added at some point to workaround a problem with
  // resizing on windows. Unfortunately it broke the desktop client behavior
  // and when retried on Windows 10 with Qt5.7 the original bug seems to be
  // gone. I'll keep this code around until such a time that we dont get any
  // complaints about it.
  //
  #if 0
  // This next block should never really be needed in a prefect world...
  // Unfortunatly this is an imperfect world and on windows sometimes what
  // would happen on startup is that we got a resize event that would make
  // the window much smaller than fullscreen.
  //

  if (isFullScreen())
  {
    QSize fsSize = screen()->size();
    if (event->size().width() < fsSize.width() || event->size().height() < fsSize.height())
    {
      QLOG_DEBUG() << "Ignoring resize event when in fullscreen...";
      return;
    }
  }
  #endif

  QQuickWindow::resizeEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////////
QScreen* KonvergoWindow::loadLastScreen()
{
  QString screenName = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceFSScreen").toString();
  if (screenName.isEmpty())
    screenName = SettingsComponent::Get().value(SETTINGS_SECTION_STATE, "lastUsedScreen").toString();
  if (screenName.isEmpty())
    return nullptr;

  for (QScreen* scr : QGuiApplication::screens())
  {
    if (scr->name() == screenName)
      return scr;
  }

  QLOG_DEBUG() << "Tried to find screen:" << screenName << "but it was not present";

  return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
QString KonvergoWindow::webUrl()
{
  return SettingsComponent::Get().getWebClientUrl(m_webDesktopMode);
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::updateScreens()
{
  QScreen* windowScreen = findCurrentScreen();
  QString screenName = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceFSScreen").toString();

  QVariantList settingList;

  QVariantMap defentry;
  defentry["value"] = "";
  defentry["title"] = "Auto";

  settingList << defentry;

  bool currentPresent = false;
  int num = 0;
  for(QScreen* screen : qApp->screens())
  {
    QRect rc = screen->geometry();
    bool active = screen == windowScreen;

    QVariantMap entry;
    entry["value"] = screen->name();
    entry["title"] =
      QString("%1,%2 %3x%4").arg(rc.left()).arg(rc.top()).arg(rc.right()).arg(rc.bottom()) +
      " (" + screen->name() + ")" +
      (active ? " *" : "");

    settingList << entry;

    bool selected = screen->name() == screenName;
    if (selected)
      currentPresent = true;

    QLOG_DEBUG() << "Screen" << (num++) << screen << screen->geometry()
                 << screen->virtualGeometry() << "active:" << active
                 << "selected:" << selected;
  }

  if (!currentPresent && !screenName.isEmpty())
  {
    QVariantMap entry;
    entry["value"] = screenName;
    entry["title"] = "[Disconnected: " + screenName + "]";

    settingList << entry;
  }

  SettingsComponent::Get().updatePossibleValues(SETTINGS_SECTION_MAIN, "forceFSScreen", settingList);

  m_currentScreenName = windowScreen ? windowScreen->name() : "";
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::onScreenAdded(QScreen *screen)
{
  updateScreens();
  // The timer is out of fear for chaotic mid-change states.
  QTimer::singleShot(200, [this]
  {
    updateForcedScreen();
  });
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::onScreenRemoved(QScreen *screen)
{
  updateScreens();
}

/////////////////////////////////////////////////////////////////////////////////////////
void KonvergoWindow::updateCurrentScreen()
{
  QScreen* current = findCurrentScreen();
  QString currentName = current ? current->name() : "";
  if (currentName != m_currentScreenName)
    updateScreens();
}
