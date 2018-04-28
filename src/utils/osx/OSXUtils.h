#ifndef OSXUTILS_H
#define OSXUTILS_H

#include <QString>
#include <ApplicationServices/ApplicationServices.h>

namespace OSXUtils
{
  QString ComputerName();
  OSStatus SendAppleEventToSystemProcess(AEEventID eventToSendID);

  void SetCursorVisible(bool visible);
};

#endif /* OSXUTILS_H */
