#include <windows.h>

#include "PowerComponentWin.h"
#include "powrprof.h"
#include "utils/Utils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
PowerComponentWin::PowerComponentWin() :  PowerComponent(nullptr), m_hasPrivileges(false)
{
  m_hasPrivileges = WinUtils::getPowerManagementPrivileges();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentWin::doDisableScreensaver()
{
  SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentWin::doEnableScreensaver()
{
  SetThreadExecutionState(ES_CONTINUOUS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentWin::Suspend()
{
  return SetSuspendState(false, true, false) == TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentWin::Reboot()
{
  return InitiateShutdownW(NULL, NULL, 0, SHUTDOWN_INSTALL_UPDATES | SHUTDOWN_RESTART,
                           SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED) == ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentWin::PowerOff()
{
  DWORD shutdownFlags = SHUTDOWN_INSTALL_UPDATES | SHUTDOWN_POWEROFF;

  if (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8)
    shutdownFlags |= SHUTDOWN_HYBRID;

  return InitiateShutdownW(NULL, NULL, 0, shutdownFlags,
                           SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED) == ERROR_SUCCESS;

}
