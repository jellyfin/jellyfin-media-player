#include <QDebug>
#include "settings/SettingsComponent.h"
#include "InputAppleRemote.h"

#include "HIDRemote.h"
#include "AppleRemoteDelegate.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputAppleRemote::initInput()
{
  m_delegate = [[AppleRemoteDelegate alloc] initWithRemoteHandler:this];
  return [m_delegate setupRemote];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::addRemoteFailed(const QString &error)
{
  qCritical() << error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::addRemote(const QString &name)
{
  m_remotes << name;
  qDebug() << "Added remote:" << name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::removeRemote(const QString &name)
{
  m_remotes.removeOne(name);
  qDebug() << "Remove remote:" << name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::remoteButtonEvent(quint8 code, bool pressed, const QString &name)
{
  // This following code emulates the harmony remotes input to PHT. The Apple API is actually
  // limited to just the number of keys that are on Apple specific remotes so we had to work
  // around that with the following hack: deviceID was used as a prefix for what the keys meant
  // so for example if we send deviceID 150 and keycode UP it means up, but instead deviceID 151
  // and UP it can be mapped to a different key.
  //
  // The harmony profile for this is using deviceID's 150-160 to prefix the keycode, so we will
  // treat them differently here than the other keycodes. We append the deviceID to the keycode
  // when the deviceID falls inside the 150-160 range, otherwise we assume the keycode is coming
  // from a "normal" remote and only forward the keycode.
  //
  // Since it's unknown if this will cause problems with any remotes I have added a setting:
  // appleremote.emulatepht to turn this off if needed, but for now we'll keep it defaulted to on
  //
  QString eventName;
  if (SettingsComponent::Get().value(SETTINGS_SECTION_APPLEREMOTE, "emulatepht").toBool() &&
      (m_remoteID >= 150 && m_remoteID <= 160))
    eventName = QString("%1-%2").arg(m_remoteID).arg(code);
  else
    eventName = QString::number(code);

  emit receivedInput("AppleRemote", eventName, pressed ? KeyDown : KeyUp);
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::changeRemoteID(quint32 newID)
{
  m_remoteID = newID;
}
