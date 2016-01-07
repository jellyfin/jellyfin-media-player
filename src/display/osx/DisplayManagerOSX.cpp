//
//  DisplayManagerOSX.cpp
//  konvergo
//
//  Created by Lionel CHAZALLON on 28/09/2014.
//
//

#include <CoreGraphics/CoreGraphics.h>
#include "utils/osx/OSXUtils.h"
#include "DisplayManagerOSX.h"

#include "QsLog.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerOSX::initialize()
{
  int totalModes = 0;

  displays.clear();

  for (int i = 0; i < m_osxDisplayModes.size(); i++)
  {
    if (m_osxDisplayModes[i])
      CFRelease(m_osxDisplayModes[i]);
  }
  m_osxDisplayModes.clear();

  CGError err = CGGetActiveDisplayList(MAX_DISPLAYS, m_osxDisplays, &m_osxnumDisplays);
  if (err)
  {
    m_osxnumDisplays = 0;
    QLOG_ERROR() << "CGGetActiveDisplayList returned failure:" << err;
    return false;
  }

  for (int displayid = 0; displayid < m_osxnumDisplays; displayid++)
  {
    // add the display to the list
    DMDisplayPtr display = DMDisplayPtr(new DMDisplay);
    display->id = displayid;
    display->name = QString("Display %1").arg(displayid);
    displays[display->id] = display;

    m_osxDisplayModes[displayid] = CGDisplayCopyAllDisplayModes(m_osxDisplays[displayid], NULL);
    if (!m_osxDisplayModes[displayid])
      continue;

    int numModes = (int)CFArrayGetCount(m_osxDisplayModes[displayid]);

    for (int modeid = 0; modeid < numModes; modeid++)
    {
      totalModes++;
      
      // add the videomode to the display
      DMVideoModePtr mode = DMVideoModePtr(new DMVideoMode);
      mode->id = modeid;
      display->videoModes[modeid] = mode;

      // grab videomode info
      CGDisplayModeRef displayMode =
      (CGDisplayModeRef)CFArrayGetValueAtIndex(m_osxDisplayModes[displayid], modeid);

      mode->height = CGDisplayModeGetHeight(displayMode);
      mode->width = CGDisplayModeGetWidth(displayMode);
      mode->refreshRate = CGDisplayModeGetRefreshRate(displayMode);

      CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(displayMode);

      if (CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) ==
          kCFCompareEqualTo)
        mode->bitsPerPixel = 32;
      else if (CFStringCompare(pixEnc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) ==
               kCFCompareEqualTo)
        mode->bitsPerPixel = 16;
      else if (CFStringCompare(pixEnc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) ==
               kCFCompareEqualTo)
        mode->bitsPerPixel = 8;

      mode->interlaced = (CGDisplayModeGetIOFlags(displayMode) & kDisplayModeInterlacedFlag) > 0;

      if (mode->refreshRate == 0)
        mode->refreshRate = 60;
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
  (CGDisplayModeRef)CFArrayGetValueAtIndex(m_osxDisplayModes[display], mode);

  CGError err = CGDisplaySetDisplayMode(m_osxDisplays[display], displayMode, NULL);
  if (err)
  {
    QLOG_ERROR() << "CGDisplaySetDisplayMode() returned failure:" << err;
    return false;
  }

  // HACK : on OSX, switching display mode can leave dock in a state where mouse cursor
  // will not hide on top of hidden dock, so we reset it state to fix this
  OSXUtils::SetMenuBarVisible(true);
  OSXUtils::SetMenuBarVisible(false);

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerOSX::getCurrentDisplayMode(int display)
{
  if (!isValidDisplay(display) || !m_osxDisplayModes[display])
    return -1;
  
  CGDisplayModeRef currentMode = CGDisplayCopyDisplayMode(m_osxDisplays[display]);

  for (int mode = 0; mode < CFArrayGetCount(m_osxDisplayModes[display]); mode++)
  {
    if (currentMode == (CGDisplayModeRef)CFArrayGetValueAtIndex(m_osxDisplayModes[display], mode))
    {
      return mode;
    }
  }

  return -1;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerOSX::getMainDisplay()
{
  CGDirectDisplayID mainID = CGMainDisplayID();

  for (int i = 0; i < m_osxnumDisplays; i++)
  {
    if (m_osxDisplays[i] == mainID)
      return i;
  }

  return -1;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayManagerOSX::~DisplayManagerOSX()
{
  for (int i = 0; i < m_osxDisplayModes.size(); i++)
  {
    if (m_osxDisplayModes[i])
      CFRelease(m_osxDisplayModes[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerOSX::getDisplayFromPoint(int x, int y)
{
  CGPoint point = { (double)x, (double)y };
  CGDirectDisplayID foundDisplay;
  uint32_t numFound;

  CGGetDisplaysWithPoint(point, 1, &foundDisplay, &numFound);

  for (int i=0; i<m_osxnumDisplays; i++)
  {
    if (foundDisplay == m_osxDisplays[i])
      return i;
  }

  return -1;
}
