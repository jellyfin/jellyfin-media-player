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

int logLevel = 0;
bool logToTerminal = false;
QHash<QtMsgType, int> messageLevelValue({{QtDebugMsg, 1}, {QtInfoMsg, 2}, {QtWarningMsg, 3}, {QtCriticalMsg, 4}, {QtFatalMsg, 5}});

/////////////////////////////////////////////////////////////////////////////////////////
// adapted from https://stackoverflow.com/a/62390212
static void qtMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  static QMutex mutex;
  QMutexLocker lock(&mutex);

  if (messageLevelValue[type] < logLevel && type != QtFatalMsg)
    return;

  static QFile logFile(Paths::logDir(Names::MainName() + ".log"));
  static bool logFileIsOpen = logFile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text);

  QString message = qFormatLogMessage(type, context, msg);
  Log::CensorAuthTokens(message);

  if (logToTerminal) {
    std::cerr << qPrintable(message) << std::endl;
  } 

  if (logFileIsOpen) {
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
}

/////////////////////////////////////////////////////////////////////////////////////////
static int logLevelFromString(const QString& str)
{
  if (str == "trace")     return 0;
  if (str == "debug")     return 1;
  if (str == "info")      return 2;
  if (str == "warn")      return 3;
  if (str == "error")     return 4;
  if (str == "fatal")     return 5;
  // if not valid, use default
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::UpdateLogLevel()
{
  QString level = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "logLevel").toString();
  if (level.size())
  {
    qInfo() << "Setting log level to:" << level;
    logLevel = logLevelFromString(level);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Log::ShouldLogInfo()
{
  return logLevel <= 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::Init()
{
  // Note where the logfile is going to be
  qDebug() << "Logging to " << qPrintable(Paths::logDir(Names::MainName() + ".log"));

  qSetMessagePattern("%{time yyyy-MM-dd hh:mm:ss.zzz} [%{type}] %{function} @ %{line} - %{message}");
  qInstallMessageHandler(qtMessageOutput);

  qInfo() << "Starting Jellyfin Media Player version:" << qPrintable(Version::GetVersionString()) << "build date:" << qPrintable(Version::GetBuildDate());
  qInfo() << qPrintable(QString("  Running on: %1 [%2] arch %3").arg(QSysInfo::prettyProductName()).arg(QSysInfo::kernelVersion()).arg(QSysInfo::currentCpuArchitecture()));
  qInfo() << "  Qt Version:" << QT_VERSION_STR << qPrintable(QString("[%1]").arg(QSysInfo::buildAbi()));
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::EnableTerminalOutput()
{
  logToTerminal = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::Uninit()
{
  qInstallMessageHandler(0);
}
