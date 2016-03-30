#include "Utils.h"
#include <QtGlobal>
#include <QStandardPaths>
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QDateTime>
#include <QHostInfo>
#include <QJsonDocument>
#include <QVariant>
#include <qnetworkinterface.h>
#include <QUuid>

#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"

#include "osx/OSXUtils.h"
#include "QsLog.h"


/////////////////////////////////////////////////////////////////////////////////////////
QString Utils::ComputerName()
{
#ifdef Q_OS_MAC
  return OSXUtils::ComputerName();
#else
  return QHostInfo::localHostName();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
QJsonDocument Utils::OpenJsonDocument(const QString& path, QJsonParseError* err)
{
  QFile fp(path);
  QByteArray fdata;
  QRegExp commentMatch("^\\s*//");

  if (fp.open(QFile::ReadOnly))
  {
    while(true)
    {
      QByteArray row = fp.readLine();

      if (row.isEmpty())
        break;

      // filter all comments
      if (commentMatch.indexIn(row) != -1)
        continue;

      fdata.append(row);
    }

    fp.close();
  }

  return QJsonDocument::fromJson(fdata, err);
}

/////////////////////////////////////////////////////////////////////////////////////////
Platform Utils::CurrentPlatform()
{
#if defined(Q_OS_MAC)
  return PLATFORM_OSX;
#elif KONVERGO_OPENELEC
  #if TARGET_RPI
    return PLATFORM_OE_RPI;
  #else
    return PLATFORM_OE_X86;
  #endif
#elif defined(Q_OS_LINUX)
  return PLATFORM_LINUX;
#elif defined(Q_OS_WIN32)
  return PLATFORM_WINDOWS;
#else
  return PLATFORM_UNKNOWN;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Utils::CurrentUserId()
{
  SettingsSection* connections = SettingsComponent::Get().getSection("connections");
  if (!connections)
    return QString();

  QVariant ulist = connections->value("users");
  if (ulist.isValid())
  {
    QVariantList users = ulist.toList();
    if (users.size() > 0)
    {
      QVariantMap user = users.at(0).toMap();
      return user.value("id").toString();
    }
  }

  return QString();
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Utils::PrimaryIPv4Address()
{
  QList<QNetworkInterface> ifs = QNetworkInterface::allInterfaces();
  for(const QNetworkInterface& iface : ifs)
  {
    if (iface.isValid() && iface.flags() & QNetworkInterface::IsUp)
    {
      QList<QHostAddress> addresses = iface.allAddresses();
      for(const QHostAddress& addr : addresses)
      {
        if (!addr.isLoopback() && !addr.isMulticast() && addr.protocol() == QAbstractSocket::IPv4Protocol)
          return addr.toString();
      }
    }
  }
  return "";
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Utils::ClientUUID()
{
  QString storedUUID = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "clientUUID").toString();
  if (storedUUID.isEmpty())
  {
    QString newUUID = QUuid::createUuid().toString();
    newUUID = newUUID.replace("{", "").replace("}", "");
    SettingsComponent::Get().setValue(SETTINGS_SECTION_MAIN, "clientUUID", newUUID);
    return newUUID;
  }
  return storedUUID;
}
