#ifndef POWERCOMPONENTDBUS_H
#define POWERCOMPONENTDBUS_H

#include "PowerComponent.h"

class PowerComponentDBus : public PowerComponent
{
  public:
    PowerComponentDBus() : PowerComponent(0) {};
    ~PowerComponentDBus() {};

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

    unsigned int screensaver_inhibit_cookie = 0;
  
  protected:
    virtual void doDisableScreensaver();
    virtual void doEnableScreensaver();
};

#endif // POWERCOMPONENTDBUS_H
