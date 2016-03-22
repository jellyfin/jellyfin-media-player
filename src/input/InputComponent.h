#ifndef INPUTADAPTER_H
#define INPUTADAPTER_H

#include <QThread>
#include <QVariantMap>
#include <QTimer>
#include <QTime>
#include "ComponentManager.h"
#include "InputMapping.h"

class InputBase : public QObject
{
  Q_OBJECT
public:
  explicit InputBase(QObject* parent = nullptr) : QObject(parent) { }
  virtual bool initInput() = 0;
  virtual const char* inputName() = 0;
  
signals:
  void receivedInput(const QString& source, const QString& keycode, bool pressDown = true);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// well known keys
///////////////////////////////////////////////////////////////////////////////////////////////////

#define INPUT_KEY_LEFT      "KEY_LEFT"
#define INPUT_KEY_RIGHT     "KEY_RIGHT"
#define INPUT_KEY_UP        "KEY_UP"
#define INPUT_KEY_DOWN      "KEY_DOWN"
#define INPUT_KEY_SELECT    "KEY_SELECT"
#define INPUT_KEY_MENU      "KEY_MENU"
#define INPUT_KEY_PLAY      "KEY_PLAY"
#define INPUT_KEY_PAUSE     "KEY_PAUSE"
#define INPUT_KEY_STOP      "KEY_STOP"
#define INPUT_KEY_DOWN      "KEY_DOWN"
#define INPUT_KEY_BACK      "KEY_BACK"
#define INPUT_KEY_SEEKFWD   "KEY_SEEKFWD"
#define INPUT_KEY_SEEKBCK   "KEY_SEEKBCK"
#define INPUT_KEY_SUBTITLES "KEY_SUBTITLES"
#define INPUT_KEY_INFO      "KEY_INFO"
#define INPUT_KEY_NEXT      "KEY_NEXT"
#define INPUT_KEY_PREV      "KEY_PREV"
#define INPUT_KEY_RED       "KEY_RED"
#define INPUT_KEY_GREEN     "KEY_GREEN"
#define INPUT_KEY_BLUE      "KEY_BLUE"
#define INPUT_KEY_YELLOW    "KEY_YELLOW"
#define INPUT_KEY_HOME      "KEY_HOME"
#define INPUT_KEY_0         "KEY_NUMERIC_0"
#define INPUT_KEY_1         "KEY_NUMERIC_1"
#define INPUT_KEY_2         "KEY_NUMERIC_2"
#define INPUT_KEY_3         "KEY_NUMERIC_3"
#define INPUT_KEY_4         "KEY_NUMERIC_4"
#define INPUT_KEY_5         "KEY_NUMERIC_5"
#define INPUT_KEY_6         "KEY_NUMERIC_6"
#define INPUT_KEY_7         "KEY_NUMERIC_7"
#define INPUT_KEY_8         "KEY_NUMERIC_8"
#define INPUT_KEY_9         "KEY_NUMERIC_9"
#define INPUT_KEY_GUIDE     "KEY_GUIDE"

#define INPUT_KEY_LEFT_LONG    "KEY_LEFT_LONG"
#define INPUT_KEY_RIGHT_LONG   "KEY_RIGHT_LONG"
#define INPUT_KEY_UP_LONG      "KEY_UP_LONG"
#define INPUT_KEY_DOWN_LONG    "KEY_DOWN_LONG"
#define INPUT_KEY_SELECT_LONG  "KEY_SELECT_LONG"
#define INPUT_KEY_MENU_LONG    "KEY_MENU_LONG"
#define INPUT_KEY_PLAY_LONG    "KEY_PLAY_LONG"
#define INPUT_KEY_DOWN_LONG    "KEY_DOWN_LONG"


struct ReceiverSlot
{
  QObject* m_receiver;
  QByteArray m_slot;
  bool m_hasArguments;
};

class InputComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(InputComponent);

public:
  const char* componentName() override { return "input"; }
  bool componentExport() override { return true; }
  bool componentInitialize() override;

  void registerHostCommand(const QString& command, QObject* receiver, const char* slot);

signals:
  void receivedAction(const QString& action);

private Q_SLOTS:
  void remapInput(const QString& source, const QString& keycode, bool pressDown = true);
  
private:
  explicit InputComponent(QObject *parent = nullptr);
  bool addInput(InputBase* base);
  void handleAction(const QString& action, bool autoRepeat = true);

  QHash<QString, ReceiverSlot*> m_hostCommands;
  QList<InputBase*> m_inputs;
  InputMapping* m_mappings;
  QTimer* m_autoRepeatTimer;
  QVariantMap m_currentLongPressAction;
  qint32 m_currentActionCount;

  QTime m_longHoldTimer;
  QString m_currentAction;
};

#endif // INPUTADAPTER_H
