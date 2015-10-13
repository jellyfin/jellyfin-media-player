#ifndef POWERCOMPONENTOE_H
#define POWERCOMPONENTOE_H

#include "PowerComponent.h"

class PowerComponentOE : public PowerComponent
{
  public:
    PowerComponentOE() : PowerComponent(0) {};
    ~PowerComponentOE() {};

  public Q_SLOTS:
    virtual bool canPowerOff() { return isPowerMethodAvailable("CanPowerOff"); }
    virtual bool canReboot() { return isPowerMethodAvailable("CanReboot"); }
    virtual bool canSuspend() { return isPowerMethodAvailable("CanSuspend"); }
    virtual bool canRelaunch() { return true; }

    virtual bool PowerOff() { return callPowerMethod("PowerOff"); }
    virtual bool Reboot() { return callPowerMethod("Reboot"); }
    virtual bool Suspend() { return callPowerMethod("Suspend"); }

  private:
    bool callPowerMethod(QString method);
    bool isPowerMethodAvailable(QString method);
};

#endif // POWERCOMPONENTOE_H
