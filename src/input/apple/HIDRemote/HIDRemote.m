//
//  HIDRemote.m
//  HIDRemote V1.2 (27th May 2011)
//
//  Created by Felix Schwarz on 06.04.07.
//  Copyright 2007-2011 IOSPIRIT GmbH. All rights reserved.
//
//  The latest version of this class is available at
//     http://www.iospirit.com/developers/hidremote/
//
//  ** LICENSE *************************************************************************
//
//  Copyright (c) 2007-2011 IOSPIRIT GmbH (http://www.iospirit.com/)
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modification,
//  are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this list
//    of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice, this
//    list of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
//  * Neither the name of IOSPIRIT GmbH nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific prior
//    written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
//
//  ************************************************************************************

//  ************************************************************************************
//  ********************************** DOCUMENTATION ***********************************
//  ************************************************************************************
//
//  - a reference is available at http://www.iospirit.com/developers/hidremote/reference/
//  - for a guide, please see http://www.iospirit.com/developers/hidremote/guide/
//
//  ************************************************************************************

#import "HIDRemote.h"

// Callback Prototypes
static void HIDEventCallback(void * target,
                             IOReturn result,
                             void * refcon,
                             void * sender);

static void ServiceMatchingCallback(void *refCon,
                                    io_iterator_t iterator);

static void ServiceNotificationCallback(void *      refCon,
                                        io_service_t    service,
                                        natural_t   messageType,
                                        void *      messageArgument);

static void SecureInputNotificationCallback(void *      refCon,
                                            io_service_t    service,
                                            natural_t   messageType,
                                            void *      messageArgument);

// Shared HIDRemote instance
static HIDRemote *sHIDRemote = nil;

@implementation HIDRemote

#pragma mark -- Init, dealloc & shared instance --

+ (HIDRemote *)sharedHIDRemote
{
    if (sHIDRemote==nil)
    {
        sHIDRemote = [[HIDRemote alloc] init];
    }

    return (sHIDRemote);
}

- (id)init
{
    if ((self = [super init]) != nil)
    {
#ifdef HIDREMOTE_THREADSAFETY_HARDENED_NOTIFICATION_HANDLING
        _runOnThread = [NSThread currentThread];
#endif

        // Detect application becoming active/inactive
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_appStatusChanged:)    name:NSApplicationDidBecomeActiveNotification  object:[NSApplication sharedApplication]];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_appStatusChanged:)    name:NSApplicationWillResignActiveNotification object:[NSApplication sharedApplication]];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_appStatusChanged:)    name:NSApplicationWillTerminateNotification    object:[NSApplication sharedApplication]];

        // Handle distributed notifications
        _pidString = [[NSString alloc] initWithFormat:@"%d", getpid()];

        [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(_handleNotifications:) name:kHIDRemoteDNHIDRemotePing  object:nil];
        [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(_handleNotifications:) name:kHIDRemoteDNHIDRemoteRetry object:kHIDRemoteDNHIDRemoteRetryGlobalObject];
        [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(_handleNotifications:) name:kHIDRemoteDNHIDRemoteRetry object:_pidString];

        // Enabled by default: simulate hold events for plus/minus
        _simulateHoldEvents = YES;

        // Enabled by default: work around for a locking issue introduced with Security Update 2008-004 / 10.4.9 and beyond (credit for finding this workaround goes to Martin Kahr)
        _secureEventInputWorkAround = YES;
        _secureInputNotification = 0;

        // Initialize instance variables
        _lastSeenRemoteID = -1;
        _lastSeenModel = kHIDRemoteModelUndetermined;
        _unusedButtonCodes = [[NSMutableArray alloc] init];
        _exclusiveLockLending = NO;
        _sendExclusiveResourceReuseNotification = YES;
        _applicationIsTerminating = NO;

        // Send status notifications
        _sendStatusNotifications = YES;
    }

    return (self);
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationWillTerminateNotification object:[NSApplication sharedApplication]];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationWillResignActiveNotification object:[NSApplication sharedApplication]];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationDidBecomeActiveNotification object:[NSApplication sharedApplication]];

    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:kHIDRemoteDNHIDRemotePing  object:nil];
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:kHIDRemoteDNHIDRemoteRetry object:kHIDRemoteDNHIDRemoteRetryGlobalObject];
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:kHIDRemoteDNHIDRemoteRetry object:_pidString];
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:nil object:nil]; /* As demanded by the documentation for -[NSDistributedNotificationCenter removeObserver:name:object:] */

    [self stopRemoteControl];

    [self setExclusiveLockLendingEnabled:NO];

    [self setDelegate:nil];

    _unusedButtonCodes = nil;

#ifdef HIDREMOTE_THREADSAFETY_HARDENED_NOTIFICATION_HANDLING
    _runOnThread = nil;
#endif

    _pidString = nil;
}

#pragma mark -- PUBLIC: System Information --
+ (BOOL)isCandelairInstalled
{
    mach_port_t masterPort = 0;
    kern_return_t   kernResult;
    io_service_t    matchingService = 0;
    BOOL isInstalled = NO;

    kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if ((kernResult!=kIOReturnSuccess) || (masterPort==0)) { return(NO); }

    if ((matchingService = IOServiceGetMatchingService(masterPort, IOServiceMatching("IOSPIRITIRController"))) != 0)
    {
        isInstalled = YES;
        IOObjectRelease((io_object_t) matchingService);
    }

    mach_port_deallocate(mach_task_self(), masterPort);

    return (isInstalled);
}

+ (BOOL)isCandelairInstallationRequiredForRemoteMode:(HIDRemoteMode)remoteMode
{
    return (NO);
}

- (HIDRemoteAluminumRemoteSupportLevel)aluminiumRemoteSystemSupportLevel
{
    HIDRemoteAluminumRemoteSupportLevel supportLevel = kHIDRemoteAluminumRemoteSupportLevelNone;

    for (NSDictionary *hidAttribsDict in [_serviceAttribMap objectEnumerator])
    {
        NSNumber *deviceSupportLevel;

        if ((deviceSupportLevel = hidAttribsDict[kHIDRemoteAluminumRemoteSupportLevel]) != nil)
        {
            if ([deviceSupportLevel intValue] > (int)supportLevel)
            {
                supportLevel = [deviceSupportLevel intValue];
            }
        }
    }

    return (supportLevel);
}

#pragma mark -- PUBLIC: Interface / API --
- (BOOL)startRemoteControl:(HIDRemoteMode)hidRemoteMode
{
    if ((_mode == kHIDRemoteModeNone) && (hidRemoteMode != kHIDRemoteModeNone))
    {
        kern_return_t       kernReturn;
        CFMutableDictionaryRef  matchDict=NULL;
        io_service_t rootService;

        do
        {
            // Get IOKit master port
            kernReturn = IOMasterPort(bootstrap_port, &_masterPort);
            if ((kernReturn!=kIOReturnSuccess) || (_masterPort==0)) { break; }

            // Setup notification port
            _notifyPort = IONotificationPortCreate(_masterPort);

            if ((_notifyRLSource = IONotificationPortGetRunLoopSource(_notifyPort)) != NULL)
            {
                CFRunLoopAddSource( CFRunLoopGetCurrent(),
                                   _notifyRLSource,
                                   kCFRunLoopCommonModes);
            }
            else
            {
                break;
            }

            // Setup SecureInput notification
            if ((hidRemoteMode == kHIDRemoteModeExclusive) || (hidRemoteMode == kHIDRemoteModeExclusiveAuto))
            {
                if ((rootService = IORegistryEntryFromPath(_masterPort, kIOServicePlane ":/")) != 0)
                {
                    kernReturn = IOServiceAddInterestNotification(  _notifyPort,
                                                                  rootService,
                                                                  kIOBusyInterest,
                                                                  SecureInputNotificationCallback,
                                                                  (__bridge void *)self,
                                                                  &_secureInputNotification);
                    if (kernReturn != kIOReturnSuccess) { break; }

                    [self _updateSessionInformation];
                }
                else
                {
                    break;
                }
            }

            // Setup notification matching dict
            matchDict = IOServiceMatching(kIOHIDDeviceKey);
            CFRetain(matchDict);

            // Actually add notification
            kernReturn = IOServiceAddMatchingNotification(  _notifyPort,
                                                          kIOFirstMatchNotification,
                                                          matchDict,            // one reference count consumed by this call
                                                          ServiceMatchingCallback,
                                                          (__bridge void *) self,
                                                          &_matchingServicesIterator);
            if (kernReturn != kIOReturnSuccess) { break; }

            // Setup serviceAttribMap
            _serviceAttribMap = [[NSMutableDictionary alloc] init];
            if (_serviceAttribMap==nil) { break; }

            // Phew .. everything went well!
            _mode = hidRemoteMode;
            CFRelease(matchDict);

            [self _serviceMatching:_matchingServicesIterator];

            [self _postStatusWithAction:kHIDRemoteDNStatusActionStart];

            return (YES);

        }while(0);

        // An error occured. Do necessary clean up.
        if (matchDict!=NULL)
        {
            CFRelease(matchDict);
            matchDict = NULL;
        }

        [self stopRemoteControl];
    }

    return (NO);
}

