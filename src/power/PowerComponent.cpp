//
// Created by Tobias Hieta on 25/03/15.
//

#include "PowerComponent.h"
#include "player/PlayerComponent.h"
#include "input/InputComponent.h"

#ifdef Q_OS_MAC
#include "PowerComponentMac.h"
#elif KONVERGO_OPENELEC
#include "PowerComponentOE.h"
#elif USE_X11POWER
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
#elif KONVERGO_OPENELEC
  static PowerComponentOE instance;
  return instance;
#elif USE_X11POWER
  static PowerComponentX11 instance;
  return instance;
#elif defined(Q_OS_WIN32)
  static PowerComponentWin instance;
  return instance;
#else
  QLOG_WARN() << "Could not find a power component matching this platform. OS screensaver control disabled.";

  static PowerComponent instance;
  return instance;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponent::componentInitialize()
{
  PlayerComponent* player = &PlayerComponent::Get();

  connect(player, &PlayerComponent::playing, this, &PowerComponent::playbackStarted);
  connect(player, &PlayerComponent::playbackEnded, this, &PowerComponent::playbackEnded);

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void PowerComponent::setFullscreenState(bool fullscreen)
{
  m_fullscreenState = fullscreen;
  redecideScreeensaverState();
}

/////////////////////////////////////////////////////////////////////////////////////////
void PowerComponent::redecideScreeensaverState()
{
  bool enableOsScreensaver = !m_fullscreenState && !m_videoPlaying;
  if (m_currentScreensaverEnabled != enableOsScreensaver)
  {
    m_currentScreensaverEnabled = enableOsScreensaver;
    if (enableOsScreensaver)
    {
      QLOG_DEBUG() << "Enabling OS screensaver";
      doEnableScreensaver();
      emit screenSaverEnabled();
    }
    else
    {
      QLOG_DEBUG() << "Disabling OS screensaver";
      doDisableScreensaver();
      emit screenSaverDisabled();
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void PowerComponent::playbackStarted()
{
  m_videoPlaying = true;
  redecideScreeensaverState();
}

/////////////////////////////////////////////////////////////////////////////////////////
void PowerComponent::playbackEnded()
{
  m_videoPlaying = false;
  redecideScreeensaverState();
}

/////////////////////////////////////////////////////////////////////////////////////////
void PowerComponent::componentPostInitialize()
{
  InputComponent::Get().registerHostCommand("poweroff", this, "PowerOff");
  InputComponent::Get().registerHostCommand("reboot", this, "Reboot");
  InputComponent::Get().registerHostCommand("suspend", this, "Suspend");
}
