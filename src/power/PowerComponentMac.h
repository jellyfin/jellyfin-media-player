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

  virtual int getPowerCapabilities() override
  {
    int flags = IOPMSleepEnabled() ? CAP_SUSPEND : 0;
    return flags | CAP_POWER_OFF | CAP_REBOOT;
  }

  virtual bool PowerOff() override;
  virtual bool Reboot() override;
  virtual bool Suspend() override;

private:
  IOPMAssertionID m_assertion = 0;
};

#endif // POWERCOMPONENTMAC_H