- (void)stopRemoteControl
{
    UInt32 serviceCount = 0;

    _autoRecover = NO;
    _isStopping = YES;

    if (_autoRecoveryTimer!=nil)
    {
        [_autoRecoveryTimer invalidate];
        _autoRecoveryTimer = nil;
    }

    if (_serviceAttribMap!=nil)
    {
        NSDictionary *cloneDict = [[NSDictionary alloc] initWithDictionary:_serviceAttribMap];

        for (NSNumber *serviceValue in [cloneDict keyEnumerator])
        {
            [self _destructService:(io_object_t)[serviceValue unsignedIntValue]];
            serviceCount++;
        };

        _serviceAttribMap = nil;
    }

    if (_matchingServicesIterator!=0)
    {
        IOObjectRelease((io_object_t) _matchingServicesIterator);
        _matchingServicesIterator = 0;
    }

    if (_secureInputNotification!=0)
    {
        IOObjectRelease((io_object_t) _secureInputNotification);
        _secureInputNotification = 0;
    }

    if (_notifyRLSource!=NULL)
    {
        CFRunLoopSourceInvalidate(_notifyRLSource);
        _notifyRLSource = NULL;
    }

    if (_notifyPort!=NULL)
    {
        IONotificationPortDestroy(_notifyPort);
        _notifyPort = NULL;
    }

    if (_masterPort!=0)
    {
        mach_port_deallocate(mach_task_self(), _masterPort);
        _masterPort = 0;
    }

    _returnToPID = nil;

    if (_mode!=kHIDRemoteModeNone)
    {
        // Post status
        [self _postStatusWithAction:kHIDRemoteDNStatusActionStop];

        if (_sendStatusNotifications)
        {
            // In case we were not ready to lend it earlier, tell other HIDRemote apps that the resources (if any were used) are now again available for use by other applications
            if (((_mode==kHIDRemoteModeExclusive) || (_mode==kHIDRemoteModeExclusiveAuto)) && (_sendExclusiveResourceReuseNotification==YES) && (_exclusiveLockLending==NO) && (serviceCount>0))
            {
                _mode = kHIDRemoteModeNone;

                if (!_isRestarting)
                {
                    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:kHIDRemoteDNHIDRemoteRetry
                                                                                   object:kHIDRemoteDNHIDRemoteRetryGlobalObject
                                                                                 userInfo:@{kHIDRemoteDNStatusPIDKey: @((unsigned int)getpid()),
                                                                                           (NSString *)kCFBundleIdentifierKey: [[NSBundle mainBundle] bundleIdentifier]}
                                                                       deliverImmediately:YES];
                }
            }
        }
    }

    _mode = kHIDRemoteModeNone;
    _isStopping = NO;
}

- (BOOL)isStarted
{
    return (_mode != kHIDRemoteModeNone);
}

- (HIDRemoteMode)startedInMode
{
    return (_mode);
}

- (unsigned)activeRemoteControlCount
{
    return (int)[_serviceAttribMap count];
}

- (SInt32)lastSeenRemoteControlID
{
    return (_lastSeenRemoteID);
}

- (HIDRemoteModel)lastSeenModel
{
    return (_lastSeenModel);
}

- (void)setLastSeenModel:(HIDRemoteModel)aModel
{
    _lastSeenModel = aModel;
}

- (void)setSimulateHoldEvents:(BOOL)newSimulateHoldEvents
{
    _simulateHoldEvents = newSimulateHoldEvents;
}

- (BOOL)simulateHoldEvents
{
    return (_simulateHoldEvents);
}

- (NSArray *)unusedButtonCodes
{
    return (_unusedButtonCodes);
}

- (void)setUnusedButtonCodes:(NSArray *)newArrayWithUnusedButtonCodesAsNSNumbers
{
    _unusedButtonCodes = newArrayWithUnusedButtonCodesAsNSNumbers;

    [self _postStatusWithAction:kHIDRemoteDNStatusActionUpdate];
}

- (void)setDelegate:(NSObject <HIDRemoteDelegate> *)newDelegate
{
    _delegate = newDelegate;
}

- (NSObject <HIDRemoteDelegate> *)delegate
{
    return (_delegate);
}

#pragma mark -- PUBLIC: Expert APIs --
- (void)setEnableSecureEventInputWorkaround:(BOOL)newEnableSecureEventInputWorkaround
{
    _secureEventInputWorkAround = newEnableSecureEventInputWorkaround;
}

- (BOOL)enableSecureEventInputWorkaround
{
    return (_secureEventInputWorkAround);
}

