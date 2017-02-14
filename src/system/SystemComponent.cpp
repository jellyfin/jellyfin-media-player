#include <QSysInfo>
#include <QProcess>
#include <QMap>
#include <QtNetwork/qnetworkinterface.h>
#include <QGuiApplication>
#include <QDesktopServices>
#include <QDir>

#include "input/InputComponent.h"
#include "SystemComponent.h"
#include "Version.h"
#include "QsLog.h"
#include "settings/SettingsComponent.h"
#include "ui/KonvergoWindow.h"
#include "settings/SettingsSection.h"
#include "Paths.h"
#include "Names.h"
#include "utils/Utils.h"

#define MOUSE_TIMEOUT 5 * 1000

#define KONVERGO_PRODUCTID_DEFAULT  3
#define KONVERGO_PRODUCTID_OPENELEC 4

// Platform types map
QMap<SystemComponent::PlatformType, QString> g_platformTypeNames = { \
  { SystemComponent::platformTypeOsx, "macosx" }, \
  { SystemComponent::platformTypeWindows, "windows" },
  { SystemComponent::platformTypeLinux, "linux" },
  { SystemComponent::platformTypeOpenELEC, "openelec" },
  { SystemComponent::platformTypeUnknown, "unknown" },
};

// platform Archictecture map
QMap<SystemComponent::PlatformArch, QString> g_platformArchNames = {
  { SystemComponent::platformArchX86_32, "i386" },
  { SystemComponent::platformArchX86_64, "x86_64" },
  { SystemComponent::platformArchRpi2, "rpi2" },
  { SystemComponent::platformArchUnknown, "unknown" }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
SystemComponent::SystemComponent(QObject* parent) : ComponentBase(parent), m_platformType(platformTypeUnknown), m_platformArch(platformArchUnknown), m_doLogMessages(false), m_cursorVisible(false)
{
  m_mouseOutTimer = new QTimer(this);
  m_mouseOutTimer->setSingleShot(true);
  connect(m_mouseOutTimer, &QTimer::timeout, [&] () { setCursorVisibility(false); });

  m_mouseOutTimer->start(MOUSE_TIMEOUT);

// define OS Type
#if defined(Q_OS_MAC)
  m_platformType = platformTypeOsx;
#elif defined(Q_OS_WIN)
  m_platformType = platformTypeWindows;
#elif defined(KONVERGO_OPENELEC)
  m_platformType = platformTypeOpenELEC;
#elif defined(Q_OS_LINUX)
  m_platformType = platformTypeLinux;
#endif

// define target type
#if TARGET_RPI
  m_platformArch = platformArchRpi2;
#elif defined(Q_PROCESSOR_X86_32)
  m_platformArch = platformArchX86_32;
#elif defined(Q_PROCESSOR_X86_64)
  m_platformArch = platformArchX86_64;
#endif

  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO), &SettingsSection::valuesUpdated, [=]()
  {
    emit capabilitiesChanged(getCapabilitiesString());
  });
}

