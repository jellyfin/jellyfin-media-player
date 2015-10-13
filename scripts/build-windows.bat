REM This script assumes the environment is already setup. For example:
REM
REM set PATH=C:\Python27;C:\Tools;C:\msys32\mingw32\bin;c:\msys32\usr\bin;C:\MinGW\msys\1.0\bin\;c:\windows\system32
REM set CMAKE_DIR="c:\Program Files (x86)\CMake 3.2.3\bin"
REM set QTROOT=c:\qt\qt5\build
REM set EXTRADEPS=c:\whateveryouhave (see below)
REM call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\vsvars32.bat"
REM set BUILD_DIR=build
REM (BUILD_DIR should be the name of the working dir - it must be a direct child directory of the repo dir)

REM EXTRADEPS directory contents:
REM - download libsdl.org/release/SDL2-devel-2.0.3-VC.zip
REM - copy the include dir contents to EXTRADEPS/include
REM - copy lib/x64/SDL2.{dll,lib} to EXTRADEPS/lib

scripts\fetch-binaries.py -p windows-mingw32-x86_64 -n latest -d plexmediaplayer-windows-dependencies --nochecksha  || exit /b
scripts\fetch-binaries.py -p msvc-windows-i386 -n latest -d plexmediaplayer-dependencies-msvc --nochecksha  || exit /b

REM put all dependencies into a single dir
rmdir /S /Q dependencies\all-deps
mkdir dependencies\all-deps  || exit /b
robocopy dependencies\konvergo-depends-windows-mingw32-x86_64-release dependencies\all-deps * /e
robocopy dependencies\konvergo-depends-msvc-windows-i386-release dependencies\all-deps *  /e
robocopy %EXTRADEPS% dependencies\all-deps *  /e
set DEPS=%cd%\dependencies\all-deps

LIB /def:%DEPS%\bin\mpv-1.def /out:%DEPS%\lib\mpv.lib /MACHINE:X64  || exit /b

cd %BUILD_DIR%  || exit /b

%CMAKE_DIR%\cmake -DQTROOT=%QTROOT% -DCMAKE_INSTALL_PREFIX=output -DDEPENDENCY_ROOT=%DEPS% -DBREAKPAD_LIBRARY=%DEPS%\lib\breakpad.lib -DMPV_INCLUDE_DIR=%DEPS%\include -DMPV_LIBRARY=%DEPS%\lib\mpv.lib -DSDL2_LIBRARY=%DEPS%\lib\sdl2.lib -DSDL2_INCLUDE_DIR=%DEPS%\include -DCEC_LIBRARY=%DEPS%\lib\libcec.lib -DCEC_INCLUDE_DIR=%DEPS%\include .. -G "Visual Studio 12 2013 Win64" -DENABLE_DUMP_SYMBOLS=off -DCMAKE_CONFIGURATION_TYPES=RelWithDebInfo  -DCODE_SIGN=ON || exit /b

msbuild PlexMediaPlayer.sln /p:configuration=RelWithDebInfo  || exit /b

mkdir output

%CMAKE_DIR%\cmake  -P cmake_install.cmake || exit /b

%CMAKE_DIR%\cpack   || exit /b

..\scripts\WindowsSign.cmd PlexMediaPlayer-*.exe
