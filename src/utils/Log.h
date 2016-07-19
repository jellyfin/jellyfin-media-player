//
// Created by Tobias Hieta on 07/03/16.
//

#ifndef PLEXMEDIAPLAYER_LOG_H
#define PLEXMEDIAPLAYER_LOG_H

#include <QString>

namespace Log
{
  void Init();
  void Uninit();
  void UpdateLogLevel();
  void CensorAuthTokens(QString& msg);
}

#endif //PLEXMEDIAPLAYER_LOG_H
