//
// Created by Tobias Hieta on 25/03/15.
//
#include "PowerComponent.h"
#include "input/InputComponent.h"
#include "settings/SettingsComponent.h"

#ifdef Q_OS_MAC
#include "PowerComponentMac.h"
#elif defined(LINUX_DBUS)
#include "PowerComponentDBus.h"
#elif defined(USE_X11POWER)
#include "PowerComponentX11.h"
#elif defined(Q_OS_WIN32)
#include "PowerComponentWin.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////
PowerComponent& PowerComponent::Get()
{
#ifdef Q_OS_MAC
  static PowerComponentMac instance;
  return instance;
#elif defined(LINUX_DBUS)
  static PowerComponentDBus instance;
  return instance;
#elif defined(USE_X11POWER)
  static PowerComponentX11 instance;
  return instance;
#elif defined(Q_OS_WIN32)
  static PowerComponentWin instance;
  return instance;
#else
  qWarning() << "Could not find a power component matching this platform. OS screensaver control disabled.";

  static PowerComponent instance;
  return instance;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponent::componentInitialize()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void PowerComponent::setScreensaverEnabled(bool enabled)
{
  if (enabled)
  {
    qDebug() << "Enabling OS screensaver";
    doEnableScreensaver();
  }
  else
  {
    qDebug() << "Disabling OS screensaver";
    doDisableScreensaver();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void PowerComponent::componentPostInitialize()
{
  InputComponent::Get().registerHostCommand("poweroff", this, "PowerOff");
  InputComponent::Get().registerHostCommand("reboot", this, "Reboot");
  InputComponent::Get().registerHostCommand("suspend", this, "Suspend");
}

/////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponent::checkCap(PowerCapabilities capability)
{
  if (!SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "showPowerOptions").toBool())
    return false;

  return (getPowerCapabilities() & capability);
}


