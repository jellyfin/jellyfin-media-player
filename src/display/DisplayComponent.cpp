
#include "QsLog.h"
#include "DisplayComponent.h"
#include "DisplayManager.h"
#include "settings/SettingsComponent.h"
#include <QGuiApplication>
#include <QWindow>

#ifdef Q_OS_MAC
#include "osx/DisplayManagerOSX.h"
#elif defined(TARGET_RPI)
#include "rpi/DisplayManagerRPI.h"
#elif defined(USE_X11XRANDR)
#include "x11/DisplayManagerX11.h"
#elif defined(Q_OS_WIN)
#include "win/DisplayManagerWin.h"
#endif

#include "dummy/DisplayManagerDummy.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayComponent::DisplayComponent(QObject* parent) : ComponentBase(parent), m_initTimer(this)
{
  m_displayManager = NULL;
  m_lastVideoMode = -1;
  m_lastDisplay = -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayComponent::~DisplayComponent()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayComponent::initializeDisplayManager()
{
  m_initTimer.setSingleShot(false);

  bool res = false;
  if (m_displayManager)
    res = m_displayManager->initialize();

  emit refreshRateChanged();

  return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayComponent::componentInitialize()
{
#if 0
  m_displayManager = new DisplayManagerDummy(this);
#elif defined(Q_OS_MAC)
  m_displayManager = new DisplayManagerOSX(this);
#elif defined(TARGET_RPI)
  m_displayManager = new DisplayManagerRPI(this);
#elif defined(USE_X11XRANDR)
  m_displayManager = new DisplayManagerX11(this);
#elif defined(Q_OS_WIN)
  m_displayManager = new DisplayManagerWin(this);
#endif

  if (initializeDisplayManager())
  {
    QGuiApplication* app = (QGuiApplication*)QGuiApplication::instance();

    connect(app, SIGNAL(screenAdded(QScreen*)), this, SLOT(monitorChange()));
    connect(app, SIGNAL(screenRemoved(QScreen*)), this,  SLOT(monitorChange()));

    foreach (QScreen *screen, app->screens())
    {
      connect(screen, SIGNAL(refreshRateChanged(qreal)), this, SLOT(monitorChange()));
      connect(screen, SIGNAL(geometryChanged(QRect)), this, SLOT(monitorChange()));
    }

#ifdef TARGET_RPI
    // The firmware doesn't always make the best decision. Hope we do better.
    QLOG_INFO() << "Trying to switch to best display mode.";
    switchToBestOverallVideoMode(0);
#endif

    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayComponent::monitorChange()
{
  QLOG_INFO() << "Monitor change detected.";

  if (!m_initTimer.isSingleShot())
  {
    m_initTimer.setSingleShot(true);
    m_initTimer.singleShot(1000, this, SLOT(initializeDisplayManager()));
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayComponent::switchToBestVideoMode(float frameRate)
{
  if (!m_displayManager)
    return false;

  int currentDisplay = getApplicationDisplay();
  if (currentDisplay < 0)
    return false;

  int currentMode = m_displayManager->getCurrentDisplayMode(currentDisplay);
  if (m_lastVideoMode < 0)
  {
    m_lastVideoMode = currentMode;
    m_lastDisplay = currentDisplay;
  }

  DMMatchMediaInfo matchInfo(frameRate, false);
  int bestmode = m_displayManager->findBestMatch(currentDisplay, matchInfo);
  if (bestmode >= 0)
  {
    if (bestmode != currentMode)
    {
      QLOG_DEBUG()
      << "Best video matching mode is "
      << m_displayManager->displays[currentDisplay]->videoModes[bestmode]->getPrettyName()
      << "on display" << currentDisplay;

      m_displayManager->setDisplayMode(currentDisplay, bestmode);
      return true;
    }
    QLOG_INFO() << "No better video mode than the currently active one found.";
  }
  else
  {
    QLOG_DEBUG() << "No video mode found as better match.";
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayComponent::switchToBestOverallVideoMode(int display)
{
  if (!m_displayManager || !m_displayManager->isValidDisplay(display))
    return false;

  if (!SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "hdmi_poweron").toBool())
  {
    QLOG_INFO() << "Switching to best mode disabled.";
    return false;
  }

  int bestmode = m_displayManager->findBestMode(display);
  if (bestmode < 0)
    return false;

  QLOG_INFO() << "We think mode" << bestmode << "is the best mode.";

  if (bestmode == m_displayManager->getCurrentDisplayMode(display))
  {
    QLOG_INFO() << "This mode is the currently active one. Not switching.";
    return false;
  }

  if (!m_displayManager->setDisplayMode(display, bestmode))
  {
    QLOG_INFO() << "Switching mode failed.";
    return false;
  }

  QLOG_INFO() << "Switching mode successful.";
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
double DisplayComponent::currentRefreshRate()
{
  if (!m_displayManager)
    return 0;

  int currentDisplay = getApplicationDisplay();
  if (currentDisplay < 0)
    return 0;
  int mode = m_displayManager->getCurrentDisplayMode(currentDisplay);
  if (mode < 0)
    return 0;
  return m_displayManager->displays[currentDisplay]->videoModes[mode]->refreshRate;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayComponent::restorePreviousVideoMode()
{
  if (!m_displayManager)
    return false;

  if (!m_displayManager->isValidDisplayMode(m_lastDisplay, m_lastVideoMode))
    return false;

  bool ret = true;

  if (m_displayManager->getCurrentDisplayMode(m_lastDisplay) != m_lastVideoMode)
  {
    QLOG_DEBUG()
    << "Restoring VideoMode to"
    << m_displayManager->displays[m_lastDisplay]->videoModes[m_lastVideoMode]->getPrettyName()
    << "on display" << m_lastDisplay;

    ret =  m_displayManager->setDisplayMode(m_lastDisplay, m_lastVideoMode);
  }

  m_lastVideoMode = -1;
  m_lastDisplay = -1;

  return ret;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayComponent::getApplicationDisplay()
{
  QWindow* activeWindow = QGuiApplication::focusWindow();

  int display = -1;
  if (activeWindow && m_displayManager)
    display = m_displayManager->getDisplayFromPoint(activeWindow->geometry().center());

  if (display < 0)
  {
    QLOG_WARN() << "Unable to locate current display.";
  }
  return display;
}
