#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include "PowerComponentDBus.h"

#define DBUS_SERVICE_NAME "org.freedesktop.login1"
#define DBUS_SERVICE_PATH "/org/freedesktop/login1"
#define DBUS_INTERFACE "org.freedesktop.login1.Manager"

#define DBUS_SCREENSAVER_SERVICE_NAME "org.freedesktop.ScreenSaver"
#define DBUS_SCREENSAVER_SERVICE_PATH "/org/freedesktop/ScreenSaver"
#define DBUS_SCREENSAVER_INTERFACE "org.freedesktop.ScreenSaver"

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

void PowerComponentDBus::doDisableScreensaver()
{
  if (screensaver_inhibit_cookie)
  {
    QLOG_INFO() << "doDisableScreensaver : already disabled.";
    return;
  }
  if (QDBusConnection::systemBus().isConnected())
  {
    QDBusInterface iface(DBUS_SCREENSAVER_SERVICE_NAME, DBUS_SCREENSAVER_SERVICE_PATH,
                         DBUS_SCREENSAVER_INTERFACE, QDBusConnection::sessionBus());
    if (iface.isValid())
    {
      QDBusReply<unsigned int> reply = iface.call("Inhibit", "plexmediaplayer", "playing");

      if (reply.isValid())
      {
        screensaver_inhibit_cookie = reply.value();
      }
      else
      {
        QLOG_ERROR() << "doDisableScreensaver : Error while calling UnInhibit:"
                     << reply.error().message();
      }
    }
    else
    {
      QLOG_ERROR() << "doDisableScreensaver : failed to retrieve interface.";
    }
  }
  else
  {
    QLOG_ERROR() << "doDisableScreensaver : could not find system bus";
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentDBus::doEnableScreensaver()
{
  if (!screensaver_inhibit_cookie)
  {
    QLOG_INFO() << "doEnableScreensaver : already enabled.";
    return;
  }
  if (QDBusConnection::systemBus().isConnected())
  {
    QDBusInterface iface(DBUS_SCREENSAVER_SERVICE_NAME, DBUS_SCREENSAVER_SERVICE_PATH,
                         DBUS_SCREENSAVER_INTERFACE, QDBusConnection::sessionBus());
    if (iface.isValid())
    {
      QDBusReply<void> reply = iface.call("UnInhibit", screensaver_inhibit_cookie);

      if (reply.isValid())
      {
        screensaver_inhibit_cookie = 0;
      }
      else
      {
        QLOG_ERROR() << "doEnableScreensaver : Error while calling UnInhibit:"
                     << reply.error().message();
      } 
    }
    else
    {
      QLOG_ERROR() << "doEnableScreensaver : failed to retrieve interface.";
    }
  }
  else
  {
    QLOG_ERROR() << "doEnableScreensaver : could not find system bus";
  }
}

