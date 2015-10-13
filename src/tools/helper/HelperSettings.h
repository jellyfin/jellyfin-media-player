//
// Created by Tobias Hieta on 01/09/15.
//

#ifndef KONVERGO_HELPERSETTINGS_H
#define KONVERGO_HELPERSETTINGS_H

#include <QSettings>
#include "Paths.h"

class HelperSettings : public QSettings
{
public:
  HelperSettings() : QSettings(Paths::dataDir("helper.conf"), QSettings::IniFormat)
  {
  }
};

#endif //KONVERGO_HELPERSETTINGS_H
