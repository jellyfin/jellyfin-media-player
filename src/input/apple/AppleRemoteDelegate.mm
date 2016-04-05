#import "AppleRemoteDelegate.h"
#import "InputAppleRemote.h"

#include "QsLog.h"

@implementation AppleRemoteDelegate

///////////////////////////////////////////////////////////////////////////////////////////////////
- (instancetype)initWithRemoteHandler: (InputAppleRemote*)remoteHandler
{
  self = [super init];
  if (self) {
    m_remoteHandler = remoteHandler;
  }
  return self;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
- (bool)setupRemote
{
  [[HIDRemote sharedHIDRemote] setDelegate:self];
  [[HIDRemote sharedHIDRemote] setSimulateHoldEvents:NO];
  [[HIDRemote sharedHIDRemote] setExclusiveLockLendingEnabled:YES];
  if (![[HIDRemote sharedHIDRemote] startRemoteControl:kHIDRemoteModeExclusive])
  {
    QLOG_ERROR() << "Failed to init AppleRemote";
    return false;
  }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
- (QString)remoteNameFromAttributes:(NSMutableDictionary*)attributes
{
  NSString* product = [attributes objectForKey:kHIDRemoteProduct];
  NSString* manufacturer = [attributes objectForKey:kHIDRemoteManufacturer];
  
  return QString("%1-%2").arg([manufacturer UTF8String]).arg([product UTF8String]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
- (void)hidRemote:(HIDRemote *)hidRemote
  eventWithButton:(HIDRemoteButtonCode)buttonCode
        isPressed:(BOOL)isPressed
fromHardwareWithAttributes:(NSMutableDictionary *)attributes
{
  m_remoteHandler->remoteButtonEvent(buttonCode, isPressed, [self remoteNameFromAttributes:attributes]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
- (void)hidRemote:(HIDRemote *)hidRemote foundNewHardwareWithAttributes:(NSMutableDictionary *)attributes
{
  m_remoteHandler->addRemote([self remoteNameFromAttributes:attributes]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
- (void)hidRemote:(HIDRemote *)hidRemote releasedHardwareWithAttributes:(NSMutableDictionary *)attributes
{
  m_remoteHandler->removeRemote([self remoteNameFromAttributes:attributes]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
- (void)hidRemote:(HIDRemote *)hidRemote failedNewHardwareWithError:(NSError *)error
{
  m_remoteHandler->addRemoteFailed(QString([[error localizedDescription] UTF8String]));
}

/////////////////////////////////////////////////////////////////////////////////////////
- (void)hidRemote:(HIDRemote *)hidRemote remoteIDChangedOldID:(SInt32)old newID:(SInt32)newID
                                    forHardwareWithAttributes:(NSMutableDictionary *)attributes
{
  m_remoteHandler->changeRemoteID(newID);
}

@end
