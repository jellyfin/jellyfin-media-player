#ifndef POWERCOMPONENTOE_H
#define POWERCOMPONENTOE_H

#include "PowerComponent.h"

class PowerComponentOE : public PowerComponent
{
  public:
    PowerComponentOE() : PowerComponent(0) {};
    ~PowerComponentOE() {};

  public Q_SLOTS:

  virtual int getPowerCapabilities() override
  {
    int flags = 0;
    if (isPowerMethodAvailable("CanPowerOff"))
      flags |= CAP_POWER_OFF;
    if (isPowerMethodAvailable("CanReboot"))
      flags |= CAP_REBOOT;
    if (isPowerMethodAvailable("CanSuspend"))
      flags |= CAP_SUSPEND;
    return flags;
  }

    virtual bool PowerOff() { return callPowerMethod("PowerOff"); }
    virtual bool Reboot() { return callPowerMethod("Reboot"); }
    virtual bool Suspend() { return callPowerMethod("Suspend"); }

  private:
    bool callPowerMethod(QString method);
    bool isPowerMethodAvailable(QString method);
};

#endif // POWERCOMPONENTOE_H
