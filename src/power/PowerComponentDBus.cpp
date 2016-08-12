#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include "PowerComponentDBus.h"

#define DBUS_SERVICE_NAME "org.freedesktop.login1"
#define DBUS_SERVICE_PATH "/org/freedesktop/login1"
#define DBUS_INTERFACE "org.freedesktop.login1.Manager"

/////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentDBus::callPowerMethod(QString method)
{
  if (QDBusConnection::systemBus().isConnected())
  {
    QDBusInterface iface(DBUS_SERVICE_NAME, DBUS_SERVICE_PATH, DBUS_INTERFACE,
                         QDBusConnection::systemBus());

    if (iface.isValid())
    {
      QDBusReply<bool> reply = iface.call(method, true);

      if (reply.isValid())
      {
        return true;
      }
      else
      {
        QLOG_ERROR() << "callPowerMethod : Error while calling" << method << ":"
                     << reply.error().message();
        return false;
      }
    }
    else
    {
      QLOG_ERROR() << "callPowerMethod : failed to retrieve interface.";
    }
  }
  else
  {
    QLOG_ERROR() << "callPowerMethod : could not find system bus";
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentDBus::isPowerMethodAvailable(QString method)
{
  if (QDBusConnection::systemBus().isConnected())
  {
    QDBusInterface iface(DBUS_SERVICE_NAME, DBUS_SERVICE_PATH, DBUS_INTERFACE,
                         QDBusConnection::systemBus());

    if (iface.isValid())
    {
      QDBusReply<QString> reply = iface.call(method);

      if (reply.isValid())
      {
        return (reply.value() == "yes");
      }
      else
      {
        QLOG_ERROR() << "isPowerMethodAvailable : Error while calling" << method << ":"
                     << reply.error().message();
        return false;
      }
    }
    else
    {
      QLOG_ERROR() << "isPowerMethodAvailable : failed to retrieve interface.";
    }
  }
  else
  {
    QLOG_ERROR() << "isPowerMethodAvailable : could not find system bus";
  }

  return false;
}
