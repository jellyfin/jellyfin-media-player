//
// Created by Tobias Hieta on 2016-02-16.
//

#ifndef PLEXMEDIAPLAYER_UPDATEMANAGERWIN32_H
#define PLEXMEDIAPLAYER_UPDATEMANAGERWIN32_H

#include "UpdateManager.h"

class UpdateManagerWin32 : public UpdateManager
{
public:
  UpdateManagerWin32(QObject *parent = 0) {};
  ~UpdateManagerWin32() {};

  virtual bool applyUpdate(const QString& version) override;
};

#endif //PLEXMEDIAPLAYER_UPDATEMANAGERWIN32_H
