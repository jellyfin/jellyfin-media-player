//
// Created by Tobias Hieta on 15/05/15.
//

#ifndef KONVERGO_PLATFORMUTILS_H
#define KONVERGO_PLATFORMUTILS_H

#include <QProcess>

class PlatformUtils
{
public:
  PlatformUtils() { }
  static bool isProcessAlive(qint64 pid);
};


#endif //KONVERGO_PLATFORMUTILS_H
