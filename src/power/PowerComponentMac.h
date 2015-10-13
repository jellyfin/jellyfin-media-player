#ifndef POWERCOMPONENTMAC_H
#define POWERCOMPONENTMAC_H

#include "PowerComponent.h"
#include <IOKit/pwr_mgt/IOPMLib.h>

class PowerComponentMac : public PowerComponent
{
public:
  PowerComponentMac() : PowerComponent(0), m_assertion(0) { }
  virtual void doDisableScreensaver();
  virtual void doEnableScreensaver();

private:
  IOPMAssertionID m_assertion = 0;
};

#endif // POWERCOMPONENTMAC_H
