#ifndef OSXUTILS_H
#define OSXUTILS_H

#include <QString>
#include <ApplicationServices/ApplicationServices.h>

namespace OSXUtils
{
  void SetMenuBarVisible(bool visible);
  QString ComputerName();
  OSStatus SendAppleEventToSystemProcess(AEEventID eventToSendID);
};

#endif /* OSXUTILS_H */
