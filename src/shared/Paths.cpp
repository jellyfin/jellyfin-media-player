//
// Created by Tobias Hieta on 01/09/15.
//

#include "settings/SettingsSection.h"
#include "Paths.h"

#include <QDir>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QsLog.h>
#include <QtGui/qguiapplication.h>
#include "Names.h"
#include "Version.h"

/////////////////////////////////////////////////////////////////////////////////////////
static QDir writableLocation(QStandardPaths::StandardLocation loc)
{
  QDir d(QStandardPaths::writableLocation(loc));
  if (!d.mkpath(d.absolutePath() + "/" + Names::MainName()))
  {
    QLOG_WARN() << "Failed to create directory:" << d.absolutePath();
    return QDir();
  }

  d.cd(Names::MainName());

  return d;
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::resourceDir(const QString& file)
{
  auto resourceDir = QDir(QGuiApplication::applicationDirPath());

#ifdef Q_OS_MAC
  resourceDir.cdUp();
  resourceDir.cd("Resources");
#endif

  return resourceDir.filePath(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::dataDir(const QString& file)
{
  QDir d = writableLocation(QStandardPaths::GenericDataLocation);
  if (file.isEmpty())
    return d.absolutePath();
  return d.filePath(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::cacheDir(const QString& file)
{
  QDir d = writableLocation(QStandardPaths::GenericCacheLocation);
  if (file.isEmpty())
    return d.absolutePath();
  return d.filePath(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::logDir(const QString& file)
{
#ifdef Q_OS_MAC
  QDir ldir = QDir(QStandardPaths::locate(QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory));
  ldir.mkpath(ldir.absolutePath() + "/Library/Logs/" + Names::MainName());
  ldir.cd("Library/Logs/" + Names::MainName());
  return ldir.filePath(file);
#else
  QDir ldir = writableLocation(QStandardPaths::GenericDataLocation);
  ldir.mkpath(ldir.absolutePath() + "/logs");
  ldir.cd("logs");
  return ldir.filePath(file);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::socketName(const QString& serverName)
{
  QString userName = qgetenv("USER");

  if(userName.isEmpty())
    userName = qgetenv("USERNAME");
  if(userName.isEmpty())
    userName = "unknown";

#ifdef Q_OS_UNIX
  return QString("/tmp/pmp_%1_%2.sock").arg(serverName).arg(userName);
#else
  return QString("pmp_%1_%2.sock").arg(serverName).arg(userName);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::soundsPath(const QString& sound)
{
  // check local filesystem first
  auto localSound = dataDir("sounds/" + sound);

  QFileInfo f(localSound);
  if (f.exists())
    return f.absoluteFilePath();

  f = QFileInfo(":/sounds/" + sound);
  if (!f.exists())
  {
    QLOG_WARN() << "Can't find sound:" << sound;
    return QString();
  }

  return f.absoluteFilePath();
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::webClientPath()
{
  QString webName = QString("web-client-%1").arg(Version::GetWebVersion());
  return resourceDir(webName + "/index.html");
}
