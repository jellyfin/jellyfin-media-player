#include "Utils.h"
#include <QtGlobal>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QDateTime>
#include <QHostInfo>
#include <QJsonDocument>
#include <QVariant>
#include <qnetworkinterface.h>
#include <QUuid>
#include <QFile>
#include <QSaveFile>
#include <QRegExp>
#include <QDebug>

#include <mutex>

#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"

QList<QChar> httpSeparators = { '(', ')', '<', '>', '@', ',', ';', ':', '\\', '\"', '/', '[', ']', '?', '=', '{', '}', '\'' };

/////////////////////////////////////////////////////////////////////////////////////////
QString Utils::sanitizeForHttpSeparators(const QString& input)
{
  auto output = input;

  for (const QChar& c : httpSeparators)
    output.replace(c, "");

  for (int i = 0; i < output.size(); i ++)
  {
    if (output.at(i).unicode() > 127)
      output[i] = '_';
  }
  return output;
}

/////////////////////////////////////////////////////////////////////////////////////////
QString Utils::ComputerName()
{
  static std::once_flag flag;
  static QString name;
  std::call_once(flag, [](){
#ifdef Q_OS_MAC
    name = OSXUtils::ComputerName();
#else
    name = QHostInfo::localHostName();
#endif
  });
  return name;
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

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Utils::safelyWriteFile(const QString& filename, const QByteArray& data)
{
  QSaveFile file(filename);
  if (!file.open(QIODevice::WriteOnly))
    return false;
  file.write(data);
  return file.commit();
}
