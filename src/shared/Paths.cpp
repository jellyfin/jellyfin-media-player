//
// Created by Tobias Hieta on 01/09/15.
//

#include "settings/SettingsSection.h"
#include "Paths.h"

#include <QDir>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QtGui/qguiapplication.h>
#include <QDebug>
#include "Names.h"
#include "Version.h"

/////////////////////////////////////////////////////////////////////////////////////////
static QString g_configDirOverride;
static QString g_activeProfileId;

/////////////////////////////////////////////////////////////////////////////////////////
void Paths::setConfigDir(const QString& path)
{
  g_configDirOverride = path;
}

/////////////////////////////////////////////////////////////////////////////////////////
static QDir writableLocation(QStandardPaths::StandardLocation loc)
{
  QDir d(QStandardPaths::writableLocation(loc));
  if (!d.mkpath(d.absolutePath() + "/" + Names::DataName()))
  {
    qWarning() << "Failed to create directory:" << d.absolutePath();
    return QDir();
  }

  d.cd(Names::DataName());

  return d;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Try a couple of different strategies to find the file we are looking for.
// 1) By looking next to the application binary
// 2) By looking in binary/../Resources
// 3) By looking in PREFIX/share/jellyfin-desktop
// 4) By looking in PREFIX/jellyfin-desktop
//
QString Paths::resourceDir(const QString& file)
{
  auto appResourceDir = QGuiApplication::applicationDirPath() + "/";
  auto prefixDir = QString(PREFIX);

  QStringList possibleResourceDirs = {
    appResourceDir,
    appResourceDir + "../Resources/",
    prefixDir + "/share/jellyfin-desktop/",
    prefixDir + "/jellyfin-desktop/"
  };

  for (const auto& fileStr : possibleResourceDirs)
  {
    if (QFile::exists(fileStr + file))
      return fileStr + file;
  }

  return appResourceDir + file;
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::globalDataDir(const QString& file)
{
  QDir d;

  if (!g_configDirOverride.isEmpty())
  {
    d = QDir(g_configDirOverride);
    if (!d.mkpath(d.absolutePath()))
    {
      qWarning() << "Failed to create override directory:" << d.absolutePath();
      d = writableLocation(QStandardPaths::GenericDataLocation);
    }
  }
  else
  {
    d = writableLocation(QStandardPaths::GenericDataLocation);
  }

  if (file.isEmpty())
    return d.absolutePath();
  return d.filePath(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::globalCacheDir(const QString& file)
{
  QDir d = writableLocation(QStandardPaths::GenericCacheLocation);
  if (file.isEmpty())
    return d.absolutePath();
  return d.filePath(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::dataDir(const QString& file)
{
  // Use profile-specific directory when profile is active
  if (!g_activeProfileId.isEmpty())
  {
    QDir d(globalDataDir("profiles/" + g_activeProfileId));
    if (!d.mkpath(d.absolutePath()))
    {
      qWarning() << "Failed to create profile data directory:" << d.absolutePath();
      return globalDataDir(file);
    }

    if (file.isEmpty())
      return d.absolutePath();
    return d.filePath(file);
  }

  // Fallback to global when no profile active
  return globalDataDir(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::cacheDir(const QString& file)
{
  // Use profile-specific directory when profile is active
  if (!g_activeProfileId.isEmpty())
  {
    QDir d(globalCacheDir("profiles/" + g_activeProfileId));
    if (!d.mkpath(d.absolutePath()))
    {
      qWarning() << "Failed to create profile cache directory:" << d.absolutePath();
      return globalCacheDir(file);
    }

    if (file.isEmpty())
      return d.absolutePath();
    return d.filePath(file);
  }

  // Fallback to global when no profile active
  return globalCacheDir(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::logDir(const QString& file)
{
#ifdef Q_OS_MAC
  // On macOS, logs go to ~/Library/Logs/
  QDir ldir = QDir(QStandardPaths::locate(QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory));

  if (!g_activeProfileId.isEmpty())
  {
    ldir.mkpath(ldir.absolutePath() + "/Library/Logs/" + Names::DataName() + "/" + g_activeProfileId);
    ldir.cd("Library/Logs/" + Names::DataName() + "/" + g_activeProfileId);
  }
  else
  {
    ldir.mkpath(ldir.absolutePath() + "/Library/Logs/" + Names::DataName());
    ldir.cd("Library/Logs/" + Names::DataName());
  }

  return ldir.filePath(file);
#else
  // On other platforms, logs go to dataDir/logs/
  QDir ldir(dataDir());
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

  // Include profile ID in socket name for profile isolation
  QString profileSuffix;
  if (!g_activeProfileId.isEmpty())
    profileSuffix = "_" + g_activeProfileId.left(8);

#ifdef Q_OS_UNIX
  return QString("/tmp/jmp_%1_%2%3.sock").arg(serverName, userName, profileSuffix);
#else
  return QString("jmp_%1_%2%3.sock").arg(serverName, userName, profileSuffix);
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
    qWarning() << "Can't find sound:" << sound;
    return QString();
  }

  return f.absoluteFilePath();
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::webClientPath(const QString& mode)
{
  QString webName = QString("web-client/%1").arg(mode);
  return resourceDir(webName + "/index.html");
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::webExtensionPath(const QString& mode)
{
  QString webName = QString("web-client/%1").arg(mode);
  return resourceDir(webName + "/");
}

/////////////////////////////////////////////////////////////////////////////////////////
void Paths::setActiveProfileId(const QString& id)
{
  g_activeProfileId = id;
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Paths::activeProfileId()
{
  return g_activeProfileId;
}
