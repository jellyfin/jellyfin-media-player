#ifndef DISPLAYMANAGERWIN_H
#define DISPLAYMANAGERWIN_H

#include <windows.h>
#include "display/DisplayManager.h"
///////////////////////////////////////////////////////////////////////////////////////////////////
// Displays

class DisplayManagerWin : public DisplayManager
{
  Q_OBJECT
private:
  bool getDisplayInfo(int display, DISPLAY_DEVICEW& info);
  bool getModeInfo(int display, int mode, DEVMODEW& info);
  bool isModeMatching(DEVMODEW& modeInfo, DMVideoModePtr videoMode);

  QMap<int, QString> m_displayAdapters;

public:
  DisplayManagerWin(QObject* parent) : DisplayManager(parent) {}
  virtual ~DisplayManagerWin();

  virtual bool initialize();
  virtual bool setDisplayMode(int display, int mode);
  virtual int getCurrentDisplayMode(int display);
  virtual int getMainDisplay();
  virtual int getDisplayFromPoint(int x, int y);
};

#endif // DISPLAYMANAGERWIN_H
