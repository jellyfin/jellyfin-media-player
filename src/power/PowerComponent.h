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
    CAP_POWER_OFF,
    CAP_REBOOT,
    CAP_SUSPEND,
    CAP_RELAUNCH
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
  virtual bool canPowerOff() { return false; }
  virtual bool canReboot() { return false; }
  virtual bool canSuspend() { return false; }
  virtual bool canRelaunch() { return false; }

  virtual bool PowerOff() { return false; }
  virtual bool Reboot() { return false; }
  virtual bool Suspend() { return false; }

private Q_SLOTS:
  void playbackStarted();
  void playbackEnded();

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

