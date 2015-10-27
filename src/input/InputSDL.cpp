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
    {
      SDL_JoystickClose(m_joysticks[joyid]);
    }

    SDL_Quit();
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

  QHash<int,int> repeats;

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
          return;
          break;

        case SDL_JOYBUTTONDOWN:
        {
          QLOG_DEBUG() << "SDL Got button down for button #" << event.jbutton.button
                       << " on Joystick #" << event.jbutton.which;
          // emit that the button has gone down
          emit receivedInput(nameForId(event.jbutton.which), QString("KEY_BUTTON_%1").arg(event.jbutton.button));
          
          // set up the repeat timer for this button
          QElapsedTimer* repeatTimer = new QElapsedTimer();
          m_buttonTimestamps[event.jbutton.button] = repeatTimer;
          repeatTimer->start();
          // reset the repeat count for this button
          repeats[event.jbutton.button] = 0;
          break;
        }

        case SDL_JOYBUTTONUP:
        {
          if (m_buttonTimestamps[event.jbutton.button])
          {          
            // remove the repeat timer for this button
            delete m_buttonTimestamps[event.jbutton.button];
            m_buttonTimestamps.remove(event.jbutton.button);
          }

          // reset the repeat count
          repeats[event.jbutton.button] = 0;

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

        case SDL_JOYAXISMOTION:
        {
          int deadband = 32768 * 0.15;

          // handle the analog value push
          // keep a dead band of 15%
          if (std::abs(event.jaxis.value) > deadband)
          {
            // normalize the value
            float normalizedvalue;
            QString keyname;
            if (event.jaxis.value > 0)
            {
              normalizedvalue = (float)(event.jaxis.value - deadband) / (float)(32767 - deadband);
              keyname = QString("KEY_AXIS_%1_VAL_UP").arg(event.jaxis.axis);
            }
            else
            {
              normalizedvalue = (float)(std::abs(event.jaxis.value) - deadband) / (float)(32768 - deadband);
              keyname = QString("KEY_AXIS_%1_VAL_DOWN").arg(event.jaxis.axis);
            }

            emit receivedInput(nameForId(event.jaxis.which), keyname, normalizedvalue);
          }

          // handle the Digital conversion of the analog axis
          if (std::abs(event.jaxis.value) > 32768 / 2)
          {
            if (!m_axisState[event.jaxis.axis])
            {
              m_axisState[event.jaxis.axis] = 1;

              if (event.jaxis.value > 0)
                emit receivedInput(nameForId(event.jaxis.which), QString("KEY_AXIS_%1_UP").arg(event.jaxis.axis));
              else
                emit receivedInput(nameForId(event.jaxis.which), QString("KEY_AXIS_%1_DOWN").arg(event.jaxis.axis));

              break;
            }
          }

          m_axisState[event.jaxis.axis] = 0;
          break;
        }
      }
    }

    // check for long press and repeats
    SDLTimeStampMapIterator it = m_buttonTimestamps.constBegin();
    while (it != m_buttonTimestamps.constEnd())
    {
      if (it.value())
      {
        // repeat at a rate of SDL_BUTTON_REPEAT_RATE
        if (repeats[it.key()] > 0 && m_buttonTimestamps[it.key()]->elapsed() > SDL_BUTTON_REPEAT_RATE)
        {
          QLOG_DEBUG() << QString("SDL Got button repeat(%1) for button #").arg(repeats[it.key()]) << event.jbutton.button
                       << " on Joystick #" << event.jbutton.which;
          emit receivedInput(nameForId(event.jbutton.which), QString("KEY_BUTTON_%1").arg(it.key()));

          m_buttonTimestamps[it.key()]->restart();
          it = m_buttonTimestamps.constBegin();

          repeats[it.key()]++;
          continue;
        }
        else
        {
          // only start repeating if SDL_BUTTON_REPEAT_DELAY time has passed
          if (m_buttonTimestamps[it.key()]->elapsed() > SDL_BUTTON_REPEAT_DELAY)
            repeats[it.key()]++;
        }
      }
      else
      {
        continue;
      }

      it++;
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
  m_buttonTimestamps.clear();

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
      m_axisState.resize(SDL_JoystickNumAxes(joystick));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
InputSDL::InputSDL(QObject* parent) : InputBase(parent)
{
  m_thread = new QThread(this);
  m_sdlworker = new InputSDLWorker(NULL);
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
    m_thread->terminate();
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
  QMetaObject::invokeMethod(m_sdlworker, "close");
}
