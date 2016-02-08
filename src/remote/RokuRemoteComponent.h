//
// Created by Tobias Hieta on 02/02/16.
//

#ifndef PLEXMEDIAPLAYER_ROKUREMOTECOMPONENT_H
#define PLEXMEDIAPLAYER_ROKUREMOTECOMPONENT_H

#include "ComponentManager.h"

class RokuRemoteComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(RokuRemoteComponent);

public:
  virtual bool componentInitialize() override { return true; };
  virtual const char* componentName() override { return "rokuremote"; };
  virtual bool componentExport() override { return false; };
};

#endif //PLEXMEDIAPLAYER_ROKUREMOTECOMPONENT_H
