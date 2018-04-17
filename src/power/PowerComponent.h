#ifndef POWERMANAGER
#define POWERMANAGER

#include <QsLog.h>
#include "ComponentManager.h"

class PowerComponent : public ComponentBase
{
  Q_OBJECT
public:

  enum PowerCapabilities
  {
    CAP_POWER_OFF = 0x01,
    CAP_REBOOT = 0x02,
    CAP_SUSPEND = 0x04,
    CAP_RELAUNCH = 0x08
  };

  static PowerComponent& Get();

  explicit PowerComponent(QObject* parent = nullptr)
  : ComponentBase(parent)
    { }

  bool componentInitialize() override;
  bool componentExport() override { return true; }
  const char* componentName() override { return "power"; }
  void componentPostInitialize() override;

public Q_SLOTS:
  bool checkCap(PowerCapabilities capability);

  bool canPowerOff() { return checkCap(CAP_POWER_OFF); }
  bool canReboot() { return checkCap(CAP_REBOOT); }
  bool canSuspend() { return checkCap(CAP_SUSPEND); }
  bool canRelaunch()
  {
#if OPENELEC
      return true;
#else
      return false;
#endif
  }

  virtual int getPowerCapabilities() { return 0; }

  virtual bool PowerOff() { return false; }
  virtual bool Reboot() { return false; }
  virtual bool Suspend() { return false; }

  void setScreensaverEnabled(bool enabled);

Q_SIGNALS:
  // Short-term compatibility with old web-client. Does nothing.
  void screenSaverEnabled();
  void screenSaverDisabled();

protected:
  virtual void doDisableScreensaver() {};
  virtual void doEnableScreensaver() {};
};


#endif // POWERMANAGER

