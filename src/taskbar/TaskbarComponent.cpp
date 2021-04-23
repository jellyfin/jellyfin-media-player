#include "TaskbarComponent.h"

#if defined(Q_OS_WIN32)
#include "TaskbarComponentWin.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////
TaskbarComponent& TaskbarComponent::Get()
{
#if defined(Q_OS_WIN32)
  static TaskbarComponentWin instance;
  return instance;
#else
  QLOG_WARN() << "Could not find a taskbar component matching this platform. Taskbar functions disabled.";

  static TaskbarComponent instance;
  return instance;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponent::setWindow(QQuickWindow* window)
{
  m_window = window;
}