- (void)setExclusiveLockLendingEnabled:(BOOL)newExclusiveLockLendingEnabled
{
    if (newExclusiveLockLendingEnabled != _exclusiveLockLending)
    {
        _exclusiveLockLending = newExclusiveLockLendingEnabled;

        if (_exclusiveLockLending)
        {
            [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(_handleNotifications:) name:kHIDRemoteDNHIDRemoteStatus object:nil];
        }
        else
        {
            [[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:kHIDRemoteDNHIDRemoteStatus object:nil];

            _waitForReturnByPID = nil;
        }
    }
}

- (BOOL)exclusiveLockLendingEnabled
{
    return (_exclusiveLockLending);
}

- (void)setSendExclusiveResourceReuseNotification:(BOOL)newSendExclusiveResourceReuseNotification
{
    _sendExclusiveResourceReuseNotification = newSendExclusiveResourceReuseNotification;
}

- (BOOL)sendExclusiveResourceReuseNotification
{
    return (_sendExclusiveResourceReuseNotification);
}

- (BOOL)isApplicationTerminating
{
    return (_applicationIsTerminating);
}

- (BOOL)isStopping
{
    return (_isStopping);
}

#pragma mark -- PRIVATE: Application becomes active / inactive handling for kHIDRemoteModeExclusiveAuto --
- (void)_appStatusChanged:(NSNotification *)notification
{
#ifdef HIDREMOTE_THREADSAFETY_HARDENED_NOTIFICATION_HANDLING
    if ([self respondsToSelector:@selector(performSelector:onThread:withObject:waitUntilDone:)]) // OS X 10.5+ only
    {
        if ([NSThread currentThread] != _runOnThread)
        {
            if ([[notification name] isEqual:NSApplicationDidBecomeActiveNotification])
            {
                if (!_autoRecover)
                {
                    return;
                }
            }

            if ([[notification name] isEqual:NSApplicationWillResignActiveNotification])
            {
                if (_mode != kHIDRemoteModeExclusiveAuto)
                {
                    return;
                }
            }

            [self performSelector:@selector(_appStatusChanged:) onThread:_runOnThread withObject:notification waitUntilDone:[[notification name] isEqual:NSApplicationWillTerminateNotification]];
            return;
        }
    }
#endif

    if (notification!=nil)
    {
        if (_autoRecoveryTimer!=nil)
        {
            [_autoRecoveryTimer invalidate];
            _autoRecoveryTimer = nil;
        }

        if ([[notification name] isEqual:NSApplicationDidBecomeActiveNotification])
        {
            if (_autoRecover)
            {
                // Delay autorecover by 0.1 to avoid race conditions
                if ((_autoRecoveryTimer = [[NSTimer alloc] initWithFireDate:[NSDate dateWithTimeIntervalSinceNow:0.1] interval:0.1 target:self selector:@selector(_delayedAutoRecovery:) userInfo:nil repeats:NO]) != nil)
                {
                    [[NSRunLoop currentRunLoop] addTimer:_autoRecoveryTimer forMode:NSRunLoopCommonModes];
                }
            }
        }

        if ([[notification name] isEqual:NSApplicationWillResignActiveNotification])
        {
            if (_mode == kHIDRemoteModeExclusiveAuto)
            {
                [self stopRemoteControl];
                _autoRecover = YES;
            }
        }

        if ([[notification name] isEqual:NSApplicationWillTerminateNotification])
        {
            _applicationIsTerminating = YES;

            if ([self isStarted])
            {
                [self stopRemoteControl];
            }
        }
    }
}

- (void)_delayedAutoRecovery:(NSTimer *)aTimer
{
    [_autoRecoveryTimer invalidate];
    _autoRecoveryTimer = nil;

    if (_autoRecover)
    {
        [self startRemoteControl:kHIDRemoteModeExclusiveAuto];
        _autoRecover = NO;
    }
}


#pragma mark -- PRIVATE: Distributed notifiations handling --
- (void)_postStatusWithAction:(NSString *)action
{
    if (_sendStatusNotifications)
    {
        NSDictionary* d = @{kHIDRemoteDNStatusHIDRemoteVersionKey: @1,
                            kHIDRemoteDNStatusPIDKey: @((unsigned int)getpid()),
                            kHIDRemoteDNStatusModeKey: @((int)_mode),
                            kHIDRemoteDNStatusRemoteControlCountKey: @((unsigned int)[self activeRemoteControlCount]),
                            kHIDRemoteDNStatusUnusedButtonCodesKey: ((_unusedButtonCodes!=nil) ? _unusedButtonCodes : @[]),
                            kHIDRemoteDNStatusActionKey: action,
                            (NSString *)kCFBundleIdentifierKey: [[NSBundle mainBundle] bundleIdentifier]};

        NSMutableDictionary* md = [NSMutableDictionary dictionaryWithDictionary:d];

        if (_returnToPID) {
            md[kHIDRemoteDNStatusReturnToPIDKey] = _returnToPID;
        }

        [[NSDistributedNotificationCenter defaultCenter] postNotificationName:kHIDRemoteDNHIDRemoteStatus
                                                                       object:((_pidString!=nil) ? _pidString : [NSString stringWithFormat:@"%d",getpid()])
                                                                     userInfo:[md copy]
                                                           deliverImmediately:YES
        ];
    }
}

- (void)_handleNotifications:(NSNotification *)notification
{
    NSString *notificationName;

#ifdef HIDREMOTE_THREADSAFETY_HARDENED_NOTIFICATION_HANDLING
    if ([self respondsToSelector:@selector(performSelector:onThread:withObject:waitUntilDone:)]) // OS X 10.5+ only
    {
        if ([NSThread currentThread] != _runOnThread)
        {
            [self performSelector:@selector(_handleNotifications:) onThread:_runOnThread withObject:notification waitUntilDone:NO];
            return;
        }
    }
#endif

    if ((notification!=nil) && ((notificationName = [notification name]) != nil))
    {
        if ([notificationName isEqual:kHIDRemoteDNHIDRemotePing])
        {
            [self _postStatusWithAction:kHIDRemoteDNStatusActionUpdate];
        }

        if ([notificationName isEqual:kHIDRemoteDNHIDRemoteRetry])
        {
            if ([self isStarted])
            {
                BOOL retry = YES;

                // Ignore our own global retry broadcasts
                if ([[notification object] isEqual:kHIDRemoteDNHIDRemoteRetryGlobalObject])
                {
                    NSNumber *fromPID;

                    if ((fromPID = [notification userInfo][kHIDRemoteDNStatusPIDKey]) != nil)
                    {
                        if (getpid() == (int)[fromPID unsignedIntValue])
                        {
                            retry = NO;
                        }
                    }
                }

                if (retry)
                {
                    if (([self delegate] != nil) &&
                        ([[self delegate] respondsToSelector:@selector(hidRemote:shouldRetryExclusiveLockWithInfo:)]))
                    {
                        retry = [[self delegate] hidRemote:self shouldRetryExclusiveLockWithInfo:[notification userInfo]];
                    }
                }

                if (retry)
                {
                    HIDRemoteMode restartInMode = _mode;

                    if (restartInMode != kHIDRemoteModeNone)
                    {
                        _isRestarting = YES;
                        [self stopRemoteControl];

                        _returnToPID = nil;

                        [self startRemoteControl:restartInMode];
                        _isRestarting = NO;

                        if (restartInMode != kHIDRemoteModeShared)
                        {
                            _returnToPID = [notification userInfo][kHIDRemoteDNStatusPIDKey];
                        }
                    }
                }
                else
                {
                    NSNumber *cacheReturnPID = _returnToPID;

                    _returnToPID = [notification userInfo][kHIDRemoteDNStatusPIDKey];
                    [self _postStatusWithAction:kHIDRemoteDNStatusActionNoNeed];

                    _returnToPID = cacheReturnPID;
                }
            }
        }

        if (_exclusiveLockLending)
        {
            if ([notificationName isEqual:kHIDRemoteDNHIDRemoteStatus])
            {
                NSString *action;

                if ((action = [notification userInfo][kHIDRemoteDNStatusActionKey]) != nil)
                {
                    if ((_mode == kHIDRemoteModeNone) && (_waitForReturnByPID!=nil))
                    {
                        NSNumber *pidNumber, *returnToPIDNumber;

                        if ((pidNumber      = [notification userInfo][kHIDRemoteDNStatusPIDKey]) != nil)
                        {
                            returnToPIDNumber = [notification userInfo][kHIDRemoteDNStatusReturnToPIDKey];
                            if (returnToPIDNumber == (id)[NSNull null]) {
                                returnToPIDNumber = nil;
                            }

                            if ([action isEqual:kHIDRemoteDNStatusActionStart])
                            {
                                if ([pidNumber isEqual:_waitForReturnByPID])
                                {
                                    NSNumber *startMode;

                                    if ((startMode = [notification userInfo][kHIDRemoteDNStatusModeKey]) != nil)
                                    {
                                        if ([startMode intValue] == kHIDRemoteModeShared)
                                        {
                                            returnToPIDNumber = @(getpid());
                                            action = kHIDRemoteDNStatusActionNoNeed;
                                        }
                                    }
                                }
                            }

                            if (returnToPIDNumber != nil)
                            {
                                if ([action isEqual:kHIDRemoteDNStatusActionStop] || [action isEqual:kHIDRemoteDNStatusActionNoNeed])
                                {
                                    if ([pidNumber isEqual:_waitForReturnByPID] && ([returnToPIDNumber intValue] == getpid()))
                                    {
                                        _waitForReturnByPID = nil;

                                        if (([self delegate] != nil) &&
                                            ([[self delegate] respondsToSelector:@selector(hidRemote:exclusiveLockReleasedByApplicationWithInfo:)]))
                                        {
                                            [[self delegate] hidRemote:self exclusiveLockReleasedByApplicationWithInfo:[notification userInfo]];
                                        }
                                        else
                                        {
                                            [self startRemoteControl:kHIDRemoteModeExclusive];
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (_mode==kHIDRemoteModeExclusive)
                    {
                        if ([action isEqual:kHIDRemoteDNStatusActionStart])
                        {
                            NSNumber *originPID = [notification userInfo][kHIDRemoteDNStatusPIDKey];
                            BOOL lendLock = YES;

                            if ([originPID intValue] != getpid())
                            {
                                if (([self delegate] != nil) &&
                                    ([[self delegate] respondsToSelector:@selector(hidRemote:lendExclusiveLockToApplicationWithInfo:)]))
                                {
                                    lendLock = [[self delegate] hidRemote:self lendExclusiveLockToApplicationWithInfo:[notification userInfo]];
                                }

                                if (lendLock)
                                {
                                    _waitForReturnByPID = originPID;

                                    if (_waitForReturnByPID != nil)
                                    {
                                        [self stopRemoteControl];

                                        [[NSDistributedNotificationCenter defaultCenter] postNotificationName:kHIDRemoteDNHIDRemoteRetry
                                                                                                       object:[NSString stringWithFormat:@"%d", [_waitForReturnByPID intValue]]
                                                                                                     userInfo:@{kHIDRemoteDNStatusPIDKey: @((unsigned int)getpid()),
                                                                                                               (NSString *)kCFBundleIdentifierKey: [[NSBundle mainBundle] bundleIdentifier]}
                                                                                           deliverImmediately:YES];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

- (void)_setSendStatusNotifications:(BOOL)doSend
{
    _sendStatusNotifications = doSend;
}

- (BOOL)_sendStatusNotifications
{
    return (_sendStatusNotifications);
}

#pragma mark -- PRIVATE: Service setup and destruction --
- (BOOL)_prematchService:(io_object_t)service
{
    BOOL serviceMatches = NO;
    NSString *ioClass;
    NSNumber *candelairHIDRemoteCompatibilityMask;

    if (service != 0)
    {
        // IOClass matching
        if ((ioClass = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty((io_registry_entry_t)service,
                                                                   CFSTR(kIOClassKey),
                                                                   kCFAllocatorDefault,
                                                                   0)) != nil)
        {
            // Match on apple's AppleIRController and old versions of the Remote Buddy IR Controller
            if ([ioClass isEqual:@"AppleIRController"] || [ioClass isEqual:@"RBIOKitAIREmu"])
            {
                CFTypeRef candelairHIDRemoteCompatibilityDevice;

                serviceMatches = YES;

                if ((candelairHIDRemoteCompatibilityDevice = IORegistryEntryCreateCFProperty((io_registry_entry_t)service, CFSTR("CandelairHIDRemoteCompatibilityDevice"), kCFAllocatorDefault, 0)) != NULL)
                {
                    if (CFEqual(kCFBooleanTrue, candelairHIDRemoteCompatibilityDevice))
                    {
                        serviceMatches = NO;
                    }

                    CFRelease (candelairHIDRemoteCompatibilityDevice);
                }
            }

            // Match on the virtual IOSPIRIT IR Controller
            if ([ioClass isEqual:@"IOSPIRITIRController"])
            {
                serviceMatches = YES;
            }
        }

        // Match on services that claim compatibility with the HID Remote class (Candelair or third-party) by having a property of CandelairHIDRemoteCompatibilityMask = 1 <Type: Number>
        if ((candelairHIDRemoteCompatibilityMask = (__bridge_transfer NSNumber *)IORegistryEntryCreateCFProperty((io_registry_entry_t)service, CFSTR("CandelairHIDRemoteCompatibilityMask"), kCFAllocatorDefault, 0)) != nil)
        {
            if ([candelairHIDRemoteCompatibilityMask isKindOfClass:[NSNumber class]])
            {
                if ([candelairHIDRemoteCompatibilityMask unsignedIntValue] & kHIDRemoteCompatibilityFlagsStandardHIDRemoteDevice)
                {
                    serviceMatches = YES;
                }
                else
                {
                    serviceMatches = NO;
                }
            }
        }
    }

    if (([self delegate]!=nil) &&
        ([[self delegate] respondsToSelector:@selector(hidRemote:inspectNewHardwareWithService:prematchResult:)]))
    {
        serviceMatches = [((NSObject <HIDRemoteDelegate> *)[self delegate]) hidRemote:self inspectNewHardwareWithService:service prematchResult:serviceMatches];
    }

    return (serviceMatches);
}

- (HIDRemoteButtonCode)buttonCodeForUsage:(unsigned int)usage usagePage:(unsigned int)usagePage
{
    HIDRemoteButtonCode buttonCode = kHIDRemoteButtonCodeNone;

    switch (usagePage)
    {
        case kHIDPage_Consumer:
            switch (usage)
        {
            case kHIDUsage_Csmr_MenuPick:
                // Aluminum Remote: Center
                buttonCode = (kHIDRemoteButtonCodeCenter|kHIDRemoteButtonCodeAluminumMask);
                break;

            case kHIDUsage_Csmr_ModeStep:
                // Aluminium Remote: Center Hold
                buttonCode = (kHIDRemoteButtonCodeCenterHold|kHIDRemoteButtonCodeAluminumMask);
                break;

            case kHIDUsage_Csmr_PlayOrPause:
                // Aluminum Remote: Play/Pause
                buttonCode = (kHIDRemoteButtonCodePlay|kHIDRemoteButtonCodeAluminumMask);
                break;

            case kHIDUsage_Csmr_Rewind:
                buttonCode = kHIDRemoteButtonCodeLeftHold;
                break;

            case kHIDUsage_Csmr_FastForward:
                buttonCode = kHIDRemoteButtonCodeRightHold;
                break;

            case kHIDUsage_Csmr_Menu:
                buttonCode = kHIDRemoteButtonCodeMenuHold;
                break;
        }
            break;

        case kHIDPage_GenericDesktop:
            switch (usage)
        {
            case kHIDUsage_GD_SystemAppMenu:
                buttonCode = kHIDRemoteButtonCodeMenu;
                break;

            case kHIDUsage_GD_SystemMenu:
                buttonCode = kHIDRemoteButtonCodeCenter;
                break;

            case kHIDUsage_GD_SystemMenuRight:
                buttonCode = kHIDRemoteButtonCodeRight;
                break;

            case kHIDUsage_GD_SystemMenuLeft:
                buttonCode = kHIDRemoteButtonCodeLeft;
                break;

            case kHIDUsage_GD_SystemMenuUp:
                buttonCode = kHIDRemoteButtonCodeUp;
                break;

            case kHIDUsage_GD_SystemMenuDown:
                buttonCode = kHIDRemoteButtonCodeDown;
                break;
        }
            break;

        case 0x06: /* Reserved */
            switch (usage)
        {
            case 0x22:
                buttonCode = kHIDRemoteButtonCodeIDChanged;
                break;
        }
            break;

        case 0xFF01: /* Vendor specific */
            switch (usage)
        {
            case 0x23:
                buttonCode = kHIDRemoteButtonCodeCenterHold;
                break;

#ifdef _HIDREMOTE_EXTENSIONS
#define _HIDREMOTE_EXTENSIONS_SECTION 2
#include "HIDRemoteAdditions.h"
#undef _HIDREMOTE_EXTENSIONS_SECTION
#endif /* _HIDREMOTE_EXTENSIONS */
        }
            break;
    }

    return (buttonCode);
}

- (BOOL)_setupService:(io_object_t)service
{
    kern_return_t        kernResult;
    IOReturn         returnCode;
    HRESULT          hResult;
    SInt32           score;
    BOOL             opened = NO, queueStarted = NO;
    IOHIDDeviceInterface122  **hidDeviceInterface   = NULL;
    IOCFPlugInInterface  **cfPluginInterface    = NULL;
    IOHIDQueueInterface  **hidQueueInterface    = NULL;
    io_object_t      serviceNotification    = 0;
    CFRunLoopSourceRef   queueEventSource   = NULL;
    NSMutableDictionary  *hidAttribsDict    = nil;
    CFArrayRef       hidElements        = NULL;
    NSError          *error         = nil;
    UInt32           errorCode      = 0;

    if (![self _prematchService:service])
    {
        return (NO);
    }

    do
    {
        // Create a plugin interface ..
        kernResult = IOCreatePlugInInterfaceForService( service,
                                                       kIOHIDDeviceUserClientTypeID,
                                                       kIOCFPlugInInterfaceID,
                                                       &cfPluginInterface,
                                                       &score);

        if (kernResult != kIOReturnSuccess)
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:kernResult userInfo:nil];
            errorCode = 1;
            break;
        }


        // .. use it to get the HID interface ..
        hResult = (*cfPluginInterface)->QueryInterface( cfPluginInterface,
                                                       CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID122),
                                                       (LPVOID)&hidDeviceInterface);

        if ((hResult!=S_OK) || (hidDeviceInterface==NULL))
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:hResult userInfo:nil];
            errorCode = 2;
            break;
        }


        // .. then open it ..
        switch (_mode)
        {
            case kHIDRemoteModeShared:
                hResult = (*hidDeviceInterface)->open(hidDeviceInterface, kIOHIDOptionsTypeNone);
                break;

            case kHIDRemoteModeExclusive:
            case kHIDRemoteModeExclusiveAuto:
                hResult = (*hidDeviceInterface)->open(hidDeviceInterface, kIOHIDOptionsTypeSeizeDevice);
                break;

            default:
                goto cleanUp; // Ugh! But there are no "double breaks" available in C AFAIK ..
                break;
        }

        if (hResult!=S_OK)
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:hResult userInfo:nil];
            errorCode = 3;
            break;
        }

        opened = YES;

        // .. query the HID elements ..
        returnCode = (*hidDeviceInterface)->copyMatchingElements(hidDeviceInterface,
                                                                 NULL,
                                                                 &hidElements);
        if ((returnCode != kIOReturnSuccess) || (hidElements==NULL))
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:returnCode userInfo:nil];
            errorCode = 4;

            break;
        }

        // Setup an event queue for HID events!
        hidQueueInterface = (*hidDeviceInterface)->allocQueue(hidDeviceInterface);
        if (hidQueueInterface == NULL)
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:kIOReturnError userInfo:nil];
            errorCode = 5;

            break;
        }

        returnCode = (*hidQueueInterface)->create(hidQueueInterface, 0, 32);
        if (returnCode != kIOReturnSuccess)
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:returnCode userInfo:nil];
            errorCode = 6;

            break;
        }


        // Setup of attributes stored for this HID device
        hidAttribsDict = [[NSMutableDictionary alloc] initWithObjectsAndKeys:
                          [NSValue valueWithPointer:(const void *)cfPluginInterface],   kHIDRemoteCFPluginInterface,
                          [NSValue valueWithPointer:(const void *)hidDeviceInterface],  kHIDRemoteHIDDeviceInterface,
                          [NSValue valueWithPointer:(const void *)hidQueueInterface],   kHIDRemoteHIDQueueInterface,
                          nil];

        {
            UInt32 i, hidElementCnt = (unsigned)CFArrayGetCount(hidElements);
            NSMutableDictionary *cookieButtonCodeLUT = [[NSMutableDictionary alloc] init];
            NSMutableDictionary *cookieCount    = [[NSMutableDictionary alloc] init];

            if ((cookieButtonCodeLUT==nil) || (cookieCount==nil))
            {
                cookieButtonCodeLUT = nil;
                cookieCount = nil;

                error = [NSError errorWithDomain:NSMachErrorDomain code:kIOReturnError userInfo:nil];
                errorCode = 7;

                break;
            }

            // Analyze the HID elements and find matching elements
            for (i=0;i<hidElementCnt;i++)
            {
                CFDictionaryRef     hidDict;
                NSNumber        *usage, *usagePage, *cookie;
                HIDRemoteButtonCode buttonCode = kHIDRemoteButtonCodeNone;

                hidDict = CFArrayGetValueAtIndex(hidElements, i);

                usage     = (__bridge NSNumber *) CFDictionaryGetValue(hidDict, CFSTR(kIOHIDElementUsageKey));
                usagePage = (__bridge NSNumber *) CFDictionaryGetValue(hidDict, CFSTR(kIOHIDElementUsagePageKey));
                cookie    = (__bridge NSNumber *) CFDictionaryGetValue(hidDict, CFSTR(kIOHIDElementCookieKey));

                if ((usage!=nil) && (usagePage!=nil) && (cookie!=nil))
                {
                    // Find the button codes for the ID combos
                    buttonCode = [self buttonCodeForUsage:[usage unsignedIntValue] usagePage:[usagePage unsignedIntValue]];

#ifdef _HIDREMOTE_EXTENSIONS
                    // Debug logging code
#define _HIDREMOTE_EXTENSIONS_SECTION 3
#include "HIDRemoteAdditions.h"
#undef _HIDREMOTE_EXTENSIONS_SECTION
#endif /* _HIDREMOTE_EXTENSIONS */

                    // Did record match?
                    if (buttonCode != kHIDRemoteButtonCodeNone)
                    {
                        NSString *pairString        = [[NSString alloc] initWithFormat:@"%u_%u", [usagePage unsignedIntValue], [usage unsignedIntValue]];
                        NSNumber *buttonCodeNumber  = @((unsigned int)buttonCode);

#ifdef _HIDREMOTE_EXTENSIONS
                        // Debug logging code
#define _HIDREMOTE_EXTENSIONS_SECTION 4
#include "HIDRemoteAdditions.h"
#undef _HIDREMOTE_EXTENSIONS_SECTION
#endif /* _HIDREMOTE_EXTENSIONS */

                        cookieCount[pairString] = buttonCodeNumber;
                        cookieButtonCodeLUT[cookie] = buttonCodeNumber;

                        (*hidQueueInterface)->addElement(hidQueueInterface,
                                                         (IOHIDElementCookie) [cookie unsignedIntValue],
                                                         0);

#ifdef _HIDREMOTE_EXTENSIONS
                        // Get current apple Remote ID value
#define _HIDREMOTE_EXTENSIONS_SECTION 7
#include "HIDRemoteAdditions.h"
#undef _HIDREMOTE_EXTENSIONS_SECTION
#endif /* _HIDREMOTE_EXTENSIONS */
                    }
                }
            }

            // Compare number of *unique* matches (thus the cookieCount dictionary) with required minimum
            if ([cookieCount count] < 10)
            {
                cookieButtonCodeLUT = nil;
                cookieCount = nil;

                error = [NSError errorWithDomain:NSMachErrorDomain code:kIOReturnError userInfo:nil];
                errorCode = 8;

                break;
            }

            hidAttribsDict[kHIDRemoteCookieButtonCodeLUT] = cookieButtonCodeLUT;

            cookieButtonCodeLUT = nil;
            cookieCount = nil;
        }

        // Finish setup of IOHIDQueueInterface with CFRunLoop
        returnCode = (*hidQueueInterface)->createAsyncEventSource(hidQueueInterface, &queueEventSource);
        if ((returnCode != kIOReturnSuccess) || (queueEventSource == NULL))
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:returnCode userInfo:nil];
            errorCode = 9;
            break;
        }

        returnCode = (*hidQueueInterface)->setEventCallout(hidQueueInterface, HIDEventCallback, (void *)((intptr_t)service), (__bridge void *)self);
        if (returnCode != kIOReturnSuccess)
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:returnCode userInfo:nil];
            errorCode = 10;
            break;
        }

        CFRunLoopAddSource( CFRunLoopGetCurrent(),
                           queueEventSource,
                           kCFRunLoopCommonModes);
        hidAttribsDict[kHIDRemoteCFRunLoopSource] = [NSValue valueWithPointer:(const void *)queueEventSource];

        returnCode = (*hidQueueInterface)->start(hidQueueInterface);
        if (returnCode != kIOReturnSuccess)
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:returnCode userInfo:nil];
            errorCode = 11;
            break;
        }

        queueStarted = YES;

        // Setup device notifications
        returnCode = IOServiceAddInterestNotification(_notifyPort,
                                                      service,
                                                      kIOGeneralInterest,
                                                      ServiceNotificationCallback,
                                                      (__bridge void *)self,
                                                      &serviceNotification);
        if ((returnCode != kIOReturnSuccess) || (serviceNotification==0))
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:returnCode userInfo:nil];
            errorCode = 12;
            break;
        }

        hidAttribsDict[kHIDRemoteServiceNotification] = @((unsigned int)serviceNotification);

        // Retain service
        if (IOObjectRetain(service) != kIOReturnSuccess)
        {
            error = [NSError errorWithDomain:NSMachErrorDomain code:kIOReturnError userInfo:nil];
            errorCode = 13;
            break;
        }

        hidAttribsDict[kHIDRemoteService] = @((unsigned int)service);

        // Get some (somewhat optional) infos on the device
        {
            CFStringRef product, manufacturer, transport;

            if ((product = IORegistryEntryCreateCFProperty( (io_registry_entry_t)service,
                                                           (CFStringRef) @"Product",
                                                           kCFAllocatorDefault,
                                                           0)) != NULL)
            {
                if (CFGetTypeID(product) == CFStringGetTypeID())
                {
                    hidAttribsDict[kHIDRemoteProduct] = (__bridge_transfer NSString *)product;
                }
                else
                {
                    CFRelease(product);
                }
            }

            if ((manufacturer = IORegistryEntryCreateCFProperty(    (io_registry_entry_t)service,
                                                                (CFStringRef) @"Manufacturer",
                                                                kCFAllocatorDefault,
                                                                0)) != NULL)
            {
                if (CFGetTypeID(manufacturer) == CFStringGetTypeID())
                {
                    hidAttribsDict[kHIDRemoteManufacturer] = (__bridge_transfer NSString *)manufacturer;
                }
                else
                {
                    CFRelease(manufacturer);
                }
            }

            if ((transport = IORegistryEntryCreateCFProperty(   (io_registry_entry_t)service,
                                                             (CFStringRef) @"Transport",
                                                             kCFAllocatorDefault,
                                                             0)) != NULL)
            {
                if (CFGetTypeID(transport) == CFStringGetTypeID())
                {
                    hidAttribsDict[kHIDRemoteTransport] = (__bridge_transfer NSString *)transport;
                }
                else
                {
                    CFRelease(transport);
                }
            }
        }

        // Determine Aluminum Remote support
        {
            CFNumberRef aluSupport;
            HIDRemoteAluminumRemoteSupportLevel supportLevel = kHIDRemoteAluminumRemoteSupportLevelNone;

            if ((_mode == kHIDRemoteModeExclusive) || (_mode == kHIDRemoteModeExclusiveAuto))
            {
                // Determine if this driver offers on-demand support for the Aluminum Remote (only relevant under OS versions < 10.6.2)
                if ((aluSupport = IORegistryEntryCreateCFProperty((io_registry_entry_t)service,
                                                                  (CFStringRef) @"AluminumRemoteSupportLevelOnDemand",
                                                                  kCFAllocatorDefault,
                                                                  0)) != nil)
                {
                    // There is => request the driver to enable it for us
                    if (IORegistryEntrySetCFProperty((io_registry_entry_t)service,
                                                     CFSTR("EnableAluminumRemoteSupportForMe"),
                                                     (__bridge void *)@{@"pid": @((long long)getpid()),
                                                      @"uid": @((long long)getuid())}) == kIOReturnSuccess)
                    {
                        if (CFGetTypeID(aluSupport) == CFNumberGetTypeID())
                        {
                            supportLevel = (HIDRemoteAluminumRemoteSupportLevel) [(__bridge_transfer NSNumber *)aluSupport intValue];
                        }
                        else
                        {
                            CFRelease(aluSupport);
                        }

                        hidAttribsDict[kHIDRemoteAluminumRemoteSupportOnDemand] = @YES;
                    }
                    else
                    {
                        CFRelease(aluSupport);
                    }
                }
            }

            if (supportLevel == kHIDRemoteAluminumRemoteSupportLevelNone)
            {
                if ((aluSupport = IORegistryEntryCreateCFProperty((io_registry_entry_t)service,
                                                                  (CFStringRef) @"AluminumRemoteSupportLevel",
                                                                  kCFAllocatorDefault,
                                                                  0)) != nil)
                {
                    if (CFGetTypeID(aluSupport) == CFNumberGetTypeID())
                    {
                        supportLevel = (HIDRemoteAluminumRemoteSupportLevel) [(__bridge_transfer NSNumber *)aluSupport intValue];
                    }
                    else
                    {
                        CFRelease(aluSupport);
                    }
                }
                else
                {
                    CFStringRef ioKitClassName;

                    if ((ioKitClassName = IORegistryEntryCreateCFProperty(  (io_registry_entry_t)service,
                                                                          CFSTR(kIOClassKey),
                                                                          kCFAllocatorDefault,
                                                                          0)) != nil)
                    {
                        if ([(__bridge_transfer NSString *)ioKitClassName isEqual:@"AppleIRController"])
                        {
                          supportLevel = kHIDRemoteAluminumRemoteSupportLevelNative;
                        }
                    }
                }
            }

            hidAttribsDict[kHIDRemoteAluminumRemoteSupportLevel] = (NSNumber *)@((int)supportLevel);
        }

        // Add it to the serviceAttribMap
        _serviceAttribMap[@((unsigned int)service)] = hidAttribsDict;

        // And we're done with setup ..
        if (([self delegate]!=nil) &&
            ([[self delegate] respondsToSelector:@selector(hidRemote:foundNewHardwareWithAttributes:)]))
        {
            [((NSObject <HIDRemoteDelegate> *)[self delegate]) hidRemote:self foundNewHardwareWithAttributes:hidAttribsDict];
        }

        hidAttribsDict = nil;

        return(YES);

    }while(0);

