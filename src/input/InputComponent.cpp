
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
      QLOG_DEBUG() << "Handling host command:" << action.mid(5);
      emit receivedHostCommand(action.mid(5));
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
