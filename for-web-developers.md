# Jellyfin Media Player Implementation

## Detection

You can detect JMP by looking for the object `window.jmpInfo`, which is defined immediately.

## Device Profile

You can query a device profile for Jellyfin Media Player with:

```js
window.NativeShell.AppHost.getDeviceProfile()
```

This is not async. It can be queried immediately.

## Other NativeShell Functions

The following functions are available immediately:

 - `window.NativeShell.AppHost.getDefaultLayout()` - Returns `tv` or `desktop`.
 - `window.NativeShell.AppHost.appVersion()` - Returns version number in `x.y.z` format.
 - `window.NativeShell.AppHost.deviceName()` - Returns computer hostname.

The following functions are available after less than a second:

 - `window.NativeShell.openUrl(url)` - Opens a URL in the user's browser.
 - `window.NativeShell.openClientSettings()` - Opens prebuilt settings modal.
     - This uses vanilla JS and can be styled with classes.
     - You may want to use the settings API directly though.
 - `window.NativeShell.AppHost.exit()` - Closes the application.

## Media Playback

See [mpvVideoPlayer.js](https://github.com/jellyfin/jellyfin-media-player/blob/master/native/mpvVideoPlayer.js) for a complete example of how to connect and control the player.

Notable API methods:

 - `api.power.setScreensaverEnabled(enabled: boolean)` - Enable or disable screensaver.
 - `api.player.load(url, positiondata, streamdata, audioStream, subtitleStream)` - Play media. See below.
 - `api.player.setSubtitleStream(subtitleStream)` - Set subtitle stream. See below.
 - `api.player.setSubtitleDelay(msDelay: int)` - Set subtitle delay.
 - `api.player.setAudioStream(audioStream)` - Set audio stream. See below.
 - `api.player.stop()` - Stop playback.
 - `api.player.seekTo(positionMs: int)` - Seek to absolute position in milliseconds.
 - `api.player.getPosition((positionMs: int) => void)` - Get current position in milliseconds.
 - `api.player.pause()` - Pause playback.
 - `api.player.play()` - Resume playback.
 - `api.player.setPlaybackRate(playbackRate: int)` - Set playback rate. 1000 is normal speed.
 - `api.player.setVolume(volume: int)` - Set volume. 100 is full volume.
 - `api.player.setMuted(muted: boolean)` - Set if player should be muted.

Notable events (see API info below for how to connect/disconnect):

 - `api.player.playing: void` - Fires when playback starts.
 - `api.player.positionUpdate: int` - Fires periodically to update current position in milliseconds.
 - `api.player.finished: void` - Fires when playback finishes.
 - `api.player.updateDuration: int` - Fires to provide duration in milliseconds when available.
 - `api.player.error: string` - Fires if media playback fails.
 - `api.player.paused: void` - Fires when playback is paused.

Loading media:

```js
api.player.load(
    url,
    {
        startMilliseconds: ms,
        autoplay: true
    },
    streamdata,
    audioStream,
    subtitleStream,
    callback
)
```

Note: The callback does NOT mean the video actually finished loading. You need to wait for events to determine this.

`streamdata`:
```js
{
    type: 'video',  // use 'music' to disable video rendering
    headers: {
        'User-Agent': 'JellyfinMediaPlayer'
    },
    metadata: {
        // Only used for system UI integration, eg. taskbar/windows media.
        // Optional. Comes from Jellyfin metadata item.
        // Known used items are documented below.
        Id: 'media uuid',
        MediaType: 'video',
        ImageTags: {
            Primary: 'uuid' // only if photo is available
        },
        AlbumId: 'uuid', // music only
        AlbumPrimaryImageTag: 'uuid', // music only
        Name: 'media title',
        Type: 'Episode', // or other type
        SeriesName: 'tv show name', // only if episode
        Artists: ['artist1', 'artist2...'], // music only
        AlbumArtist: 'artist' // music only
    },
    media: {}
}
```

`audioStream` is `#{index}` (It starts at 1, for instance `#1`. The index is relative and only includes audio tracks.)

`subtitleStream` is:

 - empty string - disables subtitles
 - `#{index}` - subtitles embedded in file (It starts at 1, for instance `#1`. The index is relative and only includes subtitles tracks.)
 - `#,{url}` - external subtitle to load from remote url (for instance `#,https://example.com/file.srt`)

## Remote Control

You can recieve input from remote controls and gamepads via the input component. There is also an unused
"TV" mode which can be set in the config. It disables keyboard into the webapp and redirects it through
this as well.

See [jmpInputPlugin.js](https://github.com/jellyfin/jellyfin-media-player/blob/master/native/jmpInputPlugin.js) for an example which picks up events and translates them to ones that `jellyfin-web` understands.

```js
api.input.hostInput.connect((actions) => {
    actions.forEach(action => {
        // action is a string which represents the action
    });
});

api.system.hello("jmpInputPlugin"); // signals to start sending user input
```

The inputs which are actually processed by `jellyfin-web` are fairly limited, but there are many more possible events.
See the [inputmaps](https://github.com/jellyfin/jellyfin-media-player/tree/master/resources/inputmaps) folder for event names for various input devices.

## Settings

The application retains some user settings that control advanced functions, such as audio passthrough.
There is already [a modal](https://github.com/jellyfin/jellyfin-media-player/blob/7d5943becc1ca672d599887cac9107836c38d337/native/nativeshell.js#L189-L308) you can invoke display of which allows basic configuration changes.

The configuration options are defined here: [settings_description.json](https://github.com/jellyfin/jellyfin-media-player/blob/master/resources/settings/settings_description.json)

Current Settings:

 - Main Section (`main`):
     - `fullscreen: boolean`: Forces the application into fullscreen mode.
         - Note: Regular HTML5 fullscreen does still work.
     - `alwaysOnTop: boolean`: Keeps JMP on top of other windows.
     - `useOpenGL: boolean`: Windows only. Controls display method.
     - `forceFSScreen: string`: Forces fullscreen to use a specific display.
         - The options for this are automatically generated.
     - `checkForUpdates: boolean`: Allows user to disable update check plugin script.
         - This has no effect if you don't use the update script.
     - `enableInputRepeat: boolean`: Allows disabling repeating of control inputs.
     - `forceExternalWebclient: boolean`: Allows the user to connect to external webclients even if the bundled one is available.
         - This has not landed in a released version yet.
     - `userWebClient: string`: Hidden option. Set it back to an empty string to allow the user to select a different webclient path.
 - Plugins Section (`plugins`):
     - `skipintro: boolean`: Enables or disabled the plugin. You likely won't use this.
 - Audio Section (`audio`):
     - `devicetype: string enum`: Sets the device type. Can be `basic`, `spdif`, or `hdmi`.
         - `basic` disables passthrough
         - `spdif` transcodes surround sound to specific formats
         - `hdmi` supports additional passthrough options
     - `channels: string enum`: Sets how many channels are allowed. The default is `stereo` except on MacOS where it is `auto`.
         - `auto` - Automatically set.
         - `2.0` - Stereo
         - `5.1,2.0` - Up to 5.1 surround
         - `7.1,5.1,2.0` - Up to 7.1 surround
     - `device: string`: Allows forcing media output to a specific device. The default is `auto`.
         - The options for this are automatically generated.
     - `normalize: boolean`: Enable audio volume normalization.
     - `exclusive: boolean`: Takes exclusive control of audio device. The default is `false`.
         - This is only available on MacOS and Windows.
     - `passthrough.ac3: boolean`: Enable ac3 passthrough. Requires `spdif` or `hdmi` device type. Default: `false`
     - `passthrough.dts: boolean`: Enable dts passthrough. Requires `spdif` or `hdmi` device type. Default: `false`
     - `passthrough.eac3: boolean`: Enable ac3 passthrough. Requires `hdmi` device type. Default: `false`
     - `passthrough.dts-hd": boolean`: Enable ac3 passthrough. Requires `hdmi` device type. Default: `false`
     - `passthrough.truehd: boolean`: Enable ac3 passthrough. Requires `hdmi` device type. Default: `false`
 - Video Section (`video`):
     - `refreshrate.auto_switch: boolean`: Sets the display refresh rate to the media refresh rate. Default: `false`
         - This is only enabled while the player is fullscreen.
     - `hardwareDecoding: string enum`: Sets the hardware decoding method. Default: `copy`
         - `enabled` - Full hardware acceleration. Removes MPV shader pipelines. Can cause issues more often.
         - `osx_compat` - Probably not useful given we only support GitHub Actions MacOS versions.
         - `copy` - Uses hardware decoding but copies the video through MPV's shader pipelines.
         - `disabled` - Completely disable hardware decoding. Use this if video is glitchy.
     - `deinterlace: boolean`: Enables deinterlacing. Default: `false`
     - `allow_transcode_to_hevc: boolean`: If set, it allows transcoding to `hevc` in the video profile. Default: `false`
         - This is mostly added as a workaround for Dolby Vision content direct playing when it shouldn't.
     - `force_transcode_hdr: boolean`: If set, HDR media support is dropped from the video profile. Default: `false`
     - `sync_mode: string enum`: Options are `audio`, `display-resample`, `display-adrop`. Not sure what this does.
     - `cache: int`: Controls cache size in MB for video streaming. The default is `75`.
         - Provided options are `10`, `75`, `150`, and `500`.
     - `aspect: string enum`: Allows control of aspect ratio. The default is `normal`.
         - Options: `normal`, `zoom`, `force_4_3`, `force_16_9`, `force_16_9_if_4_3`, `stretch`, `noscaling`, `custom`
 - Subtitle Section (`subtitles`):
     - `placement: string enum`: Controls where subtitles are displayed on the screen. Default: `center,bottom`
         - Provided options: [see enum](https://github.com/jellyfin/jellyfin-media-player/blob/7d5943becc1ca672d599887cac9107836c38d337/resources/settings/settings_description.json#L352-L359)
     - `color: string enum`: Controls colors of subtitles. Default: `#EEEEEE`
         - Provided options: [see enum](https://github.com/jellyfin/jellyfin-media-player/blob/master/resources/settings/settings_description.json#L421-L427)
     - `border_color: string enum`: Controls font border colors of subtitles. Default: `#000000`
         - Provided options: [see enum](https://github.com/jellyfin/jellyfin-media-player/blob/master/resources/settings/settings_description.json#L434-L440)
     - `background_color: string enum`: Controls backgroud colors of subtitles. Default: `#CCCCCC`
         - Provided options: [see enum](https://github.com/jellyfin/jellyfin-media-player/blob/master/resources/settings/settings_description.json#L447-L453)
     - `background_transparency: string enum`: Controls backgroud transparency (in hex) of subtitles. Default: `00`
         - Provided options: [see enum](https://github.com/jellyfin/jellyfin-media-player/blob/master/resources/settings/settings_description.json#L460-L464)
     - `size: int`: Controls subtitle size. Default is `32`.
         - Provided options: [see enum](https://github.com/jellyfin/jellyfin-media-player/blob/7d5943becc1ca672d599887cac9107836c38d337/resources/settings/settings_description.json#L376-L382)

The global `window.jmpInfo` object contains settings for the application in the form of `window.jmpInfo.settings.[section][key] = value`.
Settings descriptions are stored in the form `window.jmpInfo.settingsDescriptions.[section] = [{ key, options }]`

 - These are kept up-to-date in response to user changes.
 - If you want to change any settings, `await window.initCompleted` first.
 - To change a setting, simply set the setting to a new value. Don't overwrite an entire section.
 - You can subscribe to settings changes by adding a `callback(section, changes)` to `window.jmpInfo.settingsUpdate`.
 - Similarly, you can subscribe to settings description updates by adding a `callback(section, changes)` to `window.jmpInfo.settingsDescriptionsUpdate`.
     - This is useful for reactive settings, for instance when you change the audio device type.

You can also use the API to set and query settings. This is largely not needed unless you want to be sure a setting change is complete before you do something:

 - `api.settings.settingDescriptions(callback)`: Get list of all setting descriptions and options.
     - You may need to re-query this if you update certain settings.
 - `api.settings.allValues(section, callback)`: Get all settings for a specific section.
 - `api.settings.value(section, key, callback)`: Get a specific setting.
 - `api.settings.setValue(section, key, value, callback)`: Set a specific setting.

## Checking for Updates

```js
const updatePlugin = await window.jmpUpdatePlugin();
updatePlugin({
  toast: () => new Promise((resolve, reject) => {
    // called if an update is available
    // resolve - opens URL for update
    // reject - ignores update
  })
})
```

## Client API

To establish a connection to the Client API from scratch, use:

```js
function loadScript(src) {
    return new Promise((resolve, reject) => {
        const s = document.createElement('script');
        s.src = src;
        s.onload = resolve;
        s.onerror = reject;
        document.head.appendChild(s);
    });
}

async function createApi() {
    await loadScript('qrc:///qtwebchannel/qwebchannel.js');
    const channel = await new Promise((resolve) => {
        new QWebChannel(window.qt.webChannelTransport, resolve);
    });
    return channel.objects;
}
```

The injected utilities script makes `window.apiPromise` available immediately and `window.api` available
after less than a second.

### Client API Usage

The API is a [QWebChannel](https://doc.qt.io/qt-5/qtwebchannel-javascript.html) API which is async and callback-driven.

To invoke a function:

```js
await new Promise(resolve => {
    window.api.settings.setValue('main', 'userWebClient', '', resolve);
});
```

You can also fire and forget the functions by calling them directly and not passing a callback.

To listen and unlisten for an event:

```js
api.system.updateInfoEmitted.connect(myEventListener);
api.system.updateInfoEmitted.disconnect(myEventListener);
```

### Client API Overview

The client API corresponds to the methods in the application with `Q_INVOKABLE`.

 - `api.display` - [DisplayComponent.h](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/display/DisplayComponent.h) [DisplayComponent.cpp](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/display/DisplayComponent.cpp)
 - `api.input` - [InputComponent.h](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/input/InputComponent.h) [InputComponent.cpp](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/input/InputComponent.cpp)
 - `api.player` - [PlayerComponent.h](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/player/PlayerComponent.h) [PlayerComponent.cpp](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/player/PlayerComponent.cpp)
 - `api.power` - [PowerComponent.h](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/power/PowerComponent.h) [PowerComponent.cpp](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/power/PowerComponent.cpp)
 - `api.settings` - [SettingsComponent.h](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/settings/SettingsComponent.h)
 - `api.system` - [SystemComponent.h](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/system/SystemComponent.h) [SystemComponent.cpp](https://github.com/jellyfin/jellyfin-media-player/blob/master/src/system/SystemComponent.cpp)
 - `api.taskbar` - (this is empty)

For a (slightly outdated) list of functions, you can look here: https://github.com/jellyfin/jellyfin-media-player/blob/master/client-api.md