cleanUp:

    if (([self delegate]!=nil) &&
        ([[self delegate] respondsToSelector:@selector(hidRemote:failedNewHardwareWithError:)]))
    {
        if (error!=nil)
        {
            error = [NSError errorWithDomain:[error domain]
                                        code:[error code]
                                    userInfo:@{@"InternalErrorCode": [NSNumber numberWithInt:errorCode]}
                     ];
        }

        [((NSObject <HIDRemoteDelegate> *)[self delegate]) hidRemote:self failedNewHardwareWithError:error];
    }

    // An error occured or this device is not of interest .. cleanup ..
    if (serviceNotification!=0)
    {
        IOObjectRelease(serviceNotification);
        serviceNotification = 0;
    }

    if (queueEventSource!=NULL)
    {
        CFRunLoopSourceInvalidate(queueEventSource);
        queueEventSource=NULL;
    }

    if (hidQueueInterface!=NULL)
    {
        if (queueStarted)
        {
            (*hidQueueInterface)->stop(hidQueueInterface);
        }
        (*hidQueueInterface)->dispose(hidQueueInterface);
        (*hidQueueInterface)->Release(hidQueueInterface);
        hidQueueInterface = NULL;
    }

    hidAttribsDict = nil;

    if (hidElements!=NULL)
    {
        CFRelease(hidElements);
        hidElements = NULL;
    }

    if (hidDeviceInterface!=NULL)
    {
        if (opened)
        {
            (*hidDeviceInterface)->close(hidDeviceInterface);
        }
        (*hidDeviceInterface)->Release(hidDeviceInterface);
        // opened = NO;
        hidDeviceInterface = NULL;
    }

    if (cfPluginInterface!=NULL)
    {
        IODestroyPlugInInterface(cfPluginInterface);
        cfPluginInterface = NULL;
    }

    return (NO);
}

