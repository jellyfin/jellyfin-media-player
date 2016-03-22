//
// Created by Tobias Hieta on 21/08/15.
//

#ifndef KONVERGO_INPUTAPPLEMEDIAKEYS_H
#define KONVERGO_INPUTAPPLEMEDIAKEYS_H

#include "input/InputComponent.h"

class InputAppleMediaKeys : public InputBase
{
  Q_OBJECT
public:
  explicit InputAppleMediaKeys(QObject* parent = nullptr) : InputBase(parent) { }
  bool initInput() override;
  const char* inputName() override { return "AppleMediaKeys"; }

private:
  void* m_delegate;
};

#endif //KONVERGO_INPUTAPPLEMEDIAKEYS_H
