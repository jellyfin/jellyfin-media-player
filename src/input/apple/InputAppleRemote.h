#ifndef __INPUT_APPLE_REMOTE_H__
#define __INPUT_APPLE_REMOTE_H__

#include <QObject>
#include <QString>
#include "input/InputComponent.h"


#ifdef __OBJC__
@class AppleRemoteDelegate;
typedef AppleRemoteDelegate delegate;
#else
typedef void delegate;
#endif

class InputAppleRemote : public InputBase
{
public:
  InputAppleRemote(QObject* parent = 0) : InputBase(parent) { }
  virtual const char* inputName() { return "AppleRemote"; }
  virtual bool initInput();
  
  void remoteButtonEvent(quint8 code, bool pressed, const QString& name);
  
  void addRemote(const QString& name);
  void removeRemote(const QString& name);
  void addRemoteFailed(const QString& error);
  
private:
  delegate* m_delegate;
  QStringList m_remotes;
  
};

#endif