//
// Created by Tobias Hieta on 21/08/15.
//

#include "InputAppleMediaKeys.h"
#include <QDebug>

#import <dlfcn.h>

#import <MediaPlayer/MediaPlayer.h>

@interface MediaKeysDelegate : NSObject
{
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
    MPRemoteCommandCenter* center = [MPRemoteCommandCenter sharedCommandCenter];
#define CONFIG_CMD(name) \
  [center.name ## Command addTarget:self action:@selector(gotCommand:)]
    CONFIG_CMD(play);
    CONFIG_CMD(pause);
    CONFIG_CMD(togglePlayPause);
    CONFIG_CMD(stop);
    CONFIG_CMD(nextTrack);
    CONFIG_CMD(previousTrack);
    CONFIG_CMD(seekForward);
    CONFIG_CMD(seekBackward);
    CONFIG_CMD(skipForward);
    CONFIG_CMD(skipBackward);
    [center.changePlaybackPositionCommand addTarget:self action:@selector(gotPlaybackPosition:)];
  }
  return self;
}

-(MPRemoteCommandHandlerStatus)gotCommand:(MPRemoteCommandEvent *)event
{
  QString keyPressed;
  MPRemoteCommand* command = [event command];

#define CMD(name) [MPRemoteCommandCenter sharedCommandCenter].name ## Command
  if (command == CMD(play)) {
    keyPressed = INPUT_KEY_PLAY;
  } else if (command == CMD(pause)) {
    keyPressed = INPUT_KEY_PAUSE;
  } else if (command == CMD(togglePlayPause)) {
    keyPressed = INPUT_KEY_PLAY_PAUSE;
  } else if (command == CMD(stop)) {
    keyPressed = INPUT_KEY_STOP;
  } else if (command == CMD(nextTrack)) {
    keyPressed = INPUT_KEY_NEXT;
  } else if (command == CMD(previousTrack)) {
    keyPressed = INPUT_KEY_PREV;
  } else {
    return MPRemoteCommandHandlerStatusCommandFailed;
  }

  emit input->receivedInput("AppleMediaKeys", keyPressed, InputBase::KeyPressed);
  return MPRemoteCommandHandlerStatusSuccess;
}

-(MPRemoteCommandHandlerStatus)gotPlaybackPosition:(MPChangePlaybackPositionCommandEvent *)event
{
  PlayerComponent::Get().seekTo(event.positionTime * 1000);
  return MPRemoteCommandHandlerStatusSuccess;
}

@end

