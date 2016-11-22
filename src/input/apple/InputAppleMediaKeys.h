//
// Created by Tobias Hieta on 21/08/15.
//

#ifndef KONVERGO_INPUTAPPLEMEDIAKEYS_H
#define KONVERGO_INPUTAPPLEMEDIAKEYS_H

#include "input/InputComponent.h"
#include "player/PlayerComponent.h"

class InputAppleMediaKeys : public InputBase
{
  Q_OBJECT
public:
  explicit InputAppleMediaKeys(QObject* parent = nullptr) : InputBase(parent) { }
  bool initInput() override;
  const char* inputName() override { return "AppleMediaKeys"; }

private:
  void* m_delegate;
  void handleStateChanged(PlayerComponent::State newState, PlayerComponent::State oldState);
  void handlePositionUpdate(quint64 position);
  void handleUpdateDuration(qint64 duration);

  typedef void (*SetNowPlayingVisibilityFunc)(void* origin, int visibility);
  typedef void* (*GetLocalOriginFunc)(void);
  typedef void (*SetCanBeNowPlayingApplicationFunc)(int);
  SetNowPlayingVisibilityFunc SetNowPlayingVisibility;
  GetLocalOriginFunc GetLocalOrigin;
  SetCanBeNowPlayingApplicationFunc SetCanBeNowPlayingApplication;

  bool m_pendingUpdate;
  quint64 m_currentTime;
};

#endif //KONVERGO_INPUTAPPLEMEDIAKEYS_H
