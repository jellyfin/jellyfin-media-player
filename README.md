## Building

You need:

* Qt 5.6 alpha
* cmake 3.0 or newer
* ninja is recommended for building

Special Qt requirements:

* On Windows, you must apply qt-patches/0003-Always-enable-viewport-stuff.patch for
  correct window scaling. Applying the patches in qt-patches/qt-5.6-alpha/ fixes
  some stability issues.
* On OSX, you should apply qt-patches/0002-qtbase-Don-t-show-the-menu-bar-at-all-in-lion-style-fullscr.patch
  to improve the user experience in fullscreen.
* You can try to use Qt 5.5, but then you also need to apply the following patches:
    qt-patches/0001-qtwebengine-Add-a-backgroundColor-property.patch
    qt-patches/0004-qtwebengine-transparency-window-creation.patch
  Without them, video playback will not work.

Get dependencies:

* scripts/fetch-binaries.py -p darwin-x86_64

If you're happy just building from the command line then run CMake for the ninja build tool:

* mkdir build ; cd build
* cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DQTROOT=/path/to/qt -DCMAKE_INSTALL_PREFIX=output ..

Build (ninja):

* ninja

Make a distributable package:

* ninja install (be patient, it's slow)

Or if you prefer working in Xcode, run CMake for the xcode build):

* mkdir build ; cd build
* cmake -GXcode -DQTROOT=/path/to/qt ..


### Debugging Web Client

You can run a locally hosted development version of the web app within the Konvergo application. If the main app window is open you can also run Chrome side by side to debug.

* Run the `grunt server:konvergo` from the `web-client` submodule. This will run a dev version of the web client
* Update the `starturl` in `~/Library/Application Support/Plex Media Player/Plex Media Player.conf` to point to `http://localhost:3333/app/dev-konvergo.html`
* Run the `Plex Media Player.app`
* Tail the `~/Library/Logs/Plex Media Player/Plex Media Player.log`, optionally grepped with `WebClient` to see `console.log`s
* Open Chrome and point to `http://localhost:3333/app/dev-konvergo.html`. This should open a Qt channel to the main `Plex Media Player.app` and function as normal - but with the ability to add breakpoints and inspect code
