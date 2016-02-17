//
// Created by msn on 2016-02-16.
//

#include "UpdateManagerWin32.h"
#include "Paths.h"

#include "QsLog.h"

#include <QFile>
#include <QDir>
#include <QProcess>

/////////////////////////////////////////////////////////////////////////////////////////
bool UpdateManagerWin32::applyUpdate(const QString& version)
{
  QString updateExe = GetPath("PlexMediaPlayer-" + version + "-windows-x64.exe", version, true);
  if(QFile::exists(updateExe))
  {
    QStringList args;
    args << "/passive" << "/norestart" << "/log" << Paths::logDir("Plex Media Player Installer.log");
    args << "autolaunch=1";

    QFile::remove(GetPath("_readyToApply", version, false));
    if (QProcess::startDetached(updateExe, args, QDir::temp().absolutePath()))
    {
      QLOG_DEBUG() << "Running update...";
      return true;
    }
  }
  else
  {
    QLOG_WARN() << "Failed to find update file:" << updateExe;
  }

  return false;
}
