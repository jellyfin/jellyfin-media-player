//
// Created by Tobias Hieta on 07/03/16.
//

#include "Log.h"

#include <QtQml>
#include <QGuiApplication>

#include "QsLog.h"
#include "shared/Names.h"
#include "shared/Paths.h"
#include "settings/SettingsComponent.h"
#include "Version.h"

using namespace QsLogging;

/////////////////////////////////////////////////////////////////////////////////////////
static void qtMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString prefix;
    if (context.line)
      prefix = QString("%1:%2:%3: ").arg(context.file).arg(context.line).arg(context.function);
    QString text = prefix + msg;
    switch (type)
    {
      case QtDebugMsg:
        QLOG_DEBUG() << text;
        break;
      case QtInfoMsg:
        QLOG_INFO() << text;
        break;
      case QtWarningMsg:
        QLOG_WARN() << text;
        break;
      case QtCriticalMsg:
        QLOG_ERROR() << text;
        break;
      case QtFatalMsg:
        QLOG_FATAL() << text;
        break;
    }
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
  elidePattern(msg, "X-Plex-Token=", 20);
  elidePattern(msg, "X-Plex-Token%3D", 20);
  elidePattern(msg, "auth_token=", 20);
  elidePattern(msg, "authenticationToken=\"", 20);
  elidePattern(msg, "token=", 20);
}

/////////////////////////////////////////////////////////////////////////////////////////
static QsLogging::Level logLevelFromString(const QString& str)
{
  if (str == "trace")     return QsLogging::Level::TraceLevel;
  if (str == "debug")     return QsLogging::Level::DebugLevel;
  if (str == "info")      return QsLogging::Level::InfoLevel;
  if (str == "warn")      return QsLogging::Level::WarnLevel;
  if (str == "error")     return QsLogging::Level::ErrorLevel;
  if (str == "fatal")     return QsLogging::Level::FatalLevel;
  if (str == "disable")   return QsLogging::Level::OffLevel;
  // if not valid, use default
  return QsLogging::Level::DebugLevel;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::UpdateLogLevel()
{
  QString level = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "logLevel").toString();
  if (level.size())
  {
    QLOG_INFO() << "Setting log level to:" << level;
    Logger::instance().setLoggingLevel(logLevelFromString(level));
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Log::Init()
{
  // Note where the logfile is going to be
  qDebug("Logging to %s", qPrintable(Paths::logDir(Names::MainName() + ".log")));

  // init logging.
  DestinationPtr dest = DestinationFactory::MakeFileDestination(
    Paths::logDir(Names::MainName() + ".log"),
    EnableLogRotationOnOpen,
    MaxSizeBytes(1024 * 1024),
    MaxOldLogCount(9));

  Logger::instance().addDestination(dest);
  Logger::instance().setLoggingLevel(DebugLevel);
  Logger::instance().setProcessingCallback(Log::CensorAuthTokens);

  qInstallMessageHandler(qtMessageOutput);

  QLOG_INFO() << "Starting Plex Media Player version:" << qPrintable(Version::GetVersionString()) << "build date:" << qPrintable(Version::GetBuildDate());
  QLOG_INFO() << qPrintable(QString("  Running on: %1 [%2] arch %3").arg(QSysInfo::prettyProductName()).arg(QSysInfo::kernelVersion()).arg(QSysInfo::currentCpuArchitecture()));
  QLOG_INFO() << "  Qt Version:" << QT_VERSION_STR << qPrintable(QString("[%1]").arg(QSysInfo::buildAbi()));
}
