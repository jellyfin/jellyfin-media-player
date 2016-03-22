#include "InputAppleRemote.h"
#include "QsLog.h"

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
  QLOG_ERROR() << error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::addRemote(const QString &name)
{
  m_remotes << name;
  QLOG_DEBUG() << "Added remote:" << name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::removeRemote(const QString &name)
{
  m_remotes.removeOne(name);
  QLOG_DEBUG() << "Remove remote:" << name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::remoteButtonEvent(quint8 code, bool pressed, const QString &name)
{
  emit receivedInput("AppleRemote", QString::number(code), pressed);
}
