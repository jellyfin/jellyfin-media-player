# Jellyfin Media Player

Desktop client using jellyfin-web with embedded MPV player. Supports Windows, Mac OS,
and Linux. Media plays within the same window using the jellyfin-web interface unlike
Jellyfin Desktop. Supports audio passthrough. Based on [Plex Media Player](https://github.com/plexinc/plex-media-player).

![Screenshot of Jellyfin Media Player](https://raw.githubusercontent.com/iwalton3/mpv-shim-misc-docs/master/images/jmp-player-win.png)

Downloads:
 - [Windows, Mac, and Linux Releases](https://github.com/jellyfin/jellyfin-media-player/releases)
 - [Flathub (Linux)](https://flathub.org/apps/details/com.github.iwalton3.jellyfin-media-player)

Related Documents:
 - Corresponding web client: [Repo](https://github.com/iwalton3/jellyfin-web-jmp/) [Release](https://github.com/iwalton3/jellyfin-web-jmp/releases/)
 - API Docs in [client-api.md](https://github.com/iwalton3/jellyfin-media-player/blob/master/client-api.md)
 - Tip: For help building, look at the GitHub Actions file!

## Building at a glance (Linux)

```bash
sudo apt install autoconf automake libtool libharfbuzz-dev libfreetype6-dev libfontconfig1-dev libx11-dev libxrandr-dev libvdpau-dev libva-dev mesa-common-dev libegl1-mesa-dev yasm libasound2-dev libpulse-dev libuchardet-dev zlib1g-dev libfribidi-dev git libgnutls28-dev libgl1-mesa-dev libsdl2-dev cmake wget python g++ qtwebengine5-dev qtquickcontrols2-5-dev libqt5x11extras5-dev libcec-dev qml-module-qtquick-controls qml-module-qtwebengine qml-module-qtwebchannel qtbase5-private-dev
mkdir jmp; cd jmp
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
./download_webclient.sh
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/ ..
make -j4
sudo make install
rm -rf ~/jmp/
```

## Building for Windows

Please install:
 - [cmake](https://cmake.org/download/) - cmake-3.20.0-windows-x86_64.msi
   - Add cmake to the path.
 - [ninja](https://github.com/ninja-build/ninja/releases)
   - Place this in the build directory.
 - [QT](https://www.qt.io/download-thank-you?hsLang=en)
   - This package is huge. You also need to make a QT account...
   - Check "MSVC 2019 64-bit" and "Qt WebEngine" under QT 5.15.2.
 - [VS2019 Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019)
   - Again this will use a lot of disk space. The installer is small though.
 - [libmpv1](https://sourceforge.net/projects/mpv-player-windows/files/libmpv/)
   - Place the contents in the build directory, in a subfolder called `mpv`.
   - Move the contents of the `include` folder to an `mpv` folder inside the `include` folder.
   - Move the `mpv-1.dll` to `mpv.dll`.
 - [WIX](https://wixtoolset.org/releases/v3.11.2/stable)

You need to run these commands in git bash.

```bash
git clone https://github.com/iwalton3/jellyfin-media-player
cd jellyfin-media-player
./download_webclient.sh
cd build
```

Open the "x86_x64 Cross Tools Command Prompt for VS 2019". `cd` to the `build` directory. Run:

```
set PATH=%PATH%;C:\Program Files (x86)\WiX Toolset v3.11\bin
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=output -DCMAKE_MAKE_PROGRAM=ninja.exe -DQTROOT=C:/Qt/5.15.2/msvc2019_64 -DMPV_INCLUDE_DIR=mpv/include -DMPV_LIBRARY=mpv/mpv.dll -DCMAKE_INSTALL_PREFIX=output ..
lib /def:mpv\mpv.def /out:mpv\mpv.dll.lib /MACHINE:X64
ninja
ninja windows_package
```

## Log File Location

 - Windows: `%LOCALAPPDATA%\JellyfinMediaPlayer\logs`
 - Linux: `~/.local/share/jellyfinmediaplayer/logs/`
 - Linux (Flatpak): `~/.var/app/com.github.iwalton3.jellyfin-media-player/data/jellyfinmediaplayer/logs/`
 - macOS: `~/Library/Logs/Jellyfin Media Player/`

## Config File Location

The main configuration file is called `jellyfinmediaplayer.conf`. You can also add a `mpv.conf` to configure MPV directly.

 - Windows: `%LOCALAPPDATA%\JellyfinMediaPlayer\`
 - Linux: `~/.local/share/jellyfinmediaplayer/`
 - Linux (Flatpak): `~/.var/app/com.github.iwalton3.jellyfin-media-player/data/jellyfinmediaplayer/`
 - macOS: `~/Library/Application Support/Jellyfin Media Player/`

## License

Jellyfin Media Player is licensed under GPL v2. See the ``LICENSE`` file.
Licenses of dependencies are summarized under ``resources/misc/licenses.txt``.
This file can also be printed at runtime when using the ``--licenses`` option.
