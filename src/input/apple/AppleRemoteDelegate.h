#ifndef __APPLE_REMOTE_DELEGATE_H__
#define __APPLE_REMOTE_DELEGATE_H__

#import <Cocoa/Cocoa.h>
#import "HIDRemote.h"

class InputAppleRemote;

@interface AppleRemoteDelegate : NSObject<HIDRemoteDelegate>
{
  InputAppleRemote* m_remoteHandler;
}

- (instancetype)initWithRemoteHandler:(InputAppleRemote*)remoteHandler;

- (void)hidRemote:(HIDRemote *)hidRemote
  eventWithButton:(HIDRemoteButtonCode)buttonCode
        isPressed:(BOOL)isPressed
fromHardwareWithAttributes:(NSMutableDictionary *)attributes;

- (void)hidRemote:(HIDRemote *)hidRemote failedNewHardwareWithError:(NSError *)error;
- (void)hidRemote:(HIDRemote *)hidRemote foundNewHardwareWithAttributes:(NSMutableDictionary *)attributes;
- (void)hidRemote:(HIDRemote *)hidRemote releasedHardwareWithAttributes:(NSMutableDictionary *)attributes;

- (bool)setupRemote;

@end

#endif
