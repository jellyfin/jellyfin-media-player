#ifndef INPUTLIRC_H
#define INPUTLIRC_H

#include <QLocalSocket>
#include <QSocketNotifier>
#include "input/InputComponent.h"

class InputLIRC : public InputBase
{
  Q_OBJECT
private:
  QLocalSocket* socket;
  QSocketNotifier* socketNotifier;

  bool connectToLIRC();
  void disconnectFromLIRC();
  bool isConnected();

public:
  InputLIRC(QObject* parent);
  ~InputLIRC();

  virtual const char* inputName() { return "LIRC"; }
  virtual bool initInput();

private Q_SLOTS:
  void connected();
  void disconnected();
  void socketerror(QLocalSocket::LocalSocketError socketError);
  void read(int handle);
};

#endif // INPUTLIRC_H
