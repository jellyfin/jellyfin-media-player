//
//  DisplayManagerWin.cpp
//  konvergo
//
//  Created by Lionel CHAZALLON on 18/06/2015.
//
//

#include <QRect>
#include <math.h>

#include "QsLog.h"
#include "DisplayManagerWin.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
static DMVideoMode convertDevMode(const DEVMODEW& devmode)
{
  DMVideoMode mode = {};
  mode.height = devmode.dmPelsHeight;
  mode.width = devmode.dmPelsWidth;
  mode.refreshRate = devmode.dmDisplayFrequency;
  mode.bitsPerPixel = devmode.dmBitsPerPel;
  mode.interlaced = !!(devmode.dmDisplayFlags & DM_INTERLACED);

  // Windows just returns integer refresh rate so let's fudge it
  if (mode.refreshRate == 59 ||
      mode.refreshRate == 29 ||
      mode.refreshRate == 23)
      mode.refreshRate = (float)(mode.refreshRate + 1) / 1.001f;

  return mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static bool modeEquals(const DMVideoMode& m1, const DMVideoMode& m2)
{
  return m1.height == m2.height &&
         m1.width == m2.width &&
         fabs(m1.refreshRate - m2.refreshRate) < 1e9 &&
         m1.bitsPerPixel == m2.bitsPerPixel &&
         m1.interlaced == m2.interlaced;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerWin::initialize()
{
  DISPLAY_DEVICEW displayInfo;
  int displayId = 0;

  m_displayAdapters.clear();
  displays.clear();

  while (getDisplayInfo(displayId, displayInfo))
  {
    if (displayInfo.StateFlags & (DISPLAY_DEVICE_ACTIVE | DISPLAY_DEVICE_ATTACHED))
    {
      DEVMODEW modeInfo;
      int modeId = 0;

      // add the display
      DMDisplayPtr display = DMDisplayPtr(new DMDisplay);
      display->id = displayId;
      display->name = QString::fromWCharArray(displayInfo.DeviceString);
      displays[display->id] = DMDisplayPtr(display);
      m_displayAdapters[display->id] = QString::fromWCharArray(displayInfo.DeviceName);

      while (getModeInfo(displayId, modeId, modeInfo))
      {
        // add the videomode to the display
        DMVideoModePtr videoMode = DMVideoModePtr(new DMVideoMode);
        *videoMode = convertDevMode(modeInfo);
        videoMode->id = modeId;
        display->videoModes[videoMode->id] = videoMode;

        modeId++;
      }
    }

    displayId++;
  }

  if (displays.size() == 0)
  {
    QLOG_DEBUG() << "No display found.";
    return false;
  }
  else
    return DisplayManager::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerWin::setDisplayMode(int display, int mode)
{
  DEVMODEW modeInfo;

  if (!isValidDisplayMode(display, mode))
    return false;

  if (getModeInfo(display, mode, modeInfo))
  {
    QLOG_DEBUG() << "Switching to mode" << mode << "on display" << display << ":" << displays[display]->videoModes[mode]->getPrettyName();

    modeInfo.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS;

    LONG rc = ChangeDisplaySettingsExW((LPCWSTR)m_displayAdapters[display].utf16(), &modeInfo, NULL,
                                       CDS_FULLSCREEN, NULL);

    if (rc != DISP_CHANGE_SUCCESSFUL)
    {
      QLOG_ERROR() << "Failed to changed DisplayMode, error" << rc;
      return false;
    }
    else
    {
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerWin::getCurrentDisplayMode(int display)
{
  if (!isValidDisplay(display))
    return -1;

  // grab current mode
  DEVMODEW modeInfo;
  ZeroMemory(&modeInfo, sizeof(modeInfo));
  modeInfo.dmSize = sizeof(modeInfo);

  // grab current mode info
  if (!EnumDisplaySettingsW((LPCWSTR)m_displayAdapters[display].utf16(), ENUM_CURRENT_SETTINGS,
                            &modeInfo))
  {
    QLOG_ERROR() << "Failed to retrieve current mode";
    return -1;
  }

  DMVideoMode mode = convertDevMode(modeInfo);

  // check if current mode info matches on of our modes
  for (int modeId = 0; modeId < displays[display]->videoModes.size(); modeId++)
  {
    if (modeEquals(mode, *displays[display]->videoModes[modeId]))
      return modeId;
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerWin::getMainDisplay()
{
  DISPLAY_DEVICEW displayInfo;
  int displayId = 0;

  while (getDisplayInfo(displayId, displayInfo))
  {
    if (displayInfo.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
      return displayId;

    displayId++;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayManagerWin::~DisplayManagerWin()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerWin::getDisplayFromPoint(int x, int y)
{
  foreach (int displayId, displays.keys())
  {
    QString dispName = m_displayAdapters[displayId];

    DEVMODEW modeInfo = {};
    modeInfo.dmSize = sizeof(modeInfo);

    QLOG_DEBUG() << "Looking at display" << displayId << dispName;

    if (!EnumDisplaySettingsW((LPCWSTR)dispName.utf16(), ENUM_CURRENT_SETTINGS,
                              &modeInfo))
    {
      QLOG_ERROR() << "Failed to retrieve current mode.";
    }
    else
    {
      QRect displayRect(modeInfo.dmPosition.x, modeInfo.dmPosition.y, modeInfo.dmPelsWidth,
                        modeInfo.dmPelsHeight);
      QLOG_DEBUG() << "Position on virtual desktop:" << displayRect;

      if (displayRect.contains(x, y))
        return displayId;
    }
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerWin::getDisplayInfo(int display, DISPLAY_DEVICEW& info)
{
  ZeroMemory(&info, sizeof(info));
  info.cb = sizeof(info);
  return EnumDisplayDevicesW(NULL, display, &info, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerWin::getModeInfo(int display, int mode, DEVMODEW& info)
{
  if (m_displayAdapters.contains(display))
  {
    ZeroMemory(&info, sizeof(info));
    info.dmSize = sizeof(info);

    return EnumDisplaySettingsExW((LPCWSTR)m_displayAdapters[display].utf16(), mode, &info, 0);
  }

  return false;
}
