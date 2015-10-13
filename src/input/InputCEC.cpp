
#include "QsLog.h"
#include "InputCEC.h"
#include "settings/SettingsComponent.h"

static QMap<int, QString> cecKeyMap   { \
                                      { CEC_USER_CONTROL_CODE_SELECT , INPUT_KEY_SELECT } , \
                                      { CEC_USER_CONTROL_CODE_UP , INPUT_KEY_UP } , \
                                      { CEC_USER_CONTROL_CODE_DOWN , INPUT_KEY_DOWN } , \
                                      { CEC_USER_CONTROL_CODE_LEFT , INPUT_KEY_LEFT } , \
                                      { CEC_USER_CONTROL_CODE_RIGHT , INPUT_KEY_RIGHT } , \
                                      { CEC_USER_CONTROL_CODE_SETUP_MENU , INPUT_KEY_MENU } , \
                                      { CEC_USER_CONTROL_CODE_PLAY , INPUT_KEY_PLAY } , \
                                      { CEC_USER_CONTROL_CODE_PAUSE , INPUT_KEY_PAUSE } , \
                                      { CEC_USER_CONTROL_CODE_STOP , INPUT_KEY_STOP } , \
                                      { CEC_USER_CONTROL_CODE_EXIT , INPUT_KEY_BACK } , \
                                      { CEC_USER_CONTROL_CODE_FAST_FORWARD , INPUT_KEY_SEEKFWD } , \
                                      { CEC_USER_CONTROL_CODE_REWIND , INPUT_KEY_SEEKBCK } , \
                                      { CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION , INPUT_KEY_INFO } , \
                                      { CEC_USER_CONTROL_CODE_FORWARD , INPUT_KEY_NEXT } , \
                                      { CEC_USER_CONTROL_CODE_BACKWARD , INPUT_KEY_PREV } , \
                                      { CEC_USER_CONTROL_CODE_F1_BLUE , INPUT_KEY_BLUE } , \
                                      { CEC_USER_CONTROL_CODE_F2_RED , INPUT_KEY_RED } , \
                                      { CEC_USER_CONTROL_CODE_F3_GREEN , INPUT_KEY_GREEN } , \
                                      { CEC_USER_CONTROL_CODE_F4_YELLOW , INPUT_KEY_YELLOW } , \
                                      { CEC_USER_CONTROL_CODE_SUB_PICTURE, INPUT_KEY_SUBTITLES } , \
                                      { CEC_USER_CONTROL_CODE_ROOT_MENU, INPUT_KEY_HOME }, \
                                      { CEC_USER_CONTROL_CODE_NUMBER0, INPUT_KEY_0 } ,  \
                                      { CEC_USER_CONTROL_CODE_NUMBER1, INPUT_KEY_1 } , \
                                      { CEC_USER_CONTROL_CODE_NUMBER2, INPUT_KEY_2 } , \
                                      { CEC_USER_CONTROL_CODE_NUMBER3, INPUT_KEY_3 } , \
                                      { CEC_USER_CONTROL_CODE_NUMBER4, INPUT_KEY_4 } , \
                                      { CEC_USER_CONTROL_CODE_NUMBER5, INPUT_KEY_5 } , \
                                      { CEC_USER_CONTROL_CODE_NUMBER6, INPUT_KEY_6 } , \
                                      { CEC_USER_CONTROL_CODE_NUMBER7, INPUT_KEY_7 } , \
                                      { CEC_USER_CONTROL_CODE_NUMBER8, INPUT_KEY_8 } , \
                                      { CEC_USER_CONTROL_CODE_NUMBER9, INPUT_KEY_9 } , \
                                      { CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE, INPUT_KEY_GUIDE } , \
                                      };

