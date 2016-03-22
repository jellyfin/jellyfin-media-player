#ifndef POWERCOMPONENTMAC_H
#define POWERCOMPONENTMAC_H

#include "PowerComponent.h"
#include <IOKit/pwr_mgt/IOPMLib.h>

class PowerComponentMac : public PowerComponent
{
public:
  PowerComponentMac() : PowerComponent(nullptr), m_assertion(0) { }
  void doDisableScreensaver() override;
  void doEnableScreensaver() override;

private:
  IOPMAssertionID m_assertion = 0;
};

#endif // POWERCOMPONENTMAC_H
