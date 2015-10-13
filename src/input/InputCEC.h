#ifndef INPUTCEC_H
#define INPUTCEC_H

#include <QMutex>
#include <QTimer>
#include "input/InputComponent.h"
#include <cec.h>

using namespace CEC;

#define CEC_LONGPRESS_DURATION 1000  // duration after keypress is considerered as long

#define CEC_INPUT_NAME "CEC"

///////////////////////////////////////////////////////////////////////////////////////////////////
class InputCEC : public InputBase
{
public:
  InputCEC(QObject* parent);
  ~InputCEC();

  virtual const char* inputName() { return CEC_INPUT_NAME; }
  virtual bool initInput();
  void close();

private:
  libcec_configuration m_configuration;
  ICECCallbacks m_callbacks;
  ICECAdapter*  m_adapter;
  QString m_adapterPort;
  QMutex m_lock;
  QTimer m_timer;
  bool m_verboseLogging;

  bool openAdapter();
  void closeAdapter();
  QString getCommandString(cec_user_control_code code, unsigned int duration);
  void sendReceivedInput(const QString& source, const QString& keycode, float amount = 1.0);


  // libcec callbacks
  static int CecLogMessage(void *cbParam, const cec_log_message message);
  static int CecKeyPress(void *cbParam, const cec_keypress key);
  static int CecCommand(void *cbParam, const cec_command command);
  static int CecAlert(void *cbParam, const libcec_alert type, const libcec_parameter param);

public:
  void reopenAdapter();

public slots:
  void checkAdapter();

};

#endif // INPUTCEC_H
