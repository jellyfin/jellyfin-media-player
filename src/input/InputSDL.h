//
//  InputSDL.h
//  konvergo
//
//  Created by Lionel CHAZALLON on 16/10/2014.
//
//

#ifndef _INPUT_SDL_
#define _INPUT_SDL_

#include <QThread>
#include <QElapsedTimer>
#include <QByteArray>
#include <SDL.h>

#include "input/InputComponent.h"

typedef QMap<int, SDL_Joystick*> SDLJoystickMap;
typedef SDLJoystickMap::const_iterator SDLJoystickMapIterator;

typedef QMap<int, QElapsedTimer*> SDLTimeStampMap;
typedef QMap<int, QElapsedTimer*>::const_iterator SDLTimeStampMapIterator;

#define SDL_POLL_TIME 50
#define SDL_BUTTON_REPEAT_DELAY 500
#define SDL_BUTTON_REPEAT_RATE 100

///////////////////////////////////////////////////////////////////////////////////////////////////
class InputSDLWorker : public QObject
{
  Q_OBJECT

public:
  explicit InputSDLWorker(QObject* parent) : QObject(parent) {}

public slots:
  void run();
  bool initialize();
  void close();

signals:
  void receivedInput(const QString& source, const QString& keycode, bool pressDown = true);

private:
  void refreshJoystickList();
  QString nameForId(SDL_JoystickID id);

  SDLJoystickMap m_joysticks;

  // map axis to up = true or down = false
  QHash<quint8, bool> m_axisState;
  QString m_lastHat;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class InputSDL : public InputBase
{
  Q_OBJECT
public:
  explicit InputSDL(QObject* parent);
  ~InputSDL() override;
  
  const char* inputName() override { return "SDL"; }
  bool initInput() override;
  
  void close();
private:
  InputSDLWorker* m_sdlworker;
  QThread* m_thread;
  
signals:
  void run();
};

#endif /* _INPUT_SDL_ */
