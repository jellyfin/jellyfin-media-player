#ifndef POWERCOMPONENTWIN_H
#define POWERCOMPONENTWIN_H

#include "PowerComponent.h"


class PowerComponentWin : public PowerComponent
{
public:
  PowerComponentWin();
  virtual void doDisableScreensaver();
  virtual void doEnableScreensaver();

  virtual int getPowerCapabilities() override
  {
    int flags = CAP_SUSPEND;
    if (m_hasPrivileges)
      flags |= CAP_POWER_OFF | CAP_REBOOT;
    return flags;
  }

  virtual bool PowerOff() override;
  virtual bool Reboot() override;
  virtual bool Suspend() override;

private:
  bool m_hasPrivileges;

};

#endif // POWERCOMPONENTWIN_H
