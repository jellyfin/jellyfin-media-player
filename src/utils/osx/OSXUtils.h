#ifndef OSXUTILS_H
#define OSXUTILS_H

#include <QString>
#include <ApplicationServices/ApplicationServices.h>

namespace OSXUtils
{
  QString ComputerName();
  OSStatus SendAppleEventToSystemProcess(AEEventID eventToSendID);

  void SetPresentationOptions(unsigned long flags);
  unsigned long GetPresentationOptions();
  unsigned long GetPresentationOptionsForFullscreen(bool hideMenuAndDock);
};

#endif /* OSXUTILS_H */
