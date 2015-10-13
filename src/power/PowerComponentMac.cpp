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
