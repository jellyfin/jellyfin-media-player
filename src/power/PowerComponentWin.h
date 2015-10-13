#ifndef POWERCOMPONENTWIN_H
#define POWERCOMPONENTWIN_H

#include "PowerComponent.h"

class PowerComponentWin : public PowerComponent
{
public:
  PowerComponentWin() : PowerComponent(0) { }
  virtual void doDisableScreensaver();
  virtual void doEnableScreensaver();
};

#endif // POWERCOMPONENTWIN_H
