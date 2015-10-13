#include <QProcess>
#include <QDir>
#include "QsLog.h"
#include "OEUpdateManager.h"
#include "SystemComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
QString OEUpdateManager::HaveUpdate()
{
  return "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OEUpdateManager::applyUpdate(const QString& version)
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OEUpdateManager::doUpdate(const QString& version)
{
  // grab the update file
  QString packagePath = GetPath("", version, true);
  QDir packageDir(packagePath);

  QStringList updateFiles = packageDir.entryList(QStringList( "*.tar"), QDir::Files, QDir::Time);

  if (updateFiles.size())
  {
    // copy the update files to /storage/.update
    QString destUpdatePath = "/storage/.update/" + updateFiles.at(0);
    if (packageDir.rename(packagePath + updateFiles.at(0), destUpdatePath))
    {
      if (isMiniUpdateArchive(destUpdatePath))
      {
        // if we have a miniupdate, just exit
        QLOG_DEBUG() << "Exiting to apply application update " << destUpdatePath;
        SystemComponent::Get().exit();
      }
      else
      {
        // now reboot to do the update
        QLOG_DEBUG() << "Rebooting to apply system update " << destUpdatePath;
        QProcess::startDetached("reboot");
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OEUpdateManager::isMiniUpdateArchive(QString archivePath)
{
  QProcess process;

  process.start("/bin/tar", QStringList() << "-tf" << archivePath);
  if (process.waitForFinished(1000) && (process.exitCode() == 0))
  {
    QByteArray output = process.readAllStandardOutput();
    return output.contains(QByteArray("bin/") + Names::MainName().toUtf8());
  }
  else
  {
    QLOG_ERROR() << "Unable to list update archive files : " << QString(process.readAllStandardError());
  }

  return false;
}
