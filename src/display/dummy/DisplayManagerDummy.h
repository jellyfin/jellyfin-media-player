#ifndef DISPLAYMANAGERDUMMY_H_
#define DISPLAYMANAGERDUMMY_H_

#include "display/DisplayManager.h"

class DisplayManagerDummy : public DisplayManager
{
  Q_OBJECT
private:

  void addMode(float rate);

  int m_currentMode;

public:
  DisplayManagerDummy(QObject* parent) : DisplayManager(parent), m_currentMode(0) {};

  virtual bool initialize();
  virtual bool setDisplayMode(int display, int mode);
  virtual int getCurrentDisplayMode(int display);
  virtual int getMainDisplay();
  virtual int getDisplayFromPoint(int x, int y);
};

#endif /* DISPLAYMANAGERX11_H_ */
