# Jellyfin Media Player

Based on (but not affiliated with) [Plex Media Player](https://github.com/plexinc/plex-media-player).

Please see:
 - Corresponding web client: [Repo](https://github.com/iwalton3/jellyfin-web-jmp/) [Release](https://github.com/iwalton3/jellyfin-web-jmp/releases/)
 - [Web client patchset](https://github.com/jellyfin/jellyfin-web/compare/release-10.7.z...iwalton3:pmptest#files_bucket)
 - API Docs in [client-api.md](https://github.com/iwalton3/jellyfin-media-player/blob/master/client-api.md)

This build strips a lot of un-needed things from the player.

## Building at a glance (Linux)

```bash
sudo apt install autoconf automake libtool libharfbuzz-dev libfreetype6-dev libfontconfig1-dev libx11-dev libxrandr-dev libvdpau-dev libva-dev mesa-common-dev libegl1-mesa-dev yasm libasound2-dev libpulse-dev libuchardet-dev zlib1g-dev libfribidi-dev git libgnutls28-dev libgl1-mesa-dev libsdl2-dev cmake wget python g++
mkdir jmp; cd jmp
wget http://download.qt.io/official_releases/qt/5.9/5.9.7/qt-opensource-linux-x64-5.9.7.run
chmod +x qt-opensource-linux-x64-5.9.7.run
sudo ./qt-opensource-linux-x64-5.9.7.run
git clone https://github.com/mpv-player/mpv-build.git
cd mpv-build
echo --enable-libmpv-shared > mpv_options
echo --disable-cplayer >> mpv_options
./rebuild -j4
sudo ./install
sudo ldconfig
cd ~/jmp/
git clone git://github.com/iwalton3/jellyfin-media-player
cd jellyfin-media-player
mkdir build
cd build
wget https://github.com/iwalton3/jellyfin-web-jmp/releases/download/jwc-1.7.0/dist.zip
unzip dist.zip
cmake -DCMAKE_BUILD_TYPE=Debug  -DQTROOT=/opt/Qt5.9.7/5.9.7/gcc_64/ -DCMAKE_INSTALL_PREFIX=/usr/local/ ..
make -j4
sudo make install
rm -rf ~/jmp/
```

## Building

You need:

* Qt 5.9.5
* cmake 3.1 or newer
* ninja is recommended for building
* FFmpeg 3.x and mpv from github

## Building on Mac OS X

I probably broke this...

Configure

If you're happy just building from the command line then run CMake for the ninja build tool:

* Install ninja with homebrew:
  * ``brew install ninja``
* Install mpv and other dependencies with homebrew:
  * ``brew install mpv --with-shared --HEAD``
* ``mkdir build ; cd build``
* ``cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=output ..``

Build (ninja):

* ``ninja``

Make a distributable package:

* ``ninja install`` (be patient, it's slow)

Or if you prefer working in Xcode, run CMake for the xcode build):

* ``mkdir build ; cd build``
* ``cmake -GXcode ..``

## Building on Linux

Jellyfin Media Player officially requires Qt 5.9.5. Our users have indicated that Qt 5.10 and Qt 5.11 also builds correctly. However, Qt 5.11.0 and 5.11.1 versions are _incompatible_ and should be avoided. Qt 5.12 may work for you. Please consider building PMP within a dedicated VM or Docker container if your system has a Qt version installed newer than 5.9.5. Downgrading a system Qt will cause issues in other applications depending on a newer Qt version.

These instructions are for Ubuntu 16.04 LTS and up. They were tested on a fresh install without extra options and with auto-updates applied.

First, some preparations:

* ``sudo apt install autoconf automake libtool libharfbuzz-dev libfreetype6-dev libfontconfig1-dev
    libx11-dev libxrandr-dev libvdpau-dev libva-dev mesa-common-dev libegl1-mesa-dev
    yasm libasound2-dev libpulse-dev libuchardet-dev zlib1g-dev libfribidi-dev git
    libgnutls28-dev libgl1-mesa-dev libsdl2-dev cmake``
* ``mkdir jmp``
* ``cd jmp``

Systems not based on Debian/Ubuntu will have similar packages, but you'll need to figure out their names yourself.

### Downloading and installing Qt

If your distro provides a Qt 5.9.5 package, try to use it. Otherwise, download a supported Qt version from qt.io.

### Building mpv and ffmpeg

While most distros have FFmpeg and mpv packages, they're often outdated. It's recommended to build a current version, or to get them from 3rd party sources (some are listed on https://mpv.io/installation/).

Here are instructions how to build them locally. First you need to install some build prerequisites:

* ``git clone https://github.com/mpv-player/mpv-build.git``
* ``cd mpv-build``
* ``echo --enable-libmpv-shared > mpv_options``
* you can also add ``echo --disable-cplayer >> mpv_options`` to prevent mpv CLI from being built
* ``./rebuild -j4`` (this steps checks out all sources and compiles them and takes a while)
* ``sudo ./install``
* ``sudo ldconfig``

With this, libmpv should have been installed to ``/usr/local/``. It does not conflict with the system. In particular, it does not install or use FFmpeg libraries. (The FFmpeg libraries are statically linked in libmpv when using mpv-build.)

You can also attempt to skip the installation step, and change the paths in the PMP build step to the build directory, but this is more complicated.

### Building jellyfin-media-player

Assuming that everything else has installed correctly, building Jellyfin Media Player should now be fairly straightforward:

* ``cd ~/jmp/``
* ``git clone git://github.com/iwalton3/jellyfin-media-player``
* ``cd jellyfin-media-player``
* ``mkdir build``
* ``cd build``
* ``cmake -DCMAKE_BUILD_TYPE=Debug -DQTROOT=/opt/Qt5.9.5/5.9/gcc_64/ -DCMAKE_INSTALL_PREFIX=/usr/local/ ..``
* ``make -j4``
* ``wget https://github.com/iwalton3/jellyfin-web-jmp/releases/download/jwc-1.7.0/dist.zip``
* ``unzip dist.zip``
* ``sudo make install``

You should now be able to start PMP as ``jellyfinmediaplayer`` from the terminal.

If you use your distro's Qt, use `-DQTROOT=/usr` or similar.

Normally, the Ninja generator (via ``-GNinja``) is preferred, but cmake + ninja support appears to be broken on Ubuntu 16.04.

If you want, you can wipe the ``~/jmp/`` directory, as the PMP installation does not depend on it. Only Qt and libmpv are needed.

Sometimes, PMP's cmake run mysteriously fails. It's possible that https://bugreports.qt.io/browse/QTBUG-54666 is causing this. Try the following:

* locate ``Qt5CoreConfigExtras.cmake`` of your Qt build/installation
* comment ``set_property(TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_FEATURES cxx_decltype)`` with ``#``

Sometimes, PMP will pick up SDL 1.x libraries. This is not supported and will lead to build failures. You need SDL 2. You can disable use of SLD with ``-DENABLE_SDL2=off`` (it's used for some remotes).

## License

Jellyfin Media Player is licensed under GPL v2. See the ``LICENSE`` file.
Licenses of dependencies are summarized under ``resources/misc/licenses.txt``.
This file can also be printed at runtime when using the ``--licenses`` option.
