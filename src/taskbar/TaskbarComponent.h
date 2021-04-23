#ifndef TASKBARCOMPONENT_H
#define TASKBARCOMPONENT_H

#include <QsLog.h>
#include <QQuickWindow>
#include "ComponentManager.h"

class TaskbarComponent : public ComponentBase
{
  Q_OBJECT
public:
  static TaskbarComponent& Get();

  explicit TaskbarComponent(QObject* parent = nullptr): ComponentBase(parent) {}

  const char* componentName() override { return "taskbar"; }
  bool componentExport() override { return true; }
  bool componentInitialize() override { return true; }
  void componentPostInitialize() override {}

  virtual void setWindow(QQuickWindow* window);

protected:
  QQuickWindow* m_window;
};

#endif // TASKBARCOMPONENT_H
