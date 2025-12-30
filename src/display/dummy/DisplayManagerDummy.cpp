#include <QDebug>

#include "DisplayManagerDummy.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayManagerDummy::addMode(float rate)
{
  if (!m_displays.size())
    return;

  DMVideoModePtr mode = DMVideoModePtr(new DMVideoMode());
  mode->m_id = static_cast<int>(m_displays[0]->m_videoModes.size());
  mode->m_interlaced = false;
  mode->m_refreshRate = rate;
  mode->m_width = 1280;
  mode->m_height = 720;
  mode->m_bitsPerPixel = 0;
  m_displays[0]->m_videoModes[mode->m_id] = mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerDummy::initialize()
{
  m_displays.clear();

  DMDisplayPtr display = DMDisplayPtr(new DMDisplay());
  display->m_id = static_cast<int>(m_displays.size());
  display->m_name = "Dummy display";
  m_displays[display->m_id] = display;

  addMode(60);

  return DisplayManager::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerDummy::setDisplayMode(int display, int mode)
{
  if (!isValidDisplayMode(display, mode))
    return false;

  DMDisplayPtr displayptr = m_displays[display];
  DMVideoModePtr videomode = displayptr->m_videoModes[mode];

  qInfo() << "Switching to" << videomode->m_width << "x" << videomode->m_height << "@" << videomode->m_refreshRate;

  m_currentMode = videomode->m_id;

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerDummy::getCurrentDisplayMode(int display)
{
  if (!isValidDisplay(display))
    return -1;

  return m_currentMode;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerDummy::getMainDisplay()
{
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerDummy::getDisplayFromPoint(int x, int y)
{
  if (m_displays.size())
  {
    if (x >= 0 && y >= 0 && x < 1280 && y < 720)
      return 0;
  }
  return -1;
}
