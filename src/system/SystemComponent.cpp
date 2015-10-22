#include <QSysInfo>
#include <QProcess>
#include <QMap>
#include <QtNetwork/qnetworkinterface.h>
#include <QGuiApplication>
#include <QDesktopServices>

#include "SystemComponent.h"
#include "Version.h"
#include "QsLog.h"
#include "settings/SettingsComponent.h"
#include "ui/KonvergoWindow.h"
#include "Paths.h"
#include "Names.h"

#define MOUSE_TIMEOUT 5 * 1000

#define KONVERGO_PRODUCTID_DEFAULT  3
#define KONVERGO_PRODUCTID_OPENELEC 4

// Platform types map
QMap<SystemComponent::PlatformType, QString> platformTypeNames = { \
  { SystemComponent::platformTypeOsx, "macosx" }, \
  { SystemComponent::platformTypeWindows, "windows" },
  { SystemComponent::platformTypeLinux, "linux" },
  { SystemComponent::platformTypeOpenELEC, "openelec" },
  { SystemComponent::platformTypeUnknown, "unknown" },
};

// platform Archictecture map
QMap<SystemComponent::PlatformArch, QString> platformArchNames = { \
  { SystemComponent::platformArchX86_64, "x86_64" }, \
  { SystemComponent::platformArchRpi2, "rpi2" },
  { SystemComponent::platformArchUnknown, "unknown" }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
SystemComponent::SystemComponent(QObject* parent) : ComponentBase(parent), m_platformType(platformTypeUnknown), m_platformArch(platformArchUnknown), m_doLogMessages(false)
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::getPlatformTypeString() const
{
  return platformTypeNames[m_platformType];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::getPlatformArchString() const
{
  return platformArchNames[m_platformArch];
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
  arch = "x86_64";
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
  if (visible)
  {
    m_mouseOutTimer->start(MOUSE_TIMEOUT);

    while (qApp->overrideCursor())
      qApp->restoreOverrideCursor();
  }
  else
  {
    if (!qApp->overrideCursor())
    {
      if (m_mouseOutTimer->isActive())
        m_mouseOutTimer->stop();

      qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::getUserAgent()
{
  QString osVersion = QSysInfo::productVersion();
  QString userAgent = QString("PlexMediaPlayer %1 (%2-%3 %4)").arg(Version::GetVersionString()).arg(getPlatformTypeString()).arg(getPlatformArchString()).arg(osVersion);
  return userAgent;
}

#ifdef Q_OS_UNIX
/////////////////////////////////////////////////////////////////////////////////////////
QMap<QString, QString> SystemComponent::networkInterfaces()
{
  QMap<QString, QString> info;

  foreach(const QNetworkInterface& interface, QNetworkInterface::allInterfaces())
  {
    if (interface.isValid() == false || (interface.hardwareAddress().isEmpty() == true) || interface.IsLoopBack == false)
      continue;

    QString interfaceName = QString("Interface%1 ").arg(interface.index());
    info[interfaceName + "Name"] = interface.humanReadableName();
    info[interfaceName + "HW address"] = interface.hardwareAddress();
    info[interfaceName + "Status"] = (interface.flags() & QNetworkInterface::IsUp) ? "Up" : "Down";

    int i = 0;
    foreach(const QNetworkAddressEntry& address, interface.addressEntries())
    {
      info[interfaceName + QString("Address%1").arg(i)] = QString("%1/%2").arg(address.ip().toString()).arg(address.netmask().toString());
      i++;
    }
  }
  return info;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
QString SystemComponent::debugInformation()
{
  QString debugInfo;
  QTextStream stream(&debugInfo);

  stream << "Plex Media Player" << endl;
  stream << "  Version: " << Version::GetVersionString() << " built: " << Version::GetBuildDate() << endl;
  stream << "  Web Client Version: " << Version::GetWebVersion() << endl;
  stream << "  Platform: " << getPlatformTypeString() << "-" << getPlatformArchString() << endl;
  stream << "  User-Agent: " << getUserAgent() << endl;
  stream << "  Qt version: " << qVersion() << endl;
  stream << endl;

  stream << "Files" << endl;
  stream << "  Log file: " << Paths::logDir(Names::MainName() + ".log") << endl;
  stream << "  Config file: " << Paths::dataDir(Names::MainName() + ".conf") << endl;
  stream << endl;

#ifdef Q_OS_UNIX
  stream << "Network" << endl;
  QMap<QString, QString> networks = networkInterfaces();
  foreach(const QString& net, networks.keys())
    stream << "  " << net << ": " << networks[net] << endl;
#endif

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
  foreach(const QHostAddress& address, QNetworkInterface::allAddresses())
  {
    if (! address.isLoopback() && (address.protocol() == QAbstractSocket::IPv4Protocol))
      list << address.toString();
  }

  return list;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::userInformation(const QVariantMap& userModel)
{
  QStringList roleList;
  auto roles = userModel.value("roles").toMap();
  foreach (const QString& key, roles.keys())
  {
    if (roles.value(key).toBool())
      roleList << key;
  }

  SettingsComponent::Get().setUserRoleList(roleList);
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemComponent::openExternalUrl(const QString& url)
{
  QDesktopServices::openUrl(QUrl(url));
}
