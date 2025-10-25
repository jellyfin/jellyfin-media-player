//
// Created by Tobias Hieta on 07/03/16.
//

#include "Log.h"

#include <QtQml>
#include <QGuiApplication>
#include <iostream>

#include "shared/Names.h"
#include "shared/Paths.h"
#include "settings/SettingsComponent.h"
#include "Version.h"

int fileLogLevel = QtDebugMsg;
int terminalLogLevel = QtWarningMsg;

/////////////////////////////////////////////////////////////////////////////////////////
// adapted from https://stackoverflow.com/a/62390212
static void qtMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  static QMutex mutex;
  QMutexLocker lock(&mutex);

  // Check if message meets any output threshold
  bool shouldOutputToTerminal = type >= terminalLogLevel;
  bool shouldOutputToFile = type >= fileLogLevel;

  if (!shouldOutputToTerminal && !shouldOutputToFile && type != QtFatalMsg)
    return;

  static QFile logFile(Paths::logDir(Names::MainName() + ".log"));
  static bool logFileIsOpen = logFile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text);

  QString message = qFormatLogMessage(type, context, msg);
  Log::CensorAuthTokens(message);

  if (shouldOutputToTerminal) {
    std::cerr << qPrintable(message) << std::endl;
  }

  if (logFileIsOpen && shouldOutputToFile) {
    logFile.write(message.toUtf8() + '\n');
    logFile.flush();
  }

  if (type == QtFatalMsg)
    abort();
}

/////////////////////////////////////////////////////////////////////////////////////////
static void elidePattern(QString& msg, const QString& substring, int chars)
{
  int start = 0;
  while (true)
  {
    start = msg.indexOf(substring, start);
    if (start < 0 || start + substring.length() + chars > msg.length())
      break;
    start += substring.length();
    for (int n = 0; n < chars; n++)
      msg[start + n] = QChar('x');
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::CensorAuthTokens(QString& msg)
{
  elidePattern(msg, "api_key=", 32);
  elidePattern(msg, "X-MediaBrowser-Token%3D", 32);
  elidePattern(msg, "X-MediaBrowser-Token=", 32);
  elidePattern(msg, "api_key=", 32);
  elidePattern(msg, "ApiKey=", 32);
  elidePattern(msg, "AccessToken=", 32);
  elidePattern(msg, "AccessToken\":\"", 32);
}

/////////////////////////////////////////////////////////////////////////////////////////
static int logLevelFromString(const QString& str)
{
  if (str == "debug")     return QtDebugMsg;
  if (str == "info")      return QtInfoMsg;
  if (str == "warn")      return QtWarningMsg;
  if (str == "error")     return QtCriticalMsg;
  if (str == "fatal")     return QtFatalMsg;
  // if not valid, use default
  return QtDebugMsg;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::SetFileLogLevel()
{
  QString level = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "logLevel").toString();
  if (level.size())
  {
    qInfo() << "Setting file log level to:" << level;
    fileLogLevel = logLevelFromString(level);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::Init()
{
  qSetMessagePattern("%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type}] %{function} @ %{line} - %{message}");
  qInstallMessageHandler(qtMessageOutput);

  // Note where the logfile is going to be (now uses our handler)
  qDebug() << "Logging to " << qPrintable(Paths::logDir(Names::MainName() + ".log"));

  qInfo() << "Starting Jellyfin Media Player version:" << qPrintable(Version::GetVersionString()) << "build date:" << qPrintable(Version::GetBuildDate());
  qInfo() << qPrintable(QString("  Running on: %1 [%2] arch %3").arg(QSysInfo::prettyProductName()).arg(QSysInfo::kernelVersion()).arg(QSysInfo::currentCpuArchitecture()));
  qInfo() << "  Qt Version:" << QT_VERSION_STR << qPrintable(QString("[%1]").arg(QSysInfo::buildAbi()));
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::SetTerminalLogLevel(int level)
{
  terminalLogLevel = level;
}
