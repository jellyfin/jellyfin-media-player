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
  explicit InputAppleRemote(QObject* parent = nullptr) : InputBase(parent), m_remoteID(0) { }
  const char* inputName() override { return "AppleRemote"; }
  bool initInput() override;
  
  void remoteButtonEvent(quint8 code, bool pressed);
  
  void addRemote(const QString& name);
  void removeRemote(const QString& name);
  void addRemoteFailed(const QString& error);
  void changeRemoteID(quint32 newID);

private:
  delegate* m_delegate;
  QStringList m_remotes;
  quint32 m_remoteID;
};

#endif