- (void)_destructService:(io_object_t)service
{
    NSNumber        *serviceValue;
    NSMutableDictionary *serviceDict = NULL;

    if ((serviceValue = @((unsigned int)service)) == nil)
    {
        return;
    }

    serviceDict  = _serviceAttribMap[serviceValue];

    if (serviceDict!=nil)
    {
        IOHIDDeviceInterface122  **hidDeviceInterface   = NULL;
        IOCFPlugInInterface  **cfPluginInterface    = NULL;
        IOHIDQueueInterface  **hidQueueInterface    = NULL;
        io_object_t      serviceNotification    = 0;
        CFRunLoopSourceRef   queueEventSource   = NULL;
        io_object_t      theService     = 0;
        NSMutableDictionary  *cookieButtonMap   = nil;
        NSTimer          *simulateHoldTimer = nil;

        serviceNotification = (io_object_t)         (serviceDict[kHIDRemoteServiceNotification]   ? [serviceDict[kHIDRemoteServiceNotification] unsignedIntValue] :   0);
        theService      = (io_object_t)         (serviceDict[kHIDRemoteService]           ? [serviceDict[kHIDRemoteService]         unsignedIntValue] :   0);
        queueEventSource    = (CFRunLoopSourceRef)      (serviceDict[kHIDRemoteCFRunLoopSource]       ? [serviceDict[kHIDRemoteCFRunLoopSource]     pointerValue]     : NULL);
        hidQueueInterface   = (IOHIDQueueInterface **)      (serviceDict[kHIDRemoteHIDQueueInterface]     ? [serviceDict[kHIDRemoteHIDQueueInterface]   pointerValue]     : NULL);
        hidDeviceInterface  = (IOHIDDeviceInterface122 **)  (serviceDict[kHIDRemoteHIDDeviceInterface]    ? [serviceDict[kHIDRemoteHIDDeviceInterface]  pointerValue]     : NULL);
        cfPluginInterface   = (IOCFPlugInInterface **)      (serviceDict[kHIDRemoteCFPluginInterface]     ? [serviceDict[kHIDRemoteCFPluginInterface]   pointerValue]     : NULL);
        cookieButtonMap     = (NSMutableDictionary *)        serviceDict[kHIDRemoteCookieButtonCodeLUT];
        simulateHoldTimer   = (NSTimer *)            serviceDict[kHIDRemoteSimulateHoldEventsTimer];

        [_serviceAttribMap removeObjectForKey:serviceValue];

        if ((serviceDict[kHIDRemoteAluminumRemoteSupportOnDemand]!=nil) && [serviceDict[kHIDRemoteAluminumRemoteSupportOnDemand] boolValue] && (theService != 0))
        {
            // We previously requested the driver to enable Aluminum Remote support for us. Tell it to turn it off again - now that we no longer need it
            IORegistryEntrySetCFProperty((io_registry_entry_t)theService,
                                         CFSTR("DisableAluminumRemoteSupportForMe"),
                                         (__bridge void *)@{@"pid": @((long long)getpid()),
                                          @"uid": @((long long)getuid())});
        }

        if (([self delegate]!=nil) &&
            ([[self delegate] respondsToSelector:@selector(hidRemote:releasedHardwareWithAttributes:)]))
        {
            [((NSObject <HIDRemoteDelegate> *)[self delegate]) hidRemote:self releasedHardwareWithAttributes:serviceDict];
        }

        if (simulateHoldTimer!=nil)
        {
            [simulateHoldTimer invalidate];
        }

        if (serviceNotification!=0)
        {
            IOObjectRelease(serviceNotification);
        }

        if (queueEventSource!=NULL)
        {
            CFRunLoopRemoveSource(  CFRunLoopGetCurrent(),
                                  queueEventSource,
                                  kCFRunLoopCommonModes);
        }

        if ((hidQueueInterface!=NULL) && (cookieButtonMap!=nil))
        {
            for (NSNumber *cookie in [cookieButtonMap keyEnumerator])
            {
                if ((*hidQueueInterface)->hasElement(hidQueueInterface, (IOHIDElementCookie) [cookie unsignedIntValue]))
                {
                    (*hidQueueInterface)->removeElement(hidQueueInterface,
                                                        (IOHIDElementCookie) [cookie unsignedIntValue]);
                }
            };
        }

        if (hidQueueInterface!=NULL)
        {
            (*hidQueueInterface)->stop(hidQueueInterface);
            (*hidQueueInterface)->dispose(hidQueueInterface);
            (*hidQueueInterface)->Release(hidQueueInterface);
        }

        if (hidDeviceInterface!=NULL)
        {
            (*hidDeviceInterface)->close(hidDeviceInterface);
            (*hidDeviceInterface)->Release(hidDeviceInterface);
        }

        if (cfPluginInterface!=NULL)
        {
            IODestroyPlugInInterface(cfPluginInterface);
        }

        if (theService!=0)
        {
            IOObjectRelease(theService);
        }
    }
}


