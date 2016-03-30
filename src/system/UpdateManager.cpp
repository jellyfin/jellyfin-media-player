#include "QsLog.h"

#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QProcess>
#include "UpdateManager.h"
#include "utils/Utils.h"
#include "utils/HelperLauncher.h"
#include "system/SystemComponent.h"

#ifdef KONVERGO_OPENELEC
#include "OEUpdateManager.h"
#endif

#ifdef Q_OS_WIN
#include "UpdateManagerWin32.h"
#endif

UpdateManager* g_updateManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
UpdateManager* UpdateManager::Get()
{
  if (!g_updateManager)
  {
#if defined(KONVERGO_OPENELEC)
    g_updateManager = new OEUpdateManager(nullptr);
#elif defined(Q_OS_WIN)
    g_updateManager = new UpdateManagerWin32(nullptr);
#else
    g_updateManager = new UpdateManager(nullptr);
#endif
  }

  return g_updateManager;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString UpdateManager::HaveUpdate()
{
  QDir updateDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/updates/");
  if (!updateDir.exists())
  {
    QLOG_DEBUG() << "No Update directory found, exiting";
    return "";
  }
  QStringList nonAppliedUpdates;

  // sort update directories, sort by the newest directory first, that way
  // we apply the latest update downloaded.
  //
  for(const QString& dir : updateDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Time))
  {
    // check if this version has been applied
    QString readyFile(GetPath("_readyToApply", dir, false));
    QString packagesDir(GetPath("packages", dir, false));

    QLOG_DEBUG() << "Checking for:" << readyFile;

    QDir packageDir(GetPath("packages", dir, false));

    if (QFile::exists(readyFile))
    {
      QLOG_DEBUG() << dir << "is not applied";
      return dir;
    }
    else if (packageDir.exists())
    {
      QLOG_DEBUG() << "Removing old update packages in dir:" << dir;
      if (!packageDir.removeRecursively())
      {
        QLOG_WARN() << "Failed to remove old update packages in dir:" << dir;
      }
    }
  }

  QLOG_DEBUG() << "No valid/applicable update found.";
  return "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool UpdateManager::applyUpdate(const QString& version)
{
  // make sure we have a manifest for this version.
  QLOG_DEBUG() << "Applying Update to version" << version;

  QString manifestPath = GetPath("manifest.xml.bz2", version, false);
  QString packagePath = QFileInfo(manifestPath).dir().absolutePath() + "/packages";

  if (!QFile(manifestPath).exists() || !QDir(packagePath).exists())
  {
    QLOG_ERROR() << "Could not find:" << manifestPath << "or" << packagePath;
    return false;
  }

  QString updaterName("updater");
#ifdef Q_OS_WIN
  updaterName = "updater.exe";
#endif

  QFile updaterFile(Paths::resourceDir(updaterName));

  if (!updaterFile.exists())
  {
    QLOG_ERROR() << "Missing updater:" << updaterFile.fileName();
    return false;
  }

  // copy the updater to a temporary directory so that we don't overwrite it.
  QString updaterPath = QDir::temp().absoluteFilePath(updaterName);

  // we need to remove it first, since QFile::copy() will fail otherwise.
  if (QFile::exists(updaterPath))
  {
    if (!QFile::remove(updaterPath))
    {
      QLOG_DEBUG() << "Failed to remove updater from " << updaterPath;
    }
  }

  if (!QFile::copy(updaterFile.fileName(), updaterPath))
  {
    QLOG_ERROR() << "Failed to copy the updater to:" << updaterPath;
    return false;
  }

  QStringList args;
  args << "--script=" + manifestPath;
  args << "--package-dir=" + packagePath;
  args << "--auto-close";

#ifdef Q_OS_MAC
  args << "--install-dir=" + QDir(QCoreApplication::applicationDirPath() + "/../../").absolutePath();
#else
  args << "--install-dir=" + QDir(QCoreApplication::applicationDirPath()).absolutePath();
#endif

  args << "--wait=" + QString::number(QCoreApplication::applicationPid());

  // beyond this point we don't want to try to install this update again.
  QFile::remove(GetPath("_readyToApply", version, false));

  QLOG_DEBUG() << "Executing:" << updaterPath << args;
  auto  process = new QProcess(nullptr);
  if (process->startDetached(updaterPath, args, QDir::temp().absolutePath()))
  {
    QLOG_DEBUG() << "Updater running, shutting down Plex Media Player";

    return true;
  }

  QLOG_ERROR() << "Failed to execute the updater";
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString UpdateManager::GetPath(const QString& file, const QString& version, bool package)
{
  QString filePath(file);

  // put the packages in the packages subdir.
  if (package)
    filePath = "packages/" + file;

  return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/updates/" + version + "/" + filePath;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void UpdateManager::doUpdate(const QString& version)
{
  QLOG_DEBUG() << "Update competed, restarting system";
  if (!applyUpdate(version))
  {
    QLOG_ERROR() "Failed to apply update";
  }
  else
  {
    QLOG_DEBUG() "Update was applied successfully";
  }

  HelperLauncher::Get().stop();
  SystemComponent::Get().exit();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool UpdateManager::CheckForUpdates()
{
  QString updateVersion = Get()->HaveUpdate();
  if (!updateVersion.isEmpty())
  {
    QLOG_DEBUG() << "We want to apply update:" << updateVersion;

    // if this call succeeds we need to shut down and let
    // the updater do it's work. Otherwise we'll just
    // start the application as normal and pretend that
    // nothing happened
    //
    if (Get()->applyUpdate(updateVersion))
      return true;
  }
  return false;
}
