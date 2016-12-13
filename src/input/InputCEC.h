#ifndef INPUTCEC_H
#define INPUTCEC_H

#include <QMutex>
#include <QTimer>
#include "input/InputComponent.h"
#include <libcec/cec.h>

using namespace CEC;

#define CEC_LONGPRESS_DURATION 1000  // duration after keypress is considerered as long

#define CEC_INPUT_NAME "CEC"

class InputCECWorker;

///////////////////////////////////////////////////////////////////////////////////////////////////
class InputCEC : public InputBase
{
public:
  explicit InputCEC(QObject* parent);
  ~InputCEC();

  const char* inputName() override { return CEC_INPUT_NAME; }
  bool initInput() override;


private:
  QThread* m_cecThread;
  InputCECWorker* m_cecWorker;
};

/////////////////////////////////////////////////////////////////////////////////////////
class InputCECWorker : public QObject
{
Q_OBJECT
public:
  explicit InputCECWorker(QObject* parent = nullptr) : QObject(parent), m_adapter(nullptr), m_adapterPort("")
  {
  }

  Q_SLOT bool init();
  Q_SIGNAL void receivedInput(const QString& source, const QString& keycode, InputBase::InputkeyState keyState);
  Q_SLOT void closeCec();

public slots:
  void checkAdapter();

private:
  bool openAdapter();
  void closeAdapter();

  QString getCommandString(cec_user_control_code code);
  void sendReceivedInput(const QString& source, const QString& keycode, InputBase::InputkeyState keyState);
  QString getCommandParamsList(cec_command command);

  // libcec callbacks
  static int CecLogMessage(void* cbParam, const cec_log_message message);
  static int CecCommand(void* cbParam, const cec_command command);
  static int CecAlert(void* cbParam, const libcec_alert type, const libcec_parameter param);

  libcec_configuration m_configuration;
  ICECCallbacks m_callbacks;
  ICECAdapter* m_adapter;
  QString m_adapterPort;
  QTimer* m_timer;
  bool m_verboseLogging;
};

#endif // INPUTCEC_H