#pragma mark -- PRIVATE: HID Event handling --
- (void)_simulateHoldEvent:(NSTimer *)aTimer
{
    NSMutableDictionary *hidAttribsDict;
    NSTimer  *shTimer;
    NSNumber *shButtonCode;

    if ((hidAttribsDict = (NSMutableDictionary *)[aTimer userInfo]) != nil)
    {
        if (((shTimer      = hidAttribsDict[kHIDRemoteSimulateHoldEventsTimer]) != nil) &&
            ((shButtonCode = hidAttribsDict[kHIDRemoteSimulateHoldEventsOriginButtonCode]) != nil))
        {
            [shTimer invalidate];
            [hidAttribsDict removeObjectForKey:kHIDRemoteSimulateHoldEventsTimer];

            [self _sendButtonCode:(((HIDRemoteButtonCode)[shButtonCode unsignedIntValue])|kHIDRemoteButtonCodeHoldMask) isPressed:YES hidAttribsDict:hidAttribsDict];
        }
    }
}

- (void)_handleButtonCode:(HIDRemoteButtonCode)buttonCode isPressed:(BOOL)isPressed hidAttribsDict:(NSMutableDictionary *)hidAttribsDict
{
    switch (buttonCode)
    {
        case kHIDRemoteButtonCodeIDChanged:
            // Do nothing, this is handled seperately
            break;

        case kHIDRemoteButtonCodeUp:
        case kHIDRemoteButtonCodeDown:
            if (_simulateHoldEvents)
            {
                NSTimer  *shTimer = nil;
                NSNumber *shButtonCode = nil;

                [hidAttribsDict[kHIDRemoteSimulateHoldEventsTimer] invalidate];

                if (isPressed)
                {
                    hidAttribsDict[kHIDRemoteSimulateHoldEventsOriginButtonCode] = [NSNumber numberWithUnsignedInt:buttonCode];

                    if ((shTimer = [[NSTimer alloc] initWithFireDate:[NSDate dateWithTimeIntervalSinceNow:0.7] interval:0.1 target:self selector:@selector(_simulateHoldEvent:) userInfo:hidAttribsDict repeats:NO]) != nil)
                    {
                        hidAttribsDict[kHIDRemoteSimulateHoldEventsTimer] = shTimer;

                        [[NSRunLoop currentRunLoop] addTimer:shTimer forMode:NSRunLoopCommonModes];

                        break;
                    }
                }
                else
                {
                    shTimer      = hidAttribsDict[kHIDRemoteSimulateHoldEventsTimer];
                    shButtonCode = hidAttribsDict[kHIDRemoteSimulateHoldEventsOriginButtonCode];

                    if ((shTimer!=nil) && (shButtonCode!=nil))
                    {
                        [self _sendButtonCode:(HIDRemoteButtonCode)[shButtonCode unsignedIntValue] isPressed:YES hidAttribsDict:hidAttribsDict];
                        [self _sendButtonCode:(HIDRemoteButtonCode)[shButtonCode unsignedIntValue] isPressed:NO hidAttribsDict:hidAttribsDict];
                    }
                    else
                    {
                        if (shButtonCode!=nil)
                        {
                            [self _sendButtonCode:(((HIDRemoteButtonCode)[shButtonCode unsignedIntValue])|kHIDRemoteButtonCodeHoldMask) isPressed:NO hidAttribsDict:hidAttribsDict];
                        }
                    }
                }

                [hidAttribsDict removeObjectForKey:kHIDRemoteSimulateHoldEventsTimer];
                [hidAttribsDict removeObjectForKey:kHIDRemoteSimulateHoldEventsOriginButtonCode];

                break;
            }

        default:
            [self _sendButtonCode:buttonCode isPressed:isPressed hidAttribsDict:hidAttribsDict];
            break;
    }
}

