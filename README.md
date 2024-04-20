# Jellyfin Media Player

Desktop client using jellyfin-web with embedded MPV player. Supports Windows, Mac OS,
and Linux. Media plays within the same window using the jellyfin-web interface unlike
Jellyfin Desktop. Supports audio passthrough. Based on [Plex Media Player](https://github.com/plexinc/plex-media-player).

![Screenshot of Jellyfin Media Player](https://raw.githubusercontent.com/iwalton3/mpv-shim-misc-docs/master/images/jmp-player-win.png)

Downloads:
 - [Windows, Mac, and Linux Releases](https://github.com/jellyfin/jellyfin-media-player/releases)
 - [Flathub (Linux)](https://flathub.org/apps/details/com.github.iwalton3.jellyfin-media-player)

Related Documents:
 - Web client: https://repo.jellyfin.org/releases/server/portable/versions/stable/web/
     - Note: If you do not provide the web client, the application will use a fallback UI where the user must select a server which has a web client.
 - Web client integration documentation: [for-web-developers.md](https://github.com/jellyfin/jellyfin-media-player/blob/master/for-web-developers.md)
 - API Docs in [client-api.md](https://github.com/jellyfin/jellyfin-media-player/blob/master/client-api.md)
 - Tip: For help building, look at the GitHub Actions file!

## Building at a glance (Linux)

To download the latest stable release, get the lattest version tag from the [latest releases page](https://github.com/jellyfin/jellyfin-media-player/releases/latest) and append the following to your pull command during the build phase for JMP "--branch $VERSIONTAG --single-branch"

Example:
```bash
git clone https://github.com/jellyfin/jellyfin-media-player.git --branch v1.9.1 --single-branch
```


### Ubuntu based systems

Install dependancies:
```bash
sudo apt install build-essential autoconf automake libtool libharfbuzz-dev libfreetype6-dev libfontconfig1-dev libx11-dev libxrandr-dev libvdpau-dev libva-dev mesa-common-dev libegl1-mesa-dev yasm libasound2-dev libpulse-dev libuchardet-dev zlib1g-dev libfribidi-dev git libgnutls28-dev libgl1-mesa-dev libsdl2-dev cmake wget python g++ qtwebengine5-dev qtquickcontrols2-5-dev libqt5x11extras5-dev libcec-dev qml-module-qtquick-controls qml-module-qtwebengine qml-module-qtwebchannel qtbase5-private-dev curl unzip
```

Build commands for Ubuntu:
```bash
mkdir ~/jmp; cd ~/jmp
git clone https://github.com/mpv-player/mpv-build.git
cd mpv-build
echo -Dlibmpv=true > mpv_options
echo -Dpipewire=disabled >> mpv_options # hopefully temporary
./rebuild -j`nproc`
sudo ./install
sudo ln -s /usr/local/lib/x86_64-linux-gnu/libmpv.so /usr/local/lib/x86_64-linux-gnu/libmpv.so.1
sudo ln -sf /usr/local/lib/x86_64-linux-gnu/libmpv.so /usr/local/lib/libmpv.so.2
sudo ldconfig
cd ~/jmp/
git clone https://github.com/jellyfin/jellyfin-media-player.git
cd jellyfin-media-player
./download_webclient.sh
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/ ..
make -j`nproc`
sudo make install
rm -rf ~/jmp/
```

### Fedora based systems

Install dependancies:
```bash
sudo dnf install autoconf automake libtool freetype-devel libXrandr-devel libvdpau-devel libva-devel  mesa-libGL-devel libdrm-devel libX11-devel  mesa-libEGL-devel yasm  alsa-lib pulseaudio-libs-devel zlib-devel fribidi-devel git gnutls-devel mesa-libGLU-devel  SDL2-devel cmake wget python g++  qt-devel libcec-devel qt5-qtbase-devel curl unzip qt5-qtwebchannel-devel qt5-qtwebengine-devel qt5-qtx11extras-devel mpv.x86_64 qwt-qt5-devel.x86_64 qt5-qtbase.x86_64 meson.noarch ninja-build.x86_64 qt5-qtbase-private-devel mpv-libs.x86_64
```

Build commands for Fedora:

Note, the only real differences here is that libraries are in diffrent directories on Fedora systems.
```bash
mkdir ~/jmp; cd ~/jmp
git clone https://github.com/mpv-player/mpv-build.git
cd mpv-build/
echo -Dlibmpv=true > mpv_options
echo -Dpipewire=disabled >> mpv_options # hopefully temporary
./rebuild -j`nproc`
sudo ./install
sudo mkdir /usr/local/lib/x86_64-linux-gnu
sudo ln -s /usr/local/lib64/libmpv.so /usr/local/lib/x86_64-linux-gnu/libmpv.so.1
sudo ln -s /usr/local/lib64/libmpv.so /usr/local/lib/x86_64-linux-gnu/libmpv.so
sudo ldconfig
cd ~/jmp/
git clone https://github.com/jellyfin/jellyfin-media-player.git
cd jellyfin-media-player/
./download_webclient.sh 
cd build/
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/ ..
make -j`nproc`
sudo make install
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
git clone https://github.com/jellyfin/jellyfin-media-player
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

## Building for MacOS

Install [QT 5.15.2](https://www.qt.io/download-thank-you?hsLang=en), remember to check `Qt WebEngine`.

Then run the following commands (replace <QT_DIR> with your QT installation location):

```bash
brew install mpv ninja
./download_webclient.sh
cd build
cmake -GNinja -DQTROOT=<QT_DIR> -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=output ..
ninja install
```

To create redistributable bundle, some library paths need to be fixed. At the project root directory, run:

```bash
python3 ./scripts/fix-install-names.py ./build/output/Jellyfin\ Media\ Player.app
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

## Web Debugger

To get browser devtools, use remote debugging.

 - Run the application with the command argument `--remote-debugging-port=9222`.
 - Open Chromium or Google Chrome.
 - Navigate to `chrome://inspect/#devices`.
 - You can now access the developer tools.

If you have problems:

 - Make sure "Discover Network Targets" is checked.
 - Make sure `localhost:9222` is in the list under "Configure...".
 - Make sure `--remote-debugging-port=9222` is specified correctly.

## License

Jellyfin Media Player is licensed under GPL v2. See the ``LICENSE`` file.
Licenses of dependencies are summarized under ``resources/misc/licenses.txt``.
This file can also be printed at runtime when using the ``--licenses`` option.

## Unofficial Plugin Support

You can enable experimental support for [Jellyscrub](https://github.com/nicknsy/jellyscrub) and [Skip Intro](https://github.com/ConfusedPolarBear/intro-skipper) in client settings. These are included for convenience only and is not an endorsement or long-term commitment to ensure functionality. See `src/native` for details on what the plugins modify code-wise.

## Known Issues

If you build MPV from source, you currently need to disable pipewire or else the client will segfault.
