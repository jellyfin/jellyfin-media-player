//
//  DisplayManagerWin.cpp
//  konvergo
//
//  Created by Lionel CHAZALLON on 18/06/2015.
//
//

#include <QRect>

#include "QsLog.h"
#include "DisplayManagerWin.h"

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
        videoMode->id = modeId;
        display->videoModes[videoMode->id] = videoMode;

        // setup mode information
        videoMode->height = modeInfo.dmPelsHeight;
        videoMode->width = modeInfo.dmPelsWidth;
        videoMode->refreshRate = modeInfo.dmDisplayFrequency;
        videoMode->bitsPerPixel = modeInfo.dmBitsPerPel;
        videoMode->interlaced = (modeInfo.dmDisplayFlags & DM_INTERLACED) ? true : false;

        // Windows just returns interger refresh rate so
        // let's fudge it
        if (videoMode->refreshRate == 59 ||
            videoMode->refreshRate == 29 ||
            videoMode->refreshRate == 23)
           videoMode->refreshRate = (float)(videoMode->refreshRate + 1) / 1.001f;

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

  // check if current mode info matches on of our modes
  for (int modeId = 0; modeId < displays[display]->videoModes.size(); modeId++)
  {
    if (isModeMatching(modeInfo, displays[display]->videoModes[modeId]))
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
    int currentMode = getCurrentDisplayMode(displayId);
    if (currentMode >= 0)
    {
      DEVMODEW modeInfo;
      if (getModeInfo(displayId, currentMode, modeInfo))
      {
        QRect displayRect(modeInfo.dmPosition.x, modeInfo.dmPosition.y, modeInfo.dmPelsWidth,
                          modeInfo.dmPelsHeight);
        QLOG_DEBUG() << "Looking at display" << displayId << "mode" << currentMode
                     << "at" << displayRect;

        if (displayRect.contains(x, y))
          return displayId;
      }
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
  DISPLAY_DEVICEW displayInfo;

  if (m_displayAdapters.contains(display))
  {
    ZeroMemory(&info, sizeof(info));
    info.dmSize = sizeof(info);

    return EnumDisplaySettingsExW((LPCWSTR)m_displayAdapters[display].utf16(), mode, &info, 0);
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerWin::isModeMatching(DEVMODEW& modeInfo, DMVideoModePtr videoMode)
{
  if (videoMode->height != modeInfo.dmPelsHeight)
    return false;
  if (videoMode->width != modeInfo.dmPelsWidth)
    return false;
  if ((int)(videoMode->refreshRate + 0.5f) != modeInfo.dmDisplayFrequency)
    return false;
  if (videoMode->bitsPerPixel != modeInfo.dmBitsPerPel)
    return false;
  if (videoMode->interlaced != ((modeInfo.dmDisplayFlags & DM_INTERLACED) ? true : false))
    return false;

  return true;
}