- (void)_sendButtonCode:(HIDRemoteButtonCode)buttonCode isPressed:(BOOL)isPressed hidAttribsDict:(NSMutableDictionary *)hidAttribsDict
{
    if (([self delegate]!=nil) &&
        ([[self delegate] respondsToSelector:@selector(hidRemote:eventWithButton:isPressed:fromHardwareWithAttributes:)]))
    {
        switch (buttonCode & (~kHIDRemoteButtonCodeAluminumMask))
        {
            case kHIDRemoteButtonCodePlay:
            case kHIDRemoteButtonCodeCenter:
                if (buttonCode & kHIDRemoteButtonCodeAluminumMask)
                {
                    _lastSeenModel         = kHIDRemoteModelAluminum;
                    _lastSeenModelRemoteID = _lastSeenRemoteID;
                }
                else
                {
                    switch ((HIDRemoteAluminumRemoteSupportLevel)[hidAttribsDict[kHIDRemoteAluminumRemoteSupportLevel] intValue])
                    {
                        case kHIDRemoteAluminumRemoteSupportLevelNone:
                        case kHIDRemoteAluminumRemoteSupportLevelEmulation:
                            // Remote type can't be determined by just the Center button press
                            break;

                        case kHIDRemoteAluminumRemoteSupportLevelNative:
                            // Remote type can be safely determined by just the Center button press
                            if (((_lastSeenModel == kHIDRemoteModelAluminum) && (_lastSeenModelRemoteID != _lastSeenRemoteID)) ||
                                (_lastSeenModel == kHIDRemoteModelUndetermined))
                            {
                                _lastSeenModel = kHIDRemoteModelWhitePlastic;
                            }
                            break;
                    }
                }
                break;
        }

        // As soon as we have received a code that's unique to the Aluminum Remote, we can tell kHIDRemoteButtonCodePlayHold and kHIDRemoteButtonCodeCenterHold apart.
        // Prior to that, a long press of the new "Play" button will be submitted as a "kHIDRemoteButtonCodeCenterHold", not a "kHIDRemoteButtonCodePlayHold" code.
        if ((buttonCode == kHIDRemoteButtonCodeCenterHold) && (_lastSeenModel == kHIDRemoteModelAluminum))
        {
            buttonCode = kHIDRemoteButtonCodePlayHold;
        }

        [((NSObject <HIDRemoteDelegate> *)[self delegate]) hidRemote:self eventWithButton:(buttonCode & (~kHIDRemoteButtonCodeAluminumMask)) isPressed:isPressed fromHardwareWithAttributes:hidAttribsDict];
    }
}

- (void)_hidEventFor:(io_service_t)hidDevice from:(IOHIDQueueInterface **)interface withResult:(IOReturn)result
{
    NSMutableDictionary *hidAttribsDict = _serviceAttribMap[@((unsigned int)hidDevice)];

    if (hidAttribsDict!=nil)
    {
        IOHIDQueueInterface **queueInterface  = NULL;

        queueInterface  = [hidAttribsDict[kHIDRemoteHIDQueueInterface] pointerValue];

        if (interface == queueInterface)
        {
            NSNumber        *lastButtonPressedNumber = nil;
            HIDRemoteButtonCode  lastButtonPressed = kHIDRemoteButtonCodeNone;
            NSMutableDictionary *cookieButtonMap = nil;

            cookieButtonMap  = hidAttribsDict[kHIDRemoteCookieButtonCodeLUT];

            if ((lastButtonPressedNumber = hidAttribsDict[kHIDRemoteLastButtonPressed]) != nil)
            {
                lastButtonPressed = [lastButtonPressedNumber unsignedIntValue];
            }

            while (result == kIOReturnSuccess)
            {
                IOHIDEventStruct hidEvent;
                AbsoluteTime supportedTime = { 0,0 };

                result = (*queueInterface)->getNextEvent(   queueInterface,
                                                         &hidEvent,
                                                         supportedTime,
                                                         0);

                if (result == kIOReturnSuccess)
                {
                    NSNumber *buttonCodeNumber = cookieButtonMap[@((unsigned int) hidEvent.elementCookie)];

#ifdef _HIDREMOTE_EXTENSIONS
                    // Debug logging code
#define _HIDREMOTE_EXTENSIONS_SECTION 5
#include "HIDRemoteAdditions.h"
#undef _HIDREMOTE_EXTENSIONS_SECTION
#endif /* _HIDREMOTE_EXTENSIONS */

                    if (buttonCodeNumber!=nil)
                    {
                        HIDRemoteButtonCode buttonCode = [buttonCodeNumber unsignedIntValue];

                        if (hidEvent.value == 0)
                        {
                            if (buttonCode == lastButtonPressed)
                            {
                                [self _handleButtonCode:lastButtonPressed isPressed:NO hidAttribsDict:hidAttribsDict];
                                lastButtonPressed = kHIDRemoteButtonCodeNone;
                            }
                        }

                        if (hidEvent.value != 0)
                        {
                            if (lastButtonPressed != kHIDRemoteButtonCodeNone)
                            {
                                [self _handleButtonCode:lastButtonPressed isPressed:NO hidAttribsDict:hidAttribsDict];
                                // lastButtonPressed = kHIDRemoteButtonCodeNone;
                            }

                            if (buttonCode == kHIDRemoteButtonCodeIDChanged)
                            {
                                if (([self delegate]!=nil) &&
                                    ([[self delegate] respondsToSelector:@selector(hidRemote:remoteIDChangedOldID:newID:forHardwareWithAttributes:)]))
                                {
                                    [((NSObject <HIDRemoteDelegate> *)[self delegate]) hidRemote:self remoteIDChangedOldID:_lastSeenRemoteID newID:hidEvent.value forHardwareWithAttributes:hidAttribsDict];
                                }

                                _lastSeenRemoteID = hidEvent.value;
                                _lastSeenModel    = kHIDRemoteModelUndetermined;
                            }

                            [self _handleButtonCode:buttonCode isPressed:YES hidAttribsDict:hidAttribsDict];
                            lastButtonPressed = buttonCode;
                        }
                    }
                }
            };

            hidAttribsDict[kHIDRemoteLastButtonPressed] = [NSNumber numberWithUnsignedInt:lastButtonPressed];
        }

#ifdef _HIDREMOTE_EXTENSIONS
        // Debug logging code
#define _HIDREMOTE_EXTENSIONS_SECTION 6
#include "HIDRemoteAdditions.h"
#undef _HIDREMOTE_EXTENSIONS_SECTION
#endif /* _HIDREMOTE_EXTENSIONS */
    }
}