/////////////////////////////////////////////////////////////////////////////////////////
bool SystemComponent::componentInitialize()
{
  QDir().mkpath(Paths::dataDir("scripts"));
  QDir().mkpath(Paths::dataDir("sounds"));

  // Hide mouse pointer on any keyboard input
  connect(&InputComponent::Get(), &InputComponent::receivedInput, [=]() { setCursorVisibility(false); });

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::crashApp()
{
  *(volatile int*)nullptr=0;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::componentPostInitialize()
{
  InputComponent::Get().registerHostCommand("crash!", this, "crashApp");
  InputComponent::Get().registerHostCommand("script", this, "runUserScript");
  InputComponent::Get().registerHostCommand("message", this, "hostMessage");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::getPlatformTypeString() const
{
  return g_platformTypeNames[m_platformType];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::getPlatformArchString() const
{
  return g_platformArchNames[m_platformArch];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariantMap SystemComponent::systemInformation() const
{
  QVariantMap info;
  QString build;
  QString dist;
  QString arch;
  int productid = KONVERGO_PRODUCTID_DEFAULT;

#ifdef Q_OS_WIN
  arch = (sizeof(void *) == 8) ? "x86_64" : "i386";
#else
  arch = QSysInfo::currentCpuArchitecture();
#endif

  build = getPlatformTypeString();
  dist = getPlatformTypeString();

#if defined(KONVERGO_OPENELEC)
  productid = KONVERGO_PRODUCTID_OPENELEC;
  dist = "openelec";

  if (m_platformArch == platformArchRpi2)
  {
    build = "rpi2";
  }
  else
  {
    build = "generic";
  }
#endif

  
  info["build"] = build + "-" + arch;
  info["dist"] = dist;
  info["version"] = Version::GetVersionString();
  info["productid"] = productid;
  
 QLOG_DEBUG() << QString(
                "System Information : build(%1)-arch(%2).dist(%3).version(%4).productid(%5)")
                .arg(build)
                .arg(arch)
                .arg(dist)
                .arg(Version::GetVersionString())
                .arg(productid);
 return info;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::exit()
{
  qApp->quit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::restart()
{
  qApp->quit();
  QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::info(QString text)
{
  if (QsLogging::Logger::instance().loggingLevel() <= QsLogging::InfoLevel)
    QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() << "JS:" << qPrintable(text);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::setCursorVisibility(bool visible)
{
  if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "webMode") == "desktop")
    visible = true;

  if (visible == m_cursorVisible)
    return;

  m_cursorVisible = visible;

  if (visible)
    m_mouseOutTimer->start(MOUSE_TIMEOUT);
  else
    m_mouseOutTimer->stop();

#ifdef Q_OS_MAC
  // OSX notifications will reset the cursor image (without Qt's knowledge). The
  // only thing we can do override this is using Cocoa's native cursor hiding.
  OSXUtils::SetCursorVisible(visible);
#else
  if (visible)
    qApp->restoreOverrideCursor();
  else
    qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::getUserAgent()
{
  QString osVersion = QSysInfo::productVersion();
  QString userAgent = QString("PlexMediaPlayer %1 (%2-%3 %4)").arg(Version::GetVersionString()).arg(getPlatformTypeString()).arg(getPlatformArchString()).arg(osVersion);
  return userAgent;
}

/////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::debugInformation()
{
  QString debugInfo;
  QTextStream stream(&debugInfo);

  stream << "Plex Media Player" << endl;
  stream << "  Version: " << Version::GetVersionString() << " built: " << Version::GetBuildDate() << endl;
  stream << "  Web Client Version: " << Version::GetWebVersion() << endl;
  stream << "  Web Client URL: " << SettingsComponent::Get().value(SETTINGS_SECTION_PATH, "startupurl").toString() << endl;
  stream << "  Platform: " << getPlatformTypeString() << "-" << getPlatformArchString() << endl;
  stream << "  User-Agent: " << getUserAgent() << endl;
  stream << "  Qt version: " << qVersion() << QString("(%1)").arg(Version::GetQtDepsVersion()) << endl;
  stream << "  Depends version: " << Version::GetDependenciesVersion() << endl;
  stream << endl;

  stream << "Files" << endl;
  stream << "  Log file: " << Paths::logDir(Names::MainName() + ".log") << endl;
  stream << "  Config file: " << Paths::dataDir(Names::MainName() + ".conf") << endl;
  stream << endl;

  stream << "Network Addresses" << endl;
  for(const QString& addr : networkAddresses())
  {
    stream << "  " << addr << endl;
  }
  stream << endl;

  stream << flush;
  return debugInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
int SystemComponent::networkPort() const
{
  return SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "webserverport").toInt();
}

/////////////////////////////////////////////////////////////////////////////////////////
QStringList SystemComponent::networkAddresses() const
{
  QStringList list;
  for(const QHostAddress& address : QNetworkInterface::allAddresses())
  {
    if (! address.isLoopback() && (address.protocol() == QAbstractSocket::IPv4Protocol ||
                                   address.protocol() == QAbstractSocket::IPv6Protocol))
    {
      auto s = address.toString();
      if (!s.startsWith("fe80::"))
        list << s;
    }
  }

  return list;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::userInformation(const QVariantMap& userModel)
{
  QStringList roleList;
  for(const QVariant& role : userModel.value("roles").toList())
  { 
    roleList << role.toMap().value("id").toString();
  }

  SettingsComponent::Get().setUserRoleList(roleList);

  m_authenticationToken = userModel.value("authToken").toString();
  emit userInfoChanged();
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::openExternalUrl(const QString& url)
{
  QDesktopServices::openUrl(QUrl(url));
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::runUserScript(QString script)
{
  // We take the path the user supplied and run it through fileInfo and
  // look for the fileName() part, this is to avoid people sharing keymaps
  // that tries to execute things like ../../ etc. Note that this function
  // is still not safe, people can do nasty things with it, so users needs
  // to be careful with their keymaps.
  //
  QFileInfo fi(script);
  QString scriptPath = Paths::dataDir("scripts/" + fi.fileName());

  QFile scriptFile(scriptPath);
  if (scriptFile.exists())
  {
    if (!QFileInfo(scriptFile).isExecutable())
    {
      QLOG_WARN() << "Script:" << script << "is not executable";
      return;
    }

    QLOG_INFO() << "Running script:" << scriptPath;

    if (QProcess::startDetached(scriptPath, QStringList()))
      QLOG_DEBUG() << "Script started successfully";
    else
      QLOG_WARN() << "Error running script:" << scriptPath;
  }
  else
  {
    QLOG_WARN() << "Could not find script:" << scriptPath;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::hello(const QString& version)
{
  QLOG_DEBUG() << QString("Web-client (%1) fully inited.").arg(version);
  m_webClientVersion = version;
}

/////////////////////////////////////////////////////////////////////////////////////////
#define BASESTR "protocols=shoutcast,http-video;videoDecoders=h264{profile:high&resolution:2160&level:52};audioDecoders=mp3,aac,dts{bitrate:800000&channels:%1},ac3{bitrate:800000&channels:%2}"

/////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::getCapabilitiesString()
{
  auto capstring = QString(BASESTR);
  auto channels = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "channels").toString();
  auto dtsenabled = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "passthrough.dts").toBool();
  auto ac3enabled = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "passthrough.ac3").toBool();

  // Assume that auto means that we want to select multi-channel tracks by default.
  // So really only disable it when 2.0 is selected.
  //
  int ac3channels = 2;
  int dtschannels = 2;

  if (channels != "2.0")
    dtschannels = ac3channels = 8;
  else if (dtsenabled)
    dtschannels = 8;
  else if (ac3enabled)
    ac3channels = 8;

  return capstring.arg(dtschannels).arg(ac3channels);
}

