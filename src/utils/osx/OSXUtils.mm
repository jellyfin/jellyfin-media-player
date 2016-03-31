#include "OSXUtils.h"
#import <Cocoa/Cocoa.h>

/////////////////////////////////////////////////////////////////////////////////////////
void OSXUtils::SetMenuBarVisible(bool visible)
{
  if(visible)
  {
    [[NSApplication sharedApplication]
      setPresentationOptions:   NSApplicationPresentationDefault];
  }
  else
  {
    [[NSApplication sharedApplication]
      setPresentationOptions:   NSApplicationPresentationHideMenuBar |
                                NSApplicationPresentationHideDock];
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
QString OSXUtils::ComputerName()
{
  return QString::fromNSString([[NSHost currentHost] localizedName]);
}

/////////////////////////////////////////////////////////////////////////////////////////
OSStatus OSXUtils::SendAppleEventToSystemProcess(AEEventID eventToSendID)
{
  AEAddressDesc targetDesc;
  static const  ProcessSerialNumber kPSNOfSystemProcess = {0, kSystemProcess };
  AppleEvent    eventReply  = {typeNull, nullptr};
  AppleEvent    eventToSend = {typeNull, nullptr};

  OSStatus status = AECreateDesc(typeProcessSerialNumber,
    &kPSNOfSystemProcess, sizeof(kPSNOfSystemProcess), &targetDesc);

  if (status != noErr)
    return status;

  status = AECreateAppleEvent(kCoreEventClass, eventToSendID,
    &targetDesc, kAutoGenerateReturnID, kAnyTransactionID, &eventToSend);
  AEDisposeDesc(&targetDesc);

  if (status != noErr)
    return status;

  status = AESendMessage(&eventToSend, &eventReply, kAENormalPriority, kAEDefaultTimeout);
  AEDisposeDesc(&eventToSend);

  if (status != noErr)
    return status;

  AEDisposeDesc(&eventReply);

  return status;
}
