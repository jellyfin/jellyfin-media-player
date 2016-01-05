
#include "QsLog.h"
#include "InputComponent.h"
#include "settings/SettingsComponent.h"
#include "system/SystemComponent.h"
#include "power/PowerComponent.h"
#include "InputKeyboard.h"
#include "InputSocket.h"

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

///////////////////////////////////////////////////////////////////////////////////////////////////
InputComponent::InputComponent(QObject* parent) : ComponentBase(parent)
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
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputComponent::componentInitialize()
{
  // load our input mappings
  m_mappings->loadMappings();

  addInput(&InputKeyboard::Get());
  addInput(new InputSocket(this));

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

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputComponent::remapInput(const QString &source, const QString &keycode, float amount)
{
  // hide mouse if it's visible.
  SystemComponent::Get().setCursorVisibility(false);

  QString action = m_mappings->mapToAction(source, keycode);
  if (!action.isEmpty())
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
          QLOG_DEBUG() << "Invoking slot" << qPrintable(recvSlot->slot.data());
          QGenericArgument arg0 = QGenericArgument();
          if (recvSlot->hasArguments)
            arg0 = Q_ARG(const QString&, hostArguments);
          QMetaObject::invokeMethod(recvSlot->receiver, recvSlot->slot.data(),
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
      QLOG_DEBUG() << "Sending action:" << action;
      emit receivedAction(action);
    }
  }
  else
  {
    QLOG_WARN() << "Could not map:" << source << keycode << "to any useful action";
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void InputComponent::registerHostCommand(const QString& command, QObject* receiver, const char* slot)
{
  ReceiverSlot* recvSlot = new ReceiverSlot;
  recvSlot->receiver = receiver;
  recvSlot->slot = QMetaObject::normalizedSignature(slot);
  recvSlot->hasArguments = false;

  QLOG_DEBUG() << "Adding host command:" << qPrintable(command) << "mapped to" << qPrintable(QString(receiver->metaObject()->className()) + "::" + recvSlot->slot);

  m_hostCommands.insert(command, recvSlot);

  auto slotWithArgs = QString("%1(QString)").arg(QString::fromLatin1(recvSlot->slot)).toLatin1().data();
  auto slotWithoutArgs = QString("%1()").arg(QString::fromLatin1(recvSlot->slot)).toLatin1().data();
  if (recvSlot->receiver->metaObject()->indexOfMethod(slotWithArgs) != -1)
  {
    QLOG_DEBUG() << "Host command maps to method with an argument.";
    recvSlot->hasArguments = true;
  }
  else if (recvSlot->receiver->metaObject()->indexOfMethod(slotWithoutArgs) != -1)
  {
    QLOG_DEBUG() << "Host command maps to method without arguments.";
  }
  else
  {
    QLOG_ERROR() << "Slot for host command missing, or has incorrect signature!";
  }
}
