//
//  DisplayManagerOSX.cpp
//  konvergo
//
//  Created by Lionel CHAZALLON on 28/09/2014.
//
//

#include <CoreGraphics/CoreGraphics.h>
#include <QDebug>
#include "utils/osx/OSXUtils.h"
#include "DisplayManagerOSX.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerOSX::initialize()
{
  int totalModes = 0;

  m_displays.clear();

  for (size_t i = 0; i < m_osxDisplayModes.size(); i++)
  {
    if (m_osxDisplayModes[static_cast<int>(i)])
      CFRelease(m_osxDisplayModes[static_cast<int>(i)]);
  }
  m_osxDisplayModes.clear();

  CGError err = CGGetActiveDisplayList(MAX_DISPLAYS, m_osxDisplays, &m_osxnumDisplays);
  if (err)
  {
    m_osxnumDisplays = 0;
    qCritical() << "CGGetActiveDisplayList returned failure:" << err;
    return false;
  }

  for (uint32_t displayid = 0; displayid < m_osxnumDisplays; displayid++)
  {
    // add the display to the list
    DMDisplayPtr display = DMDisplayPtr(new DMDisplay);
    display->m_id = displayid;
    display->m_name = QString("Display %1").arg(displayid);
    m_displays[display->m_id] = display;

    m_osxDisplayModes[displayid] = CGDisplayCopyAllDisplayModes(m_osxDisplays[displayid], nullptr);
    if (!m_osxDisplayModes[displayid])
      continue;

    CFIndex numModes = CFArrayGetCount(m_osxDisplayModes[displayid]);

    for (CFIndex modeid = 0; modeid < numModes; modeid++)
    {
      totalModes++;

      // add the videomode to the display
      DMVideoModePtr mode = DMVideoModePtr(new DMVideoMode);
      mode->m_id = static_cast<int>(modeid);
      display->m_videoModes[static_cast<int>(modeid)] = mode;

      // grab videomode info
      CGDisplayModeRef displayMode =
      static_cast<CGDisplayModeRef>(const_cast<void*>(CFArrayGetValueAtIndex(m_osxDisplayModes[displayid], modeid)));

      mode->m_height = static_cast<int>(CGDisplayModeGetHeight(displayMode));
      mode->m_width = static_cast<int>(CGDisplayModeGetWidth(displayMode));
      mode->m_refreshRate = static_cast<float>(CGDisplayModeGetRefreshRate(displayMode));

      // Assume 32-bit color depth (CGDisplayModeCopyPixelEncoding deprecated in 10.11)
      mode->m_bitsPerPixel = 32;

      mode->m_interlaced = (CGDisplayModeGetIOFlags(displayMode) & kDisplayModeInterlacedFlag) > 0;

      if (mode->m_refreshRate == 0)
        mode->m_refreshRate = 60;
    }
  }

  if (totalModes == 0)
    return false;
  else
    return DisplayManager::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerOSX::setDisplayMode(int display, int mode)
{
  if (!isValidDisplayMode(display, mode) || !m_osxDisplayModes[display])
    return false;
  
  CGDisplayModeRef displayMode =
  static_cast<CGDisplayModeRef>(const_cast<void*>(CFArrayGetValueAtIndex(m_osxDisplayModes[display], mode)));

  CGError err = CGDisplaySetDisplayMode(m_osxDisplays[display], displayMode, nullptr);
  if (err)
  {
    qCritical() << "CGDisplaySetDisplayMode() returned failure:" << err;
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerOSX::getCurrentDisplayMode(int display)
{
  if (!isValidDisplay(display) || !m_osxDisplayModes[display])
    return -1;
  
  CGDisplayModeRef currentMode = CGDisplayCopyDisplayMode(m_osxDisplays[display]);
  uint32_t currentIOKitID = CGDisplayModeGetIODisplayModeID(currentMode);

  for (CFIndex mode = 0; mode < CFArrayGetCount(m_osxDisplayModes[display]); mode++)
  {
    CGDisplayModeRef checkMode = static_cast<CGDisplayModeRef>(const_cast<void*>(CFArrayGetValueAtIndex(m_osxDisplayModes[display], mode)));
    uint32_t checkIOKitID = CGDisplayModeGetIODisplayModeID(checkMode);

    if (currentIOKitID == checkIOKitID)
    {
      CFRelease(currentMode);
      return static_cast<int>(mode);
    }
  }
  CFRelease(currentMode);

  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerOSX::getMainDisplay()
{
  CGDirectDisplayID mainID = CGMainDisplayID();

  for (uint32_t i = 0; i < m_osxnumDisplays; i++)
  {
    if (m_osxDisplays[i] == mainID)
      return static_cast<int>(i);
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayManagerOSX::~DisplayManagerOSX()
{
  for (size_t i = 0; i < m_osxDisplayModes.size(); i++)
  {
    if (m_osxDisplayModes[static_cast<int>(i)])
      CFRelease(m_osxDisplayModes[static_cast<int>(i)]);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerOSX::getDisplayFromPoint(int x, int y)
{
  CGPoint point = { static_cast<CGFloat>(x), static_cast<CGFloat>(y) };
  CGDirectDisplayID foundDisplay;
  uint32_t numFound;

  CGGetDisplaysWithPoint(point, 1, &foundDisplay, &numFound);

  for (uint32_t i = 0; i < m_osxnumDisplays; i++)
  {
    if (foundDisplay == m_osxDisplays[i])
      return static_cast<int>(i);
  }

  return -1;
}
