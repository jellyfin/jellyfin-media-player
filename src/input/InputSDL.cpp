//
//  InputSDL.cpp
//  konvergo
//
//  Created by Lionel CHAZALLON on 16/10/2014.
//
//

#include <QKeyEvent>
#include "InputSDL.h"
#include "QsLog.h"

#include <climits>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputSDLWorker::initialize()
{
  // close if it was previously inited
  close();

  // init SDL
  if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
  {
    QLOG_ERROR() << "SDL failed to initialize : " << SDL_GetError();
    return false;
  }

  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
  SDL_JoystickEventState(SDL_ENABLE);

  refreshJoystickList();

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void InputSDLWorker::close()
{
  if (SDL_WasInit(SDL_INIT_JOYSTICK))
  {
    QLOG_INFO() << "SDL is closing.";

    // we need to close all the openned joysticks here and then exit the thread
    for (int joyid = 0; joyid < m_joysticks.size(); joyid++)
      SDL_JoystickClose(m_joysticks[joyid]);

    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString InputSDLWorker::nameForId(SDL_JoystickID id)
{
  if (m_joysticks.contains(id))
    return SDL_JoystickName(m_joysticks[id]);

  return "unknown joystick";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputSDLWorker::run()
{
  QElapsedTimer polltimer;

  while (true)
  {
    SDL_Event event;

    // startpoint for polling loop
    polltimer.restart();

    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
          SDL_Quit();
          return;
          break;

        case SDL_JOYBUTTONDOWN:
        {
          emit receivedInput(nameForId(event.jbutton.which), QString("KEY_BUTTON_%1").arg(event.jbutton.button), InputBase::KeyDown);
          break;
        }

        case SDL_JOYBUTTONUP:
        {
          emit receivedInput(nameForId(event.jbutton.which), QString("KEY_BUTTON_%1").arg(event.jbutton.button), InputBase::KeyUp);
          break;
        }

        case SDL_JOYDEVICEADDED:
        {
          QLOG_INFO() << "SDL detected device was added.";
          refreshJoystickList();
          break;
        }

        case SDL_JOYDEVICEREMOVED:
        {
          QLOG_INFO() << "SDL detected device was removed.";
          refreshJoystickList();
          break;
        }

        case SDL_JOYHATMOTION:
        {

          QString hatName("KEY_HAT_");
          bool pressed = true;

          switch (event.jhat.value)
          {
            case SDL_HAT_CENTERED:
              if (!m_lastHat.isEmpty())
                hatName = m_lastHat;
              else
                hatName += "CENTERED";
              pressed = false;
              break;
            case SDL_HAT_UP:
              hatName += "UP";
              break;
            case SDL_HAT_DOWN:
              hatName += "DOWN";
              break;
            case SDL_HAT_RIGHT:
              hatName += "RIGHT";
              break;
            case SDL_HAT_LEFT:
              hatName += "LEFT";
              break;
            default:
              break;
          }

          m_lastHat = hatName;

          emit receivedInput(nameForId(event.jhat.which), hatName, pressed ? InputBase::KeyDown : InputBase::KeyUp);

          break;
        }

        case SDL_JOYAXISMOTION:
        {
          auto axis = event.jaxis.axis;
          auto value = event.jaxis.value;

          QLOG_DEBUG() << "JoyAxisMotion:" << axis << value;

          // handle the Digital conversion of the analog axis
          if (std::abs(value) > 32768 / 2)
          {
            bool up = value < 0;
            if (!m_axisState.contains(axis))
            {
              emit receivedInput(nameForId(event.jaxis.which), QString("KEY_AXIS_%1_%2").arg(axis).arg(up ? "UP" : "DOWN"), InputBase::KeyDown);
              m_axisState.insert(axis, up);
            }
            else if (m_axisState.value(axis) != up)
            {
              emit receivedInput(nameForId(event.jaxis.which), QString("KEY_AXIS_%1_%2").arg(axis).arg(m_axisState.value(axis) ? "UP" : "DOWN"), InputBase::KeyUp);
              m_axisState.remove(axis);
            }
          }
          else if (std::abs(value) < 10000 && m_axisState.contains(axis)) // back to the center.
          {
            emit receivedInput(nameForId(event.jaxis.which), QString("KEY_AXIS_%1_%2").arg(axis).arg(m_axisState.value(axis) ? "UP" : "DOWN"), InputBase::KeyUp);
            m_axisState.remove(axis);
          }
          break;
        }
        default:
        {
          QLOG_WARN() << "Unhandled SDL event:" << event.type;
          break;
        }
      }
    }

    // make the poll time fixed to SDL_POLL_TIME
    if (polltimer.elapsed() < SDL_POLL_TIME)
      QThread::msleep(SDL_POLL_TIME - polltimer.elapsed());
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void InputSDLWorker::refreshJoystickList()
{
  // close all openned joysticks
  SDLJoystickMapIterator it = m_joysticks.constBegin();
  while (it != m_joysticks.constEnd())
  {
    if (SDL_JoystickGetAttached(m_joysticks[it.key()]))
      SDL_JoystickClose(m_joysticks[it.key()]);
    it++;
  }

  m_joysticks.clear();

  // list all the joysticks and open them
  int numJoysticks = SDL_NumJoysticks();
  QLOG_INFO() << "SDL found " << numJoysticks << " joysticks";

  for (int joyid = 0; joyid < numJoysticks; joyid++)
  {
    SDL_Joystick* joystick = SDL_JoystickOpen(joyid);

    if (joystick)
    {
      int instanceid = SDL_JoystickInstanceID(joystick);
      QLOG_INFO() << "JoyStick #" << instanceid << " is " << SDL_JoystickName(joystick) << " with "
                  << SDL_JoystickNumButtons(joystick) << " buttons and " << SDL_JoystickNumAxes(joystick)
                  << "axes";
      m_joysticks[instanceid] = joystick;
      m_axisState.clear();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
InputSDL::InputSDL(QObject* parent) : InputBase(parent)
{
  m_thread = new QThread(this);
  m_sdlworker = new InputSDLWorker(nullptr);
  m_sdlworker->moveToThread(m_thread);

  connect(this, &InputSDL::run, m_sdlworker, &InputSDLWorker::run);
  connect(m_sdlworker, &InputSDLWorker::receivedInput, this, &InputBase::receivedInput);
  m_thread->start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
InputSDL::~InputSDL()
{
  close();
  
  if (m_thread->isRunning())
  {
    m_thread->exit(0);
    m_thread->wait();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool InputSDL::initInput()
{
  bool retValue;
  QMetaObject::invokeMethod(m_sdlworker, "initialize", Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(bool, retValue));

  if (retValue)
    emit run();

  return retValue;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void InputSDL::close()
{
  QMetaObject::invokeMethod(m_sdlworker, "close", Qt::DirectConnection);
}
