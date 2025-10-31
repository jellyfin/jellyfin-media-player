//
// Created by Tobias Hieta on 15/05/15.
//

#include "PlatformUtils.h"

#ifdef Q_OS_UNIX
#include <signal.h>
#endif

bool PlatformUtils::isProcessAlive(qint64 pid)
{
#ifdef Q_OS_UNIX
  int ret = kill((pid_t)pid, 0);
  return ret == 0;
#endif

  return false;
}
