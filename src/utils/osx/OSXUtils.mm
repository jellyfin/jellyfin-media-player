#include "OSXUtils.h"
#include "QsLog.h"
#import <Cocoa/Cocoa.h>

/////////////////////////////////////////////////////////////////////////////////////////
unsigned long OSXUtils::GetPresentationOptionsForFullscreen(bool hideMenuAndDock)
{
  unsigned long flags = 0;
  if (hideMenuAndDock)
  {
    flags = flags & ~(NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar);
    flags |= NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar;
  }
  else
  {
    flags = flags & ~(NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar);
    flags |= NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar;
  }

  return flags;
}

/////////////////////////////////////////////////////////////////////////////////////////
void OSXUtils::SetPresentationOptions(unsigned long flags)
{
  QLOG_DEBUG() << "Setting presentationOptions =" << flags;
  [[NSApplication sharedApplication] setPresentationOptions:flags];
}

/////////////////////////////////////////////////////////////////////////////////////////
unsigned long OSXUtils::GetPresentationOptions()
{
  unsigned long options = [[NSApplication sharedApplication] presentationOptions];
  QLOG_DEBUG() << "Getting presentationOptions =" << options;
  return options;
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

/////////////////////////////////////////////////////////////////////////////////////////
void OSXUtils::SetCursorVisible(bool visible)
{
  if (visible)
    [NSCursor unhide];
  else
    [NSCursor hide];
}
