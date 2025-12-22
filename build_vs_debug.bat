@echo off
REM Build script for Visual Studio solution with full Debug configuration
REM Requires Qt debug libraries to be installed

setlocal enabledelayedexpansion

echo ========================================
echo Jellyfin Desktop - Visual Studio Debug Build
echo ========================================
echo.

REM Set Qt root - adjust this path if needed
set QTROOT=C:\Qt\6.10.1\msvc2022_64
set BUILD_DEPS_DIR=%~dp0build_deps
set BUILD_DIR=%~dp0build_vs_debug

REM Verify Qt installation
if not exist "%QTROOT%" (
    echo ERROR: Qt directory not found at %QTROOT%
    echo Please verify your Qt installation path
    pause
    exit /b 1
)

REM Verify Qt debug libraries are installed
if not exist "%QTROOT%\bin\Qt6Cored.dll" (
    echo ERROR: Qt debug libraries not found!
    echo Please install Qt debug libraries using Qt Maintenance Tool:
    echo   - Run Qt Maintenance Tool
    echo   - Select "Add or remove components"
    echo   - Expand Qt -^> Qt 6.10.1 -^> MSVC 2022 64-bit
    echo   - Check "Qt Debug Information Files"
    echo   - Click Update/Install
    pause
    exit /b 1
)

REM Verify build dependencies
if not exist "%BUILD_DEPS_DIR%\ninja.exe" (
    echo ERROR: ninja.exe not found in %BUILD_DEPS_DIR%
    echo Please run setup_build_deps.bat first
    pause
    exit /b 1
)

if not exist "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll" (
    echo ERROR: MPV library not found in %BUILD_DEPS_DIR%\mpv
    echo Please run setup_build_deps.bat first
    pause
    exit /b 1
)

REM Force VS 2022 toolset (v143) for Qt compatibility
echo Forcing VS 2022 toolset (v143) for Qt compatibility...
call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=14.44

echo Visual Studio tools configured with v143 toolset (14.44)!
echo.

REM Clean and create build directory
if exist "%BUILD_DIR%" (
    echo Cleaning existing build directory...
    rmdir /s /q "%BUILD_DIR%"
)
echo Creating build directory...
mkdir "%BUILD_DIR%"

cd /d "%BUILD_DIR%"

REM Add WiX to PATH for installer creation
set PATH=%PATH%;C:\Program Files (x86)\WiX Toolset v3.14\bin;C:\Program Files (x86)\WiX Toolset v3.11\bin

REM Convert paths to forward slashes for CMake
set QTROOT_CMAKE=%QTROOT:\=/%
set BUILD_DEPS_CMAKE=%BUILD_DEPS_DIR:\=/%

REM Print compiler version information
echo.
echo ========================================
echo Compiler Version Information
echo ========================================
cl.exe 2>&1 | findstr /C:"Version"
echo.

REM Configure with CMake - Generate Visual Studio solution for Debug
echo Running CMake configuration for Visual Studio (Debug mode)...
cmake -G "Visual Studio 18 2026" -A x64 ^
    -DCMAKE_CONFIGURATION_TYPES="Debug;RelWithDebInfo;Release" ^
    -DCMAKE_INSTALL_PREFIX=output ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDebugDLL" ^
    -DQTROOT=%QTROOT_CMAKE% ^
    -DMPV_INCLUDE_DIR="%BUILD_DEPS_CMAKE%/mpv/include" ^
    -DMPV_LIBRARY="%BUILD_DEPS_CMAKE%/mpv/libmpv-2.dll.lib" ^
    -DCHECK_FOR_UPDATES=OFF ^
    -DUSE_STATIC_MPVQT=ON ^
    ..

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Building Debug Configuration
echo ========================================
echo.

REM Build the Debug configuration
cmake --build . --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Copying Dependencies to Debug Output
echo ========================================
echo.

REM Copy Qt debug DLLs to output directory
set DEBUG_OUTPUT=%BUILD_DIR%\src\Debug
if not exist "%DEBUG_OUTPUT%" mkdir "%DEBUG_OUTPUT%"

echo Copying all Qt debug DLLs...
for %%f in ("%QTROOT%\bin\Qt6*d.dll") do (
    echo   - %%~nxf
    copy /Y "%%f" "%DEBUG_OUTPUT%\" >nul
)

echo.
echo Copying Qt plugins...
xcopy /Y /E /I "%QTROOT%\plugins" "%DEBUG_OUTPUT%\plugins\" >nul

echo.
echo Copying Qt QML modules...
xcopy /Y /E /I "%QTROOT%\qml" "%DEBUG_OUTPUT%\qml\" >nul

echo.
echo Copying QtWebEngine resources (CRITICAL!)...
xcopy /Y /E /I "%QTROOT%\resources" "%DEBUG_OUTPUT%\resources\" >nul

echo.
echo Copying QtWebEngineProcess (debug version)...
copy /Y "%QTROOT%\bin\QtWebEngineProcessd.exe" "%DEBUG_OUTPUT%\" >nul

echo.
echo Copying MPV DLL...
copy /Y "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll" "%DEBUG_OUTPUT%\" >nul
copy /Y "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll.lib" "%DEBUG_OUTPUT%\" >nul

echo.
echo ========================================
echo Configuration Complete!
echo ========================================
echo.
echo Visual Studio solution created at:
echo   %BUILD_DIR%\jellyfin-desktop.sln
echo.
echo Debug executable at:
echo   %DEBUG_OUTPUT%\Jellyfin Desktop.exe
echo.
echo To debug in Visual Studio:
echo   1. Open: %BUILD_DIR%\jellyfin-desktop.sln
echo   2. Select "Debug" configuration (top toolbar)
echo   3. Right-click "Jellyfin Desktop" project ^> Set as Startup Project
echo   4. Press F5 to start debugging
echo.
pause
