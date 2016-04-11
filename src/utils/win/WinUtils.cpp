#include "WinUtils.h"
#include "windows.h"

/////////////////////////////////////////////////////////////////////////////////////////
bool WinUtils::getPowerManagementPrivileges()
{
  HANDLE hToken = nullptr;
  // Get a token for this process.
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
  {
    // Get the LUID for the shutdown privilege.
    TOKEN_PRIVILEGES tkp = {};
    if (LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
    {
      tkp.PrivilegeCount = 1;  // one privilege to set
      tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

      // Get the shutdown privilege for this process.
      if (AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
      {
        CloseHandle(hToken);
        return true;
      }
    }
  }

  if (hToken)
    CloseHandle(hToken);
  return false;
}
