//
// Created by Tobias Hieta on 18/09/15.
//

#include <qstandardpaths.h>
#include <qdir.h>

#include "HelperLaunchd.h"

#include "QsLog.h"
#include "plistparser.h"
#include "plistserializer.h"
#include "HelperLauncher.h"

#define LAUNCHCTL_PATH "/bin/launchctl"

/////////////////////////////////////////////////////////////////////////////////////////
HelperLaunchd::HelperLaunchd(QObject* parent) : QObject(parent)
{
  m_launchctl = new QProcess(this);

  connect(m_launchctl, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), [=](QProcess::ProcessError error){
    QLOG_ERROR() << "When trying to execute launchctl:" << m_launchctl->errorString();
  });

  connect(m_launchctl, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus status){
    if (status == QProcess::NormalExit)
    {
      QLOG_INFO() << "Ran launchctl successfully";
    }
    else
    {
      QLOG_ERROR() << "Failed to run launchctl:" << m_launchctl->errorString();
    }
  });
}

/////////////////////////////////////////////////////////////////////////////////////////
QString HelperLaunchd::launchPlistPath()
{
  QDir homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
  if (homeDir.cd("Library") && homeDir.cd("LaunchAgents"))
    return homeDir.filePath("tv.plex.player-helper.plist");
  return "";
}

/////////////////////////////////////////////////////////////////////////////////////////
bool HelperLaunchd::checkHelperPath()
{
  QString plistFile = launchPlistPath();
  if (!plistFile.isEmpty())
  {
    QFile fp(plistFile);
    if (fp.open(QIODevice::ReadOnly))
    {
      PListParser parser;
      QVariantMap plistValues = parser.parsePList(&fp).toMap();

      return plistValues.value("Program").toString() == HelperLauncher::HelperPath();
    }
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool HelperLaunchd::writePlist()
{
  QVariantMap launchPlist;

  launchPlist.insert("Label", "tv.plex.player");
  launchPlist.insert("RunAtLoad", true);

  QVariantMap keepAlive;
  keepAlive.insert("SuccessfulExit", false);
  launchPlist.insert("Keep-Alive", keepAlive);

  launchPlist.insert("ProcessType", "Background");
  launchPlist.insert("Program", HelperLauncher::HelperPath());

  PListSerializer plistOut;
  QString output = plistOut.toPList(launchPlist);

  if (!output.isEmpty())
  {
    QFile fp(launchPlistPath());
    if (fp.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
      fp.write(output.toUtf8());
    }
    else
    {
      QLOG_WARN() << "Failed to write launchd plist file:" << launchPlistPath();
      return false;
    }
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool HelperLaunchd::loadHelper()
{
  QLOG_DEBUG() << "Loading helper";
  QStringList args;
  args << "load";
  args << launchPlistPath();
  m_launchctl->start(LAUNCHCTL_PATH, args);
  m_launchctl->waitForFinished(5000);
  return m_launchctl->exitStatus() == QProcess::NormalExit;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool HelperLaunchd::unloadHelper()
{
  QLOG_DEBUG() << "Unloading helper";
  QStringList args;
  args << "unload";
  args << launchPlistPath();
  m_launchctl->start(LAUNCHCTL_PATH, args);
  m_launchctl->waitForFinished(5000);

  return m_launchctl->exitStatus() == QProcess::NormalExit;
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLaunchd::start()
{
  unloadHelper();

  if (!checkHelperPath())
    writePlist();

  loadHelper();
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLaunchd::stop()
{
  unloadHelper();
}
