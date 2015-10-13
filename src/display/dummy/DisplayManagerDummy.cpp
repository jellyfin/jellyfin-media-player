#include "QsLog.h"
#include "DisplayManagerDummy.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayManagerDummy::addMode(float rate)
{
  if (!displays.size())
    return;

  DMVideoModePtr mode = DMVideoModePtr(new DMVideoMode());
  mode->id = displays[0]->videoModes.size();
  mode->interlaced = false;
  mode->refreshRate = rate;
  mode->width = 1280;
  mode->height = 720;
  mode->bitsPerPixel = 0;
  displays[0]->videoModes[mode->id] = mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerDummy::initialize()
{
  displays.clear();

  DMDisplayPtr display = DMDisplayPtr(new DMDisplay());
  display->id = displays.size();
  display->name = "Dummy display";
  displays[display->id] = display;

  addMode(60);

  return DisplayManager::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerDummy::setDisplayMode(int display, int mode)
{
  if (!isValidDisplayMode(display, mode))
    return false;

  DMDisplayPtr displayptr = displays[display];
  DMVideoModePtr videomode = displayptr->videoModes[mode];

  QLOG_INFO() << "Switching to" << videomode->width << "x" << videomode->height << "@" << videomode->refreshRate;

  m_currentMode = videomode->id;

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
  if (displays.size())
  {
    if (x >= 0 && y >= 0 && x < 1280 && y < 720)
      return 0;
  }
  return -1;
}
