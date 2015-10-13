//
// Created by Tobias Hieta on 09/06/15.
//

#ifndef KONVERGO_INPUTKEYBOARD_H
#define KONVERGO_INPUTKEYBOARD_H

#include <QKeySequence>
#include "InputComponent.h"
#include "QsLog.h"

class InputKeyboard : public InputBase
{
  Q_OBJECT
  DEFINE_SINGLETON(InputKeyboard);

public:
  virtual bool initInput() { return true; }
  virtual const char* inputName() { return "Keyboard"; }

  void keyPress(const QKeySequence& sequence)
  {
    QLOG_DEBUG() << "Input:" << sequence.toString();
    emit receivedInput("Keyboard", sequence.toString());
  }

private:
  explicit InputKeyboard(QObject* parent = 0) : InputBase(parent) {}
};

#endif //KONVERGO_INPUTKEYBOARD_H
