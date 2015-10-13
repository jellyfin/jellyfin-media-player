# How-To build Qt 5.5.0 on Windows
This procedure was ran on a freshly installed Win7 64 bits.

This procedure assumes that the build root is `C:\qt\` but it can be changed in the beginning of each `.bat` file

1. Install Visual Studio 2013
- Install [Visual Studio 2013 Update 4](http://www.microsoft.com/fr-fr/download/details.aspx?id=44921) 
- Install [Windows SDK 8](https://msdn.microsoft.com/en-us/windows/desktop/hh852363.aspx)
- Install [ActivePerl](http://www.activestate.com/activeperl/downloads)
- Install [Python 2.7.10](https://www.python.org/downloads/release/python-2710)
- Install the following gnutools:
  - [Bison](http://gnuwin32.sourceforge.net/downlinks/bison.php)
  - [GPerf](http://gnuwin32.sourceforge.net/downlinks/gperf.php)
  - [Flex](http://gnuwin32.sourceforge.net/downlinks/flex.php)
- Grab [ICU sources](http://download.icu-project.org/files/icu4c/55.1/icu4c-55_1-src.zip) and unpack them in `C:\qt\icu`
- Grab [Openssl 1.0.2d](https://www.openssl.org/source/) and unpack them in `C:\qt\openssl`
- Grab [Qt Sources](http://download.qt.io/official_releases/qt/5.5/5.5.0/single/qt-everywhere-opensource-src-5.5.0.zip) and unpack them in  `C:\qt\qt5`
- drop the three `.bat` files in the steps below in `C:\qt\` 
- Open `C:\qt\icu\source\allionone\allinone.sln` with VS2013, you will be prompted for project conversion. Once the project is converted, just close it.
- run `c:\qt\buildicu.bat`
- run `c:\qt\buildopenssl.bat`
- run `c:\qt\buildqt.bat`

The qt build will land in `C:\qt\qt5\build`


# buildicu.bat
```
set BUILD_ROOT=C:\qtREM Set up \Microsoft Visual Studio 2013, where <arch> is \c amd64, \c x86, etc.CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64REM set compiler to multicoreset CL=/MPREM Build icucd %BUILD_ROOT%\icumsbuild source\allinone\allinone.sln /m /target:Build /property:Configuration=Release;Platform=x64msbuild source\allinone\allinone.sln /m /target:Build /property:Configuration=Debug;Platform=x64
cd ..
```

# buildopenssl.bat
```
set BUILD_ROOT=C:\qtCALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64cd opensslset CL=/MPperl Configure VC-WIN64A --prefix=%BUILD_ROOT%\openssl\buildcall ms\do_win64anmake -f ms\ntdll.maknmake -f ms\ntdll.mak installcd ..
```

# buildqt.bat
```
set BUILD_ROOT=C:\qtREM Set up \Microsoft Visual Studio 2013, where <arch> is \c amd64, \c x86, etc.CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64REM set compiler to multicoreset CL=/MPREM Add ICU dirs to the proper path (include, libs , bin)set INCLUDE=%INCLUDE%;%BUILD_ROOT%\icu\includeset LIB=%LIB%;%BUILD_ROOT%\icu\lib64set PATH=%PATH%;%BUILD_ROOT%\icu\bin64REM Add OpenSSL dirs to the proper path (include, libs , bin)set INCLUDE=%INCLUDE%;%BUILD_ROOT%\openssl\build\includeset LIB=%LIB%;%BUILD_ROOT%\openssl\build\libset PATH=%PATH%;%BUILD_ROOT%\openssl\build\binREM Add Pyhton dirs to the proper path (include, libs , bin)set PATH=%PATH%;c:\Python27\REM Add GunWindirs to the proper path (include, libs , bin)set INCLUDE=%INCLUDE%;%BUILD_ROOT%\GnuWin32\includeset LIB=%LIB%;%BUILD_ROOT%\GnuWin32\libset PATH=%PATH%;=%BUILD_ROOT%\GnuWin32\binSET QT_ROOT=%BUILD_ROOT%\qt5SET PATH=%QT_ROOT%\qtbase\bin;%PATH%SET QMAKESPEC=win32-msvc2013cd %QT_ROOT%CALL configure -prefix %QT_ROOT%\build -icu -opengl dynamic -release -nomake examples -opensource -confirm-license  -no-gif  -qt-libpng -qt-libjpeg -openssl -qt-pcre -no-cups -no-dbus -skip qtwebkit -skip qtconnectivity -skip qtdoc -skip qtgraphicaleffects -skip qtmultimedia -skip qtsensors -skip qtserialport -skip qtwebkit-examples -skip qtquick1 -skip qt3dnmakenmake installcd ..
```