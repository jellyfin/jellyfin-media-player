#include "PowerComponentMac.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentMac::doDisableScreensaver()
{
  if (m_assertion == 0)
  {
    CFStringRef why = CFSTR("org.jellyfin.JellyfinDesktop");
    IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleDisplaySleep,
                                kIOPMAssertionLevelOn,
                                why,
                                &m_assertion);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentMac::doEnableScreensaver()
{
  if (m_assertion != 0)
  {
    IOPMAssertionRelease(m_assertion);
    m_assertion = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentMac::PowerOff()
{
  OSErr error = OSXUtils::SendAppleEventToSystemProcess(kAEShutDown);
  if (error == noErr)
    qDebug() <<  "Computer is going to shutdown!";
  else
    qDebug() << "Computer wouldn't shutdown!";

  return (error == noErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentMac::Reboot()
{
  OSErr error = OSXUtils::SendAppleEventToSystemProcess(kAERestart);
  if (error == noErr)
    qDebug() <<  "Computer is going to reboot!";
  else
    qDebug() << "Computer wouldn't reboot!";

  return (error == noErr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentMac::Suspend()
{
  OSErr error = OSXUtils::SendAppleEventToSystemProcess(kAESleep);
  if (error == noErr)
    qDebug() <<  "Computer is going to sleep!";
  else
    qDebug() << "Computer wouldn't sleep!";

  return (error == noErr);
}