#pragma mark -- PRIVATE: Notification handling --
- (void)_serviceMatching:(io_iterator_t)iterator
{
    io_object_t matchingService = 0;

    while ((matchingService = IOIteratorNext(iterator)) != 0)
    {
        [self _setupService:matchingService];

        IOObjectRelease(matchingService);
    };
}

- (void)_serviceNotificationFor:(io_service_t)service messageType:(natural_t)messageType messageArgument:(void *)messageArgument
{
    if (messageType == kIOMessageServiceIsTerminated)
    {
        [self _destructService:service];
    }
}

- (void)_updateSessionInformation
{
    NSArray *consoleUsersArray;
    io_service_t rootService;

    if (_masterPort==0) { return; }

    if ((rootService = IORegistryGetRootEntry(_masterPort)) != 0)
    {
        if ((consoleUsersArray = (__bridge_transfer NSArray *)IORegistryEntryCreateCFProperty((io_registry_entry_t)rootService, CFSTR("IOConsoleUsers"), kCFAllocatorDefault, 0)) != nil)
        {
            if ([consoleUsersArray isKindOfClass:[NSArray class]])  // Be careful - ensure this really is an array
            {
                UInt64 secureEventInputPIDSum = 0;
                uid_t frontUserSession = 0;

                for (NSDictionary *consoleUserDict in consoleUsersArray)
                {
                    if ([consoleUserDict isKindOfClass:[NSDictionary class]]) // Be careful - ensure this really is a dictionary
                    {
                        NSNumber *secureInputPID;
                        NSNumber *onConsole;
                        NSNumber *userID;

                        if ((secureInputPID = consoleUserDict[@"kCGSSessionSecureInputPID"]) != nil)
                        {
                            if ([secureInputPID isKindOfClass:[NSNumber class]])
                            {
                                secureEventInputPIDSum += ((UInt64) [secureInputPID intValue]);
                            }
                        }

                        if (((onConsole = consoleUserDict[@"kCGSSessionOnConsoleKey"]) != nil) &&
                            ((userID    = consoleUserDict[@"kCGSSessionUserIDKey"]) != nil))
                        {
                            if ([onConsole isKindOfClass:[NSNumber class]] && [userID isKindOfClass:[NSNumber class]])
                            {
                                if ([onConsole boolValue])
                                {
                                    frontUserSession = (uid_t) [userID intValue];
                                }
                            }
                        }
                    }
                }

                _lastSecureEventInputPIDSum = secureEventInputPIDSum;
                _lastFrontUserSession       = frontUserSession;
            }
        }

        IOObjectRelease((io_object_t) rootService);
    }
}

- (void)_secureInputNotificationFor:(io_service_t)service messageType:(natural_t)messageType messageArgument:(void *)messageArgument
{
    if (messageType == kIOMessageServiceBusyStateChange)
    {
        UInt64 old_lastSecureEventInputPIDSum = _lastSecureEventInputPIDSum;
        uid_t  old_lastFrontUserSession = _lastFrontUserSession;

        [self _updateSessionInformation];

        if (((old_lastSecureEventInputPIDSum != _lastSecureEventInputPIDSum) || (old_lastFrontUserSession != _lastFrontUserSession)) && _secureEventInputWorkAround)
        {
            if ((_mode == kHIDRemoteModeExclusive) || (_mode == kHIDRemoteModeExclusiveAuto))
            {
                HIDRemoteMode restartInMode = _mode;

                _isRestarting = YES;
                [self stopRemoteControl];
                [self startRemoteControl:restartInMode];
                _isRestarting = NO;
            }
        }
    }
}

@end

#pragma mark -- PRIVATE: IOKitLib Callbacks --

static void HIDEventCallback(   void * target,
                             IOReturn result,
                             void * refCon,
                             void * sender)
{
    HIDRemote       *hidRemote = (__bridge HIDRemote *)refCon;

    @autoreleasepool {
        [hidRemote _hidEventFor:(io_service_t)((intptr_t)target) from:(IOHIDQueueInterface**)sender withResult:(IOReturn)result];
    }
}


static void ServiceMatchingCallback(    void *refCon,
                                    io_iterator_t iterator)
{
    HIDRemote       *hidRemote = (__bridge HIDRemote *)refCon;

    @autoreleasepool {
        [hidRemote _serviceMatching:iterator];
    }
}

static void ServiceNotificationCallback(void *      refCon,
                                        io_service_t    service,
                                        natural_t   messageType,
                                        void *      messageArgument)
{
    HIDRemote       *hidRemote = (__bridge HIDRemote *)refCon;

    @autoreleasepool {
        [hidRemote _serviceNotificationFor:service
                               messageType:messageType
                           messageArgument:messageArgument];
    }
}

static void SecureInputNotificationCallback(    void *      refCon,
                                            io_service_t    service,
                                            natural_t   messageType,
                                            void *      messageArgument)
{
    HIDRemote       *hidRemote = (__bridge HIDRemote *)refCon;

    @autoreleasepool {
        [hidRemote _secureInputNotificationFor:service
                                   messageType:messageType
                               messageArgument:messageArgument];
    }
}

// Attribute dictionary keys
NSString *kHIDRemoteCFPluginInterface           = @"CFPluginInterface";
NSString *kHIDRemoteHIDDeviceInterface          = @"HIDDeviceInterface";
NSString *kHIDRemoteCookieButtonCodeLUT         = @"CookieButtonCodeLUT";
NSString *kHIDRemoteHIDQueueInterface           = @"HIDQueueInterface";
NSString *kHIDRemoteServiceNotification         = @"ServiceNotification";
NSString *kHIDRemoteCFRunLoopSource         = @"CFRunLoopSource";
NSString *kHIDRemoteLastButtonPressed           = @"LastButtonPressed";
NSString *kHIDRemoteService             = @"Service";
NSString *kHIDRemoteSimulateHoldEventsTimer     = @"SimulateHoldEventsTimer";
NSString *kHIDRemoteSimulateHoldEventsOriginButtonCode  = @"SimulateHoldEventsOriginButtonCode";
NSString *kHIDRemoteAluminumRemoteSupportLevel      = @"AluminumRemoteSupportLevel";
NSString *kHIDRemoteAluminumRemoteSupportOnDemand   = @"AluminumRemoteSupportLevelOnDemand";

NSString *kHIDRemoteManufacturer            = @"Manufacturer";
NSString *kHIDRemoteProduct             = @"Product";
NSString *kHIDRemoteTransport               = @"Transport";

// Distributed notifications
NSString *kHIDRemoteDNHIDRemotePing         = @"com.candelair.ping";
NSString *kHIDRemoteDNHIDRemoteRetry            = @"com.candelair.retry";
NSString *kHIDRemoteDNHIDRemoteStatus           = @"com.candelair.status";

NSString *kHIDRemoteDNHIDRemoteRetryGlobalObject    = @"global";

// Distributed notifications userInfo keys and values
NSString *kHIDRemoteDNStatusHIDRemoteVersionKey     = @"HIDRemoteVersion";
NSString *kHIDRemoteDNStatusPIDKey          = @"PID";
NSString *kHIDRemoteDNStatusModeKey         = @"Mode";
NSString *kHIDRemoteDNStatusUnusedButtonCodesKey    = @"UnusedButtonCodes";
NSString *kHIDRemoteDNStatusActionKey           = @"Action";
NSString *kHIDRemoteDNStatusRemoteControlCountKey   = @"RemoteControlCount";
NSString *kHIDRemoteDNStatusReturnToPIDKey      = @"ReturnToPID";
NSString *kHIDRemoteDNStatusActionStart         = @"start";
NSString *kHIDRemoteDNStatusActionStop          = @"stop";
NSString *kHIDRemoteDNStatusActionUpdate        = @"update";
NSString *kHIDRemoteDNStatusActionNoNeed        = @"noneed";
