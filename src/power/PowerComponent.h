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
  : ComponentBase(parent),
    m_currentScreensaverEnabled(true),
    m_fullscreenState(false),
    m_videoPlaying(false)
    { }

  bool componentInitialize() override;
  bool componentExport() override { return true; }
  const char* componentName() override { return "power"; }
  void componentPostInitialize() override;

  void setFullscreenState(bool fullscreen);

public Q_SLOTS:
  bool checkCap(PowerCapabilities capability);

  bool canPowerOff() { return checkCap(CAP_POWER_OFF); }
  bool canReboot() { return checkCap(CAP_REBOOT); }
  bool canSuspend() { return checkCap(CAP_SUSPEND); }
  bool canRelaunch() { return checkCap(CAP_RELAUNCH); }

  virtual int getPowerCapabilities() { return CAP_RELAUNCH; }

  virtual bool PowerOff() { return false; }
  virtual bool Reboot() { return false; }
  virtual bool Suspend() { return false; }

private Q_SLOTS:
  void playbackActive(bool active);

Q_SIGNALS:
  void screenSaverEnabled();
  void screenSaverDisabled();

protected:
  virtual void doDisableScreensaver() {};
  virtual void doEnableScreensaver() {};

private:
  void redecideScreeensaverState();

  bool m_currentScreensaverEnabled;
  bool m_fullscreenState;
  bool m_videoPlaying;
};


#endif // POWERMANAGER

