//
// Created by Tobias Hieta on 21/08/15.
//

#include "InputAppleMediaKeys.h"
#include "SPMediaKeyTap.h"
#include "QsLog.h"

@interface MediaKeysDelegate : NSObject
{
  SPMediaKeyTap* keyTap;
  InputAppleMediaKeys* input;
}
-(instancetype)initWithInput:(InputAppleMediaKeys*)input;
@end

@implementation MediaKeysDelegate

- (instancetype)initWithInput:(InputAppleMediaKeys*)input_
{
  self = [super init];
  if (self) {
    input = input_;
    keyTap = [[SPMediaKeyTap alloc] initWithDelegate:self];
    if ([SPMediaKeyTap usesGlobalMediaKeyTap])
      [keyTap startWatchingMediaKeys];
    else
      QLOG_WARN() << "Could not grab global media keys";
  }
  return self;
}

- (void)dealloc
{
  [super dealloc];
}

-(void)mediaKeyTap:(SPMediaKeyTap *)keyTap receivedMediaKeyEvent:(NSEvent *)event
{
  int keyCode = (([event data1] & 0xFFFF0000) >> 16);
  int keyFlags = ([event data1] & 0x0000FFFF);
  BOOL keyIsPressed = (((keyFlags & 0xFF00) >> 8)) == 0xA;

  QString keyPressed;

  if (keyIsPressed) {
    switch (keyCode) {
      case NX_KEYTYPE_PLAY:
        keyPressed = INPUT_KEY_PLAY;
        break;
      case NX_KEYTYPE_FAST:
        keyPressed = "KEY_FAST";
        break;
      case NX_KEYTYPE_REWIND:
        keyPressed = "KEY_REWIND";
        break;
      case NX_KEYTYPE_NEXT:
        keyPressed = INPUT_KEY_NEXT;
        break;
      case NX_KEYTYPE_PREVIOUS:
        keyPressed = INPUT_KEY_PREV;
        break;
      default:
        break;
        // More cases defined in hidsystem/ev_keymap.h
    }

    emit input->receivedInput("AppleMediaKeys", keyPressed);
  }
}

@end

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputAppleMediaKeys::initInput()
{
  m_delegate = [[MediaKeysDelegate alloc] initWithInput:this];
  return true;
}
