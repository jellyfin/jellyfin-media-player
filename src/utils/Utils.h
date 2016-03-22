#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QUrl>
#include <QVariant>
#include <QException>
#include <QtCore/qjsondocument.h>

#ifdef Q_OS_MAC
  #include "osx/OSXUtils.h"
#endif

#define DEFINE_SINGLETON(cls) \
  public:                     \
  static cls& Get()           \
  {                           \
    static cls __instance;    \
    return __instance;        \
  }                           \


class FatalException : public QException
{
public:
  explicit FatalException(const QString& message) : m_message(message) {}
  const QString& message() const { return m_message; }

  ~FatalException() throw() override { }

private:
  QString m_message;
};

  enum Platform
  {
    PLATFORM_UNKNOWN = 0,
    PLATFORM_OSX = (1 << 0),
    PLATFORM_LINUX = (1 << 1),
    PLATFORM_OE_X86 = (1 << 2),
    PLATFORM_OE_RPI = (1 << 3),
    PLATFORM_WINDOWS = (1 << 4),
    PLATFORM_OE = (PLATFORM_OE_RPI | PLATFORM_OE_X86),
    PLATFORM_ANY = (PLATFORM_OSX | PLATFORM_WINDOWS | PLATFORM_LINUX | PLATFORM_OE)
  };

#define PLATFORM_ANY_EXCEPT(x) (PLATFORM_ANY & (~(x)))

namespace Utils
{
  Platform CurrentPlatform();
  QJsonDocument OpenJsonDocument(const QString& path, QJsonParseError* err);
  QString CurrentUserId();
  QString ComputerName();
  QString PrimaryIPv4Address();
  QString ClientUUID();
}

#endif // UTILS_H
