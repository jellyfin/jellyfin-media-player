#include "InputAppleRemote.h"
#include "QsLog.h"

#include "HIDRemote/HIDRemote.h"
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
static QString codeToString(HIDRemoteButtonCode code)
{
  switch (code)
  {
    case kHIDRemoteButtonCodeCenter:
      return INPUT_KEY_SELECT;
    case kHIDRemoteButtonCodeCenterHold:
      return INPUT_KEY_SELECT_LONG;
    case kHIDRemoteButtonCodeDown:
      return INPUT_KEY_DOWN;
    case kHIDRemoteButtonCodeDownHold:
      return INPUT_KEY_DOWN_LONG;
    case kHIDRemoteButtonCodeLeft:
      return INPUT_KEY_LEFT;
    case kHIDRemoteButtonCodeLeftHold:
      return INPUT_KEY_LEFT_LONG;
    case kHIDRemoteButtonCodeRight:
      return INPUT_KEY_RIGHT;
    case kHIDRemoteButtonCodeRightHold:
      return INPUT_KEY_RIGHT_LONG;
    case kHIDRemoteButtonCodeMenu:
      return INPUT_KEY_MENU;
    case kHIDRemoteButtonCodeMenuHold:
      return INPUT_KEY_MENU_LONG;
    case kHIDRemoteButtonCodePlay:
      return INPUT_KEY_PLAY;
    case kHIDRemoteButtonCodePlayHold:
      return INPUT_KEY_PLAY_LONG;
    case kHIDRemoteButtonCodeUp:
      return INPUT_KEY_UP;
    case kHIDRemoteButtonCodeUpHold:
      return INPUT_KEY_UP_LONG;
    default:
      QLOG_WARN() << "could not handle key code:" << code;
  }
  return "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleRemote::remoteButtonEvent(quint8 code, bool pressed, const QString &name)
{
  if (pressed)
  {
    QString codeStr = codeToString((HIDRemoteButtonCode)code);
    if (!codeStr.isEmpty())
    {
      QLOG_DEBUG() << name << "pressed button:" << codeStr;
      emit receivedInput("AppleRemote", codeStr);
    }
  }
}