// macOS private enum
enum {
    MRNowPlayingClientVisibilityUndefined = 0,
    MRNowPlayingClientVisibilityAlwaysVisible,
    MRNowPlayingClientVisibilityVisibleWhenBackgrounded,
    MRNowPlayingClientVisibilityNeverVisible
};

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputAppleMediaKeys::initInput()
{
  m_currentTime = 0;
  m_pendingUpdate = false;
  m_delegate = [[MediaKeysDelegate alloc] initWithInput:this];
  connect(&PlayerComponent::Get(), &PlayerComponent::stateChanged, this,
          &InputAppleMediaKeys::handleStateChanged);
  connect(&PlayerComponent::Get(), &PlayerComponent::positionUpdate, this,
          &InputAppleMediaKeys::handlePositionUpdate);
  connect(&PlayerComponent::Get(), &PlayerComponent::updateDuration, this,
          &InputAppleMediaKeys::handleUpdateDuration);
  connect(&PlayerComponent::Get(), &PlayerComponent::onMetaData, this,
          &InputAppleMediaKeys::handleUpdateMetaData);

  if (auto lib =
      dlopen("/System/Library/PrivateFrameworks/MediaRemote.framework/MediaRemote", RTLD_NOW))
  {
#define LOAD_FUNC(name) \
  name = (name ## Func)dlsym(lib, "MRMediaRemote" #name)
      LOAD_FUNC(SetNowPlayingVisibility);
      LOAD_FUNC(GetLocalOrigin);
      LOAD_FUNC(SetCanBeNowPlayingApplication);
      if (SetCanBeNowPlayingApplication)
        SetCanBeNowPlayingApplication(1);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static MPNowPlayingPlaybackState convertState(PlayerComponent::State newState)
{
  switch (newState) {
    case PlayerComponent::State::finished:
      return MPNowPlayingPlaybackStateStopped;
    case PlayerComponent::State::canceled:
    case PlayerComponent::State::error:
      return MPNowPlayingPlaybackStateInterrupted;
    case PlayerComponent::State::buffering:
    case PlayerComponent::State::paused:
      return MPNowPlayingPlaybackStatePaused;
    case PlayerComponent::State::playing:
      return MPNowPlayingPlaybackStatePlaying;
    default:
      return MPNowPlayingPlaybackStateUnknown;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleMediaKeys::handleStateChanged(PlayerComponent::State newState, PlayerComponent::State oldState)
{
  MPNowPlayingPlaybackState newMPState = convertState(newState);
  MPNowPlayingInfoCenter *center = [MPNowPlayingInfoCenter defaultCenter];
  NSMutableDictionary *playingInfo = [NSMutableDictionary dictionaryWithDictionary:center.nowPlayingInfo];
  [playingInfo setObject:[NSNumber numberWithDouble:(double)m_currentTime / 1000] forKey:MPNowPlayingInfoPropertyElapsedPlaybackTime];
  center.nowPlayingInfo = playingInfo;
  [MPNowPlayingInfoCenter defaultCenter].playbackState = newMPState;
  if (SetNowPlayingVisibility && GetLocalOrigin) {
    if (newState == PlayerComponent::State::finished || newState == PlayerComponent::State::canceled || newState == PlayerComponent::State::error)
      SetNowPlayingVisibility(GetLocalOrigin(), MRNowPlayingClientVisibilityNeverVisible);
    else if (newState == PlayerComponent::State::paused || newState == PlayerComponent::State::playing || newState == PlayerComponent::State::buffering)
      SetNowPlayingVisibility(GetLocalOrigin(), MRNowPlayingClientVisibilityAlwaysVisible);
  }

  m_pendingUpdate = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleMediaKeys::handlePositionUpdate(quint64 position)
{
  m_currentTime = position;

  if (m_pendingUpdate) {
    MPNowPlayingInfoCenter *center = [MPNowPlayingInfoCenter defaultCenter];
    NSMutableDictionary *playingInfo = [NSMutableDictionary dictionaryWithDictionary:center.nowPlayingInfo];
    [playingInfo setObject:[NSNumber numberWithDouble:(double)position / 1000] forKey:MPNowPlayingInfoPropertyElapsedPlaybackTime];
    center.nowPlayingInfo = playingInfo;
    m_pendingUpdate = false;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleMediaKeys::handleUpdateDuration(qint64 duration)
{
  MPNowPlayingInfoCenter *center = [MPNowPlayingInfoCenter defaultCenter];
  NSMutableDictionary *playingInfo = [NSMutableDictionary dictionaryWithDictionary:center.nowPlayingInfo];
  [playingInfo setObject:[NSNumber numberWithDouble:(double)duration / 1000] forKey:MPMediaItemPropertyPlaybackDuration];
  center.nowPlayingInfo = playingInfo;
  m_pendingUpdate = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputAppleMediaKeys::handleUpdateMetaData(const QVariantMap& meta)
{
  auto info = [NSMutableDictionary
  dictionaryWithDictionary:MPNowPlayingInfoCenter.defaultCenter.nowPlayingInfo];
  info[MPMediaItemPropertyTitle] = meta["Name"].toString().toNSString();

  if (meta["MediaType"] == QLatin1String{ "Video" })
  {
    info[MPNowPlayingInfoPropertyMediaType] = @(MPNowPlayingInfoMediaTypeVideo);
    if (meta["Type"] == QLatin1String{ "Episode" })
    {
      [info addEntriesFromDictionary:@{
        MPMediaItemPropertyArtist : meta["SeriesName"].toString().toNSString(),
        MPMediaItemPropertyMediaType : @(MPMediaTypeTVShow),
      }];
    }
    else
      info[MPMediaItemPropertyMediaType] = @(MPMediaTypeMovie);
  }
  else // audio most probably
  {
    [info addEntriesFromDictionary:@{
      MPMediaItemPropertyAlbumArtist : meta["AlbumArtist"].toString().toNSString(),
      MPMediaItemPropertyAlbumTitle : meta["Album"].toString().toNSString(),
      MPMediaItemPropertyArtist : meta["Artists"].toStringList().join(", ").toNSString(),
      MPMediaItemPropertyMediaType : @(MPMediaTypeMusic),
      MPNowPlayingInfoPropertyMediaType : @(MPNowPlayingInfoMediaTypeAudio),
    }];
  }

  MPNowPlayingInfoCenter.defaultCenter.nowPlayingInfo = info;
}
