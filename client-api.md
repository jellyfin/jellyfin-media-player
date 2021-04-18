To connect to the API:

```html
  <script src="qrc:///qtwebchannel/qwebchannel.js"></script>
  <script>
      new QWebChannel(window.qt.webChannelTransport, function(channel) {
          // Called when API is ready
          window.channel = channel;
      });
  </script>
```

The API endpoints will be present on `window.channel.objects`. To call them, include a function in the call to recieve the callback data. (The calls are async.)

To connect to an event, call the `.connect` method on the event and pass a callback. To disconnect the callback, call the `.disconnect` method and pass the same callback.

An example call to play a video URL:

```js
window.channel.objects.player.load("your_video_file", {}, {type: "video", headers: {"User-Agent": "TestPlayer"}, startMilliseconds: 0, frameRate: 0, media: {}}, "", "", function(){window.channel.objects.player.play()});
```

# display
display/DisplayComponent - [header](https://github.com/plexinc/plex-media-player/blob/master/src/display/DisplayComponent.h), [implementation](https://github.com/plexinc/plex-media-player/blob/master/src/display/DisplayComponent.cpp)
## funcs:
- void monitorChange()
- bool initializeDisplayManager()
- bool restorePreviousVideoMode()
- void switchCommand(str command) - uses string command to set display modes
## events:
- refreshRateChanged(float rate)
# input
input/InputComponent - [header](https://github.com/plexinc/plex-media-player/blob/master/src/input/InputComponent.h), [implementation](https://github.com/plexinc/plex-media-player/blob/master/src/input/InputComponent.cpp)
## funcs:
- void executeActions(list[str] actions)
## events:
- receivedInput(str source, str keycode, keystate keystate)
- hostInput(list[str] actions)
## types:
- keystate: enum { KeyDown, KeyUp, KeyPressed }
# player
player/PlayerComponent - [header](https://github.com/plexinc/plex-media-player/blob/master/src/player/PlayerComponent.h), [implementation](https://github.com/plexinc/plex-media-player/blob/master/src/player/PlayerComponent.cpp)
## funcs:
- void setAudioConfiguration()
- void updateAudioDeviceList()
- void updateSubtitleSettings()
- void updateVideoSettings()
- bool load(str url, dict options, dict metadata, str audioStream="", str subtitleStream="")
    - options: dict { int startMilliseconds, bool autoplay }
    - metadata: dict { bool interlaced, float frameRate, str type }
        - type can be "video" or "music"
    - audioStream: "#" + index from mkv, or pass external url
    - subtitleStream: "#" + index from mkv, or pass external url
- void queueMedia(str url, dict options, dict metadata, str audioStream, str subtitleStream)
- void clearQueue()
- void seekTo(int ms)
- void stop()
- void streamSwitch()
- void pause()
- void play()
- void setVolume(int volume) - 0-100
- int volume()
- void setMuted(bool muted)
- bool muted()
- list[dict{str name, str description}] getAudioDeviceList()
- void setAudioDevice(str name)
- void setAudioStream(str audioStream)
- void setSubtitleStream(str subtitleStream)
    - subtitleStream: "#" + index from mkv, or pass external url
- void setAudioDelay(int ms)
- void setSubtitleDelay(int ms)
- void setPlaybackRate(int rate) - 1000 = normal speed
- int getPosition()
- void setVideoOnlyMode(bool enable) - hides webview
- bool checkCodecSupport(str codec) - can check for vc1 and mpeg2video
- list[codecdriver] installedCodecDrivers()
- list[str] installedDecoderCodecs() - returns names of supported codecs (eg h264)
- void userCommand(str command)
- void setVideoRectangle(int x, int y, int w, int h) - use all -1 to revert to default
## events:
- playing()
- buffering(float percent)
- paused()
- finished()
- canceled()
- error(str msg)
- stopped() - deprecated, listen for finished and canceled
- stateChanged(state newState, state oldState)
- videoPlaybackActive(bool active) - true if the video (or music) is actually playing
- windowVisible(bool visible)
- updateDuration(int ms) - duration of the file
- positionUpdate(int ms) - emitted twice a second
- onVideoRecangleChanged()
- onMpvEvents()
## types:
- codecdriver: see src/player/CodecsComponent.h > struct CodecDriver
- state: enum { finished, canceled, error, paused, playing, buffering }
# power
power/PowerComponent - [header](https://github.com/plexinc/plex-media-player/blob/master/src/power/PowerComponent.h), [implementation](https://github.com/plexinc/plex-media-player/blob/master/src/power/PowerComponent.cpp)
## funcs:
- bool checkCap(powercapabilities capability)
- bool canPowerOff()
- bool canReboot()
- bool canSuspend()
- bool canRelaunch() - true for OPENELEC only...
- int getPowerCapabilities() - always returns 0...
- bool PowerOff()
- bool Reboot()
- bool Suspend()
- void setScreensaverEnabled(bool enabled)
## events:
- screenSaverEnabled()
- screenSaverDisabled()
## types:
- powercapabilities - enum { PowerOff = 1, Reboot = 2, Suspend = 4, Relaunch = 8 }
# settings 
settings/SettingsComponent - [header](https://github.com/plexinc/plex-media-player/blob/master/src/settings/SettingsComponent.h), [implementation](https://github.com/plexinc/plex-media-player/blob/master/src/settings/SettingsComponent.cpp)
## funcs:
- void cycleSettingCommand(str args)
- void setSettingCommand(str args)
- void setValue(str section, str key, str value)
- void setValues(dict options)
- any value(str section, str key)
- any allValues(str section)
- void removeValue(str sectionOrKey)
- void resetToDefaultAll()
- coid resetToDefault(str sectionId)
- list settingDescriptions()
- str getWebClientUrl(bool desktop)
## events:
- groupUpdate(str section, any description) - Fired when a section's description is updated.
- sectionValueUpdate(str section, dict values) - Sends dictionary of updated mpv key:value pairs.
# system
system/SystemComponent - [header](https://github.com/plexinc/plex-media-player/blob/master/src/system/SystemComponent.h), [implementation](https://github.com/plexinc/plex-media-player/blob/master/src/system/SystemComponent.cpp)
## funcs:
- dict systemInformation()
- void exit()
- void restart()
- void info(str text)
- void setCursorVisibility(bool visible)
- str getUserAgent()
- str debugInformation()
- list[str] networkAddresses()
- void openExternalUrl(str url) - Opens in external browser.
- void runUserScript(str script) - Starts processes in the datadir + scripts/
- void hello(str version) - called by web client when loading done
- str getCapabilitiesString()
- void crashApp() - Dereferences a null pointer.......
## events:
- capabilitiesChanged(str capabilities)
- userInfoChanged()
- hostMessage(str message)
- settingsMessage(str setting, str value)
- scaleChanged(float scale)