//////////////////////////////////////////////////////////////////////////////////////////////////
InputCEC::InputCEC(QObject *parent) : InputBase(parent)
{
  m_configuration.Clear();
  m_callbacks.Clear();
  m_adapter = NULL;
  m_verboseLogging = false;

  // setup adapter check timer
  m_timer.setInterval(1000);
  m_timer.setSingleShot(false);
  connect(&m_timer, &QTimer::timeout, this, &InputCEC::checkAdapter);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool InputCEC::initInput()
{
  m_configuration.Clear();
  m_callbacks.Clear();

  m_verboseLogging = SettingsComponent::Get().value(SETTINGS_SECTION_CEC, "verbose_logging").toBool();

  m_configuration.clientVersion = LIBCEC_VERSION_CURRENT;
  strcpy(m_configuration.strDeviceName, "PlexMPlayer");
  m_configuration.bActivateSource = 0;
  m_callbacks.CBCecLogMessage = &CecLogMessage;
  m_callbacks.CBCecKeyPress = &CecKeyPress;
  m_callbacks.CBCecCommand = &CecCommand;
  m_callbacks.CBCecAlert = &CecAlert;
  m_configuration.callbackParam = this;
  m_configuration.callbacks = &m_callbacks;
  m_configuration.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
  m_configuration.bAutodetectAddress =  CEC_DEFAULT_SETTING_AUTODETECT_ADDRESS;
  m_configuration.iPhysicalAddress = CEC_PHYSICAL_ADDRESS_TV;
  m_configuration.baseDevice = CECDEVICE_AUDIOSYSTEM;
  m_configuration.bActivateSource = 1;

  m_configuration.iHDMIPort = (quint8)SettingsComponent::Get().value(SETTINGS_SECTION_CEC, "hdmiport").toInt();

  // open libcec
  m_adapter = (ICECAdapter*)CECInitialise(&m_configuration);
  if (!m_adapter)
  {
    QLOG_ERROR() << "Unable to initialize libCEC.";
    return false;
  }

  QLOG_INFO() << "libCEC was successfully initialized, found version"
              << m_configuration.serverVersion;

  // init video on targets that need this
  m_adapter->InitVideoStandalone();

  // start the adapter check timer
  m_timer.start();
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
InputCEC::~InputCEC()
{
  m_timer.stop();
  close();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void InputCEC::close()
{
  if (m_adapter)
  {
    QLOG_DEBUG() << "Closing libCEC.";
    closeAdapter();
    CECDestroy(m_adapter);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool InputCEC::openAdapter()
{
  bool ret = false;
  m_lock.lock();

  // try to find devices
  cec_adapter devices[10];
  int devicesCount = m_adapter->FindAdapters(devices, 10, NULL);
  if (devicesCount > 0)
  {
    // list devices
    QLOG_INFO() << "libCEC found" << devicesCount << "CEC adapters.";

    // open first adapter
    m_adapterPort = devices[0].comm;
    if (m_adapter->Open(m_adapterPort.toStdString().c_str()))
    {
      QLOG_INFO() << "Device " << devices[0].path << "was successfully openned";
      ret = true;
    }
    else
    {
      QLOG_ERROR() << "Opening device" << devices[0].path << "failed";
      ret = false;
    }
  }

  m_lock.unlock();
  return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void InputCEC::closeAdapter()
{
  m_lock.lock();

  if (m_adapter)
    m_adapter->Close();

  m_adapterPort = "";
  m_lock.unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void InputCEC::reopenAdapter()
{
  closeAdapter();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputCEC::checkAdapter()
{
  if (m_adapterPort.isEmpty())
  {
    openAdapter();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputCEC::sendReceivedInput(const QString &source, const QString &keycode, float amount)
{
  emit receivedInput(source, keycode, amount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString InputCEC::getCommandString(cec_user_control_code code, unsigned int duration)
{
  QString key = cecKeyMap[code];

  if (!key.isEmpty() && (duration > CEC_LONGPRESS_DURATION))
  {
    key += "_LONG";
  }

  return key;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int InputCEC::CecLogMessage(void* cbParam, const cec_log_message message)
{
  InputCEC *cec = (InputCEC*)cbParam;
  switch (message.level)
  {
    case CEC_LOG_ERROR:
      QLOG_ERROR() << "libCEC ERROR:" << message.message;
      break;

    case CEC_LOG_WARNING:
      QLOG_WARN() << "libCEC WARNING:" << message.message;
      break;

    case CEC_LOG_NOTICE:
      QLOG_INFO() << "libCEC NOTICE:" << message.message;
      break;

    case CEC_LOG_DEBUG:
      if (cec->m_verboseLogging)
      {
        QLOG_DEBUG() << "libCEC DEBUG:" << message.message;
      }
      break;

    case CEC_LOG_TRAFFIC:
      break;

    default:
      break;
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int InputCEC::CecKeyPress(void *cbParam, const cec_keypress key)
{
  InputCEC *cec = (InputCEC*)cbParam;
  if (cec && key.duration == 0)
  {
    QString cmdString = cec->getCommandString(key.keycode, key.duration);

    if (!cmdString.isEmpty())
      cec->sendReceivedInput(CEC_INPUT_NAME, cmdString);
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int InputCEC::CecCommand(void *cbParam, const cec_command command)
{
  InputCEC *cec = (InputCEC*)cbParam;

  switch(command.opcode)
  {
    case CEC_OPCODE_PLAY:
      cec->sendReceivedInput(CEC_INPUT_NAME, INPUT_KEY_PLAY);
      break;

    case CEC_OPCODE_DECK_CONTROL:
      if (command.parameters.size)
      {
        switch(command.parameters[0])
        {
          case CEC_DECK_CONTROL_MODE_SKIP_FORWARD_WIND:
            cec->sendReceivedInput(CEC_INPUT_NAME, INPUT_KEY_SEEKFWD);
            break;

          case CEC_DECK_CONTROL_MODE_SKIP_REVERSE_REWIND:
            cec->sendReceivedInput(CEC_INPUT_NAME, INPUT_KEY_SEEKBCK);
            break;

          case CEC_DECK_CONTROL_MODE_STOP:
            cec->sendReceivedInput(CEC_INPUT_NAME, INPUT_KEY_STOP);
            break;

          default:
            break;
        }
      }
      break;

    case CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN:
      if (command.parameters.size)
      {
        switch(command.parameters[0])
        {
          // samsung Return key
          case CEC_USER_CONTROL_CODE_AN_RETURN:
            cec->sendReceivedInput(CEC_INPUT_NAME, INPUT_KEY_BACK);
            break;

          default:
            break;
        }
      }
      break;

    case CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP:
    case CEC_OPCODE_USER_CONTROL_PRESSED:
      // ignore those commands as they are handled in CecKeypress
      break;

    case CEC_OPCODE_GIVE_OSD_NAME:
    case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
      // ignore those known commands (only pollng from TV)
      break;

    default:
      QLOG_DEBUG() << "Unhandled CEC command " << command.opcode;
      if (command.parameters.size)
      {
        for (int i=0; i<command.parameters.size; i++)
          QLOG_DEBUG() << command.parameters.size << QString("parameters -> [%1]= %2").arg(i).arg(command.parameters[i]);
      }
      break;
  }

  return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int InputCEC::CecAlert(void *cbParam, const libcec_alert type, const libcec_parameter param)
{
  bool reopen = false;

  switch (type)
  {
    case CEC_ALERT_SERVICE_DEVICE:
      QLOG_ERROR() << "libCEC : Alert CEC_ALERT_SERVICE_DEVICE";
      break;

    case CEC_ALERT_CONNECTION_LOST:
      QLOG_ERROR() << "libCEC : Alert CEC_ALERT_CONNECTION_LOST";
      break;

    case CEC_ALERT_PERMISSION_ERROR:
      QLOG_ERROR() << "libCEC : Alert CEC_ALERT_PERMISSION_ERROR";
      reopen = true;
      break;

    case CEC_ALERT_PORT_BUSY:
      QLOG_ERROR() << "libCEC : Alert CEC_ALERT_PORT_BUSY";
      reopen = true;
      break;

    default:
      break;
  }

  if (reopen)
  {
    QLOG_DEBUG() << "libCEC : Reopenning adapter";
    InputCEC *cec = (InputCEC*)cbParam;
    if (cec)
      cec->reopenAdapter();
  }

  return 0;
}





