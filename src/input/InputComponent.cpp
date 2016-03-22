#include "QsLog.h"
#include "InputComponent.h"
#include "settings/SettingsComponent.h"
#include "system/SystemComponent.h"
#include "power/PowerComponent.h"
#include "InputKeyboard.h"
#include "InputSocket.h"
#include "InputRoku.h"

#ifdef Q_OS_MAC
#include "apple/InputAppleRemote.h"
#include "apple/InputAppleMediaKeys.h"
#endif

#ifdef HAVE_SDL
#include "InputSDL.h"
#endif

#ifdef HAVE_LIRC
#include "InputLIRC.h"
#endif

#ifdef HAVE_CEC
#include "InputCEC.h"
#endif

#define LONG_HOLD_MSEC 500

///////////////////////////////////////////////////////////////////////////////////////////////////
InputComponent::InputComponent(QObject* parent) : ComponentBase(parent), m_currentActionCount(0)
{
  m_mappings = new InputMapping(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputComponent::addInput(InputBase* base)
{
  if (!base->initInput())
  {
    QLOG_WARN() << "Failed to init input:" << base->inputName();
    return false;
  }
  
  QLOG_INFO() << "Successfully inited input:" << base->inputName();
  m_inputs.push_back(base);

  // we connect to the provider receivedInput signal, then we check if the name
  // needs to be remaped in remapInput and then finally send it out to JS land.
  //
  connect(base, &InputBase::receivedInput, this, &InputComponent::remapInput);


  // for auto-repeating inputs
  //
  m_autoRepeatTimer = new QTimer(this);
  connect(m_autoRepeatTimer, &QTimer::timeout, [=]()
  {
    if (!m_currentAction.isEmpty())
    {
      m_currentActionCount ++;
      emit receivedAction(m_currentAction);
    }

    qint32 multiplier = qMin(5, qMax(1, m_currentActionCount / 5));
    m_autoRepeatTimer->setInterval(100 / multiplier);
  });

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputComponent::componentInitialize()
{
  // load our input mappings
  m_mappings->loadMappings();

  addInput(&InputKeyboard::Get());
  addInput(new InputSocket(this));
  addInput(new InputRoku(this));

#ifdef Q_OS_MAC
  addInput(new InputAppleRemote(this));
  addInput(new InputAppleMediaKeys(this));
#endif
#ifdef HAVE_SDL
  addInput(new InputSDL(this));
#endif
#ifdef HAVE_LIRC
  addInput(new InputLIRC(this));
#endif
#ifdef HAVE_CEC
  if (SettingsComponent::Get().value(SETTINGS_SECTION_CEC, "enable").toBool())
    addInput(new InputCEC(this));
#endif

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputComponent::handleAction(const QString& action, bool autoRepeat)
{
  if (action.startsWith("host:"))
  {
    QStringList argList = action.mid(5).split(" ");
    QString hostCommand = argList.value(0);
    QString hostArguments;

    if (argList.size() > 1)
    {
      argList.pop_front();
      hostArguments = argList.join(" ");
    }

    QLOG_DEBUG() << "Got host command:" << hostCommand << "arguments:" << hostArguments;
    if (m_hostCommands.contains(hostCommand))
    {
      ReceiverSlot* recvSlot = m_hostCommands.value(hostCommand);
      if (recvSlot)
      {
        QLOG_DEBUG() << "Invoking slot" << qPrintable(recvSlot->m_slot.data());
        QGenericArgument arg0 = QGenericArgument();
        if (recvSlot->m_hasArguments)
          arg0 = Q_ARG(const QString&, hostArguments);
        QMetaObject::invokeMethod(recvSlot->m_receiver, recvSlot->m_slot.data(),
                                  Qt::AutoConnection, arg0);
      }
    }
    else
    {
      QLOG_WARN() << "No such host command:" << hostCommand;
    }
  }
  else
  {
    m_currentAction = action;
    emit receivedAction(action);

    if (autoRepeat)
      m_autoRepeatTimer->start(500);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputComponent::remapInput(const QString &source, const QString &keycode, bool pressDown)
{
  QLOG_DEBUG() << "Input received: source:" << source << "keycode:" << keycode << "pressed:" << (pressDown ? "down" : "release");

  if (!pressDown)
  {
    m_autoRepeatTimer->stop();
    m_currentAction.clear();
    m_currentActionCount = 0;

    if (!m_currentLongPressAction.isEmpty())
    {
      if (m_longHoldTimer.elapsed() > LONG_HOLD_MSEC)
        handleAction(m_currentLongPressAction.value("long").toString(), false);
      else
        handleAction(m_currentLongPressAction.value("short").toString(), false);

      m_currentLongPressAction.clear();
    }

    return;
  }

  // hide mouse if it's visible.
  SystemComponent::Get().setCursorVisibility(false);

  QVariant action = m_mappings->mapToAction(source, keycode);
  if (action.isNull())
  {
    QLOG_WARN() << "Could not map:" << source << keycode << "to any useful action";
    return;
  }

  if (action.type() == QVariant::String)
  {
    handleAction(action.toString());
  }
  else if (action.type() == QVariant::Map)
  {
    QVariantMap map = action.toMap();
    if (map.contains("long"))
    {
      m_longHoldTimer.start();
      m_currentLongPressAction = map;
    }
    else if (map.contains("short"))
    {
      handleAction(map.value("short").toString());
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputComponent::registerHostCommand(const QString& command, QObject* receiver, const char* slot)
{
  auto  recvSlot = new ReceiverSlot;
  recvSlot->m_receiver = receiver;
  recvSlot->m_slot = QMetaObject::normalizedSignature(slot);
  recvSlot->m_hasArguments = false;

  QLOG_DEBUG() << "Adding host command:" << qPrintable(command) << "mapped to"
               << qPrintable(QString(receiver->metaObject()->className()) + "::" + recvSlot->m_slot);

  m_hostCommands.insert(command, recvSlot);

  auto slotWithArgs = QString("%1(QString)").arg(QString::fromLatin1(recvSlot->m_slot)).toLatin1();
  auto slotWithoutArgs = QString("%1()").arg(QString::fromLatin1(recvSlot->m_slot)).toLatin1();
  if (recvSlot->m_receiver->metaObject()->indexOfMethod(slotWithArgs.data()) != -1)
  {
    QLOG_DEBUG() << "Host command maps to method with an argument.";
    recvSlot->m_hasArguments = true;
  }
  else if (recvSlot->m_receiver->metaObject()->indexOfMethod(slotWithoutArgs.data()) != -1)
  {
    QLOG_DEBUG() << "Host command maps to method without arguments.";
  }
  else
  {
    QLOG_ERROR() << "Slot for host command missing, or has incorrect signature!";
  }
}
