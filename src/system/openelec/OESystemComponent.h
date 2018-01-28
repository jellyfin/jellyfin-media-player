#ifndef OESYSTEMCOMPONENT_H
#define OESYSTEMCOMPONENT_H

#include <QObject>

#include "ComponentManager.h"

class OESystemComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(OESystemComponent)

public:
  OESystemComponent(QObject *parent = 0);

  virtual bool componentExport() { return true; }
  virtual const char* componentName() { return "oesystem"; }
  virtual bool componentInitialize();

  void updateSectionSettings(const QVariantMap& values);
  bool setHostName(QString name);
};

#endif // OESYSTEMCOMPONENT_H
