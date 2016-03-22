#ifndef POWERMANAGER
#define POWERMANAGER

#include <QsLog.h>
#include "ComponentManager.h"

class PowerComponent : public ComponentBase
{
  Q_OBJECT
public:
  static PowerComponent& Get();

  PowerComponent(QObject* parent = nullptr)
  : ComponentBase(parent),
    m_currentScreensaverEnabled(true),
    m_fullscreenState(false),
    m_videoPlaying(false)
    { }

  virtual bool componentInitialize();
  virtual bool componentExport() { return true; }
  virtual const char* componentName() { return "power"; }
  virtual void componentPostInitialize();

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

