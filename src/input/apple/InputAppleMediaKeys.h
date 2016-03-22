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
  InputAppleMediaKeys(QObject* parent = nullptr) : InputBase(parent) { }
  bool initInput();
  const char* inputName() { return "AppleMediaKeys"; }

private:
  void* m_delegate;
};

#endif //KONVERGO_INPUTAPPLEMEDIAKEYS_H
