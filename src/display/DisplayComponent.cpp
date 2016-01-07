
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
#include "input/InputComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayComponent::DisplayComponent(QObject* parent) : ComponentBase(parent), m_initTimer(this)
{
  m_displayManager = NULL;
  m_lastVideoMode = -1;
  m_lastDisplay = -1;
  m_applicationWindow = NULL;
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
  {
    QLOG_INFO() << "Not switching rate - current display not found.";
    return false;
  }

  int currentMode = m_displayManager->getCurrentDisplayMode(currentDisplay);
  if (m_lastVideoMode < 0)
  {
    m_lastVideoMode = currentMode;
    m_lastDisplay = currentDisplay;
  }

  QLOG_DEBUG() << "Current display:" << currentDisplay << "mode:" << currentMode;

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

      return m_displayManager->setDisplayMode(currentDisplay, bestmode);
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
int DisplayComponent::getApplicationDisplay(bool silent)
{
  QWindow* activeWindow = m_applicationWindow;

  int display = -1;
  if (activeWindow && m_displayManager)
  {
    if (!silent)
    {
      QLOG_DEBUG() << "Looking for a display at:" << activeWindow->geometry()
                   << "(center:" << activeWindow->geometry().center() << ")";
    }
    display = m_displayManager->getDisplayFromPoint(activeWindow->geometry().center());
  }

  if (!silent)
  {
    QLOG_DEBUG() << "Display index:" << display;
  }
  return display;
}

/////////////////////////////////////////////////////////////////////////////////////////
QString DisplayComponent::displayName(int display)
{
  if (display < 0)
    return "(not found)";
  QString id = QString("#%0 ").arg(display);
  if (m_displayManager->isValidDisplay(display))
    return id + m_displayManager->displays[display]->name;
  else
    return id + "(not valid)";
}

/////////////////////////////////////////////////////////////////////////////////////////
QString DisplayComponent::modePretty(int display, int mode)
{
  if (mode < 0)
    return "(not found)";
  QString id = QString("#%0 ").arg(mode);
  if (m_displayManager->isValidDisplayMode(display, mode))
    return id + m_displayManager->displays[display]->videoModes[mode]->getPrettyName();
  else
    return id + "(not valid)";
}

/////////////////////////////////////////////////////////////////////////////////////////
QString DisplayComponent::debugInformation()
{
  QString debugInfo;
  QTextStream stream(&debugInfo);
  stream << "Display" << endl;

  if (!m_displayManager)
  {
    stream << "  (no DisplayManager initialized)" << endl;
  }
  else
  {
    int display = getApplicationDisplay(true);
    int mode = display < 0 ? -1 : m_displayManager->getCurrentDisplayMode(display);

    stream << "  Current screen: " << displayName(display) << endl;
    if (display >= 0)
      stream << "  Current mode: " << modePretty(display, mode) << endl;
    if (m_displayManager->isValidDisplayMode(m_lastDisplay, m_lastVideoMode))
    {
      stream << "  Switch back on screen: " << displayName(m_lastDisplay) << endl;
      stream << "  Switch back to mode: " << modePretty(m_lastDisplay, m_lastVideoMode) << endl;
    }
  }

  stream << endl;
  stream << flush;
  return debugInfo;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static bool modeEqualsFuzzy(const DMVideoMode& m1, const DMVideoMode& m2, float tolerance)
{
  return m1.height == m2.height &&
         m1.width == m2.width &&
         fabs(m1.refreshRate - m2.refreshRate) < tolerance &&
         m1.bitsPerPixel == m2.bitsPerPixel &&
         m1.interlaced == m2.interlaced;
}

/////////////////////////////////////////////////////////////////////////////////////////
void DisplayComponent::switchCommand(QString command)
{
  if (!m_displayManager)
  {
    QLOG_ERROR() << "Display manager not set";
    return;
  }

  if (!initializeDisplayManager())
  {
    QLOG_ERROR() << "Could not reinitialize display manager";
    return;
  }

  int currentDisplay = getApplicationDisplay();
  if (currentDisplay < 0)
  {
    QLOG_ERROR() << "Current display not found";
    return;
  }
  int id = m_displayManager->getCurrentDisplayMode(currentDisplay);
  if (id < 0)
  {
    QLOG_ERROR() << "Current mode not found";
    return;
  }
  DMVideoMode current_mode = *m_displayManager->displays[currentDisplay]->videoModes[id];
  DMVideoMode mode = current_mode;

  foreach (QString a, command.split(" "))
  {
    a = a.trimmed();
    if (a == "p")
    {
      mode.interlaced = false;
    }
    else if (a == "i")
    {
      mode.interlaced = true;
    }
    else if (a.endsWith("hz"))
    {
      a = a.mid(0, a.size() - 2);
      bool ok;
      float rate = a.toFloat(&ok);
      if (ok)
        mode.refreshRate = rate;
    }
    else if (a.indexOf("x") >= 0)
    {
      QStringList sub = a.split("x");
      if (sub.size() != 2)
        continue;
      bool ok;
      int w = sub[0].toInt(&ok);
      if (!ok)
        continue;
      int h = sub[1].toInt(&ok);
      if (!ok)
        continue;
      mode.width = w;
      mode.height = h;
    }
  }

  QLOG_INFO() << "Current mode:" << current_mode.getPrettyName();
  QLOG_INFO() << "Mode requested by command:" << mode.getPrettyName();

  foreach (auto cur, m_displayManager->displays[currentDisplay]->videoModes)
  {
    // Require matching one digit before the decimal point.
    // This doesn't work if there are several modes that would match. To do
    // this without requiring the user to give the refresh rate with full
    // precision, we'd have to find a closest match instead.
    if (modeEqualsFuzzy(*cur, mode, 0.1))
    {
      QLOG_INFO() << "Found mode to switch to:" << cur->getPrettyName();
      if (m_displayManager->setDisplayMode(currentDisplay, cur->id))
      {
        m_lastDisplay = m_lastVideoMode = -1;
      }
      else
      {
        QLOG_INFO() << "Switching failed.";
      }
      return;
    }
  }

  QLOG_INFO() << "Requested mode not found.";
}

/////////////////////////////////////////////////////////////////////////////////////////
void DisplayComponent::componentPostInitialize()
{
  InputComponent::Get().registerHostCommand("switch", this, "switchCommand");

  if (m_displayManager)
    InputComponent::Get().registerHostCommand("recreateRpiUI", m_displayManager, "resetRendering");
}
