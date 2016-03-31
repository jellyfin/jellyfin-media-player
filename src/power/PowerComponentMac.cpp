#include "PowerComponentMac.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentMac::doDisableScreensaver()
{
  if (m_assertion == 0)
  {
    CFStringRef why = CFSTR("tv.plex.player");
    IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
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
    QLOG_DEBUG() <<  "Computer is going to shutdown!";
  else
    QLOG_DEBUG() << "Computer wouldn't shutdown!";

  return (error == noErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentMac::Reboot()
{
  OSErr error = OSXUtils::SendAppleEventToSystemProcess(kAERestart);
  if (error == noErr)
    QLOG_DEBUG() <<  "Computer is going to reboot!";
  else
    QLOG_DEBUG() << "Computer wouldn't reboot!";

  return (error == noErr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PowerComponentMac::Suspend()
{
  OSErr error = OSXUtils::SendAppleEventToSystemProcess(kAESleep);
  if (error == noErr)
    QLOG_DEBUG() <<  "Computer is going to sleep!";
  else
    QLOG_DEBUG() << "Computer wouldn't sleep!";

  return (error == noErr);
}
