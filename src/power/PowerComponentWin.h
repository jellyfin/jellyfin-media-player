#ifndef POWERCOMPONENTWIN_H
#define POWERCOMPONENTWIN_H

#include "PowerComponent.h"


class PowerComponentWin : public PowerComponent
{
public:
  PowerComponentWin();
  virtual void doDisableScreensaver();
  virtual void doEnableScreensaver();

  virtual bool canPowerOff() override { return m_hasPrivileges; }
  virtual bool canReboot() override { return m_hasPrivileges; }
  virtual bool canSuspend() override { return true; }

  virtual bool PowerOff() override;
  virtual bool Reboot() override;
  virtual bool Suspend() override;

private:
  bool m_hasPrivileges;

};

#endif // POWERCOMPONENTWIN_H
