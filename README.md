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
