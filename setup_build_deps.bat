@echo off
REM ========================================
REM Jellyfin Desktop - Build Dependencies Setup
REM ========================================
REM
REM This script helps you set up the build dependencies
REM in the build_deps folder
REM

setlocal enabledelayedexpansion

echo.
echo ========================================
echo Jellyfin Desktop Build Dependencies Setup
echo ========================================
echo.
echo This script will help you set up the required build dependencies.
echo.
echo Build dependencies will be stored in: %CD%\build_deps
echo.

set BUILD_DEPS_DIR=%CD%\build_deps

if not exist "%BUILD_DEPS_DIR%" (
    echo Creating build_deps directory...
    mkdir "%BUILD_DEPS_DIR%"
)

echo.
echo ========================================
echo Dependency 1: Ninja Build System
echo ========================================
echo.
echo Ninja is a fast build system used for compiling Jellyfin Desktop.
echo.

if exist "%BUILD_DEPS_DIR%\ninja.exe" (
    echo [OK] Ninja already installed: %BUILD_DEPS_DIR%\ninja.exe
) else (
    echo Ninja not found.
    echo.
    echo Please download ninja.exe:
    echo   1. Go to: https://github.com/ninja-build/ninja/releases/latest
    echo   2. Download: ninja-win.zip
    echo   3. Extract ninja.exe to: %BUILD_DEPS_DIR%\
    echo.
    echo After downloading, press any key to continue...
    pause >nul

    if exist "%BUILD_DEPS_DIR%\ninja.exe" (
        echo [OK] Ninja installed successfully!
    ) else (
        echo [WARNING] Ninja still not found. Please install manually.
    )
)

echo.
echo ========================================
echo Dependency 2: libmpv (Media Player)
echo ========================================
echo.
echo libmpv is the media player library used by Jellyfin Desktop.
echo.

if exist "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll" (
    echo [OK] libmpv already installed: %BUILD_DEPS_DIR%\mpv\libmpv-2.dll
) else (
    echo libmpv not found.
    echo.
    echo Opening MPV download page in your browser...
    timeout /t 1 >nul
    start https://sourceforge.net/projects/mpv-player-windows/files/libmpv/
    echo.
    echo ========================================
    echo MPV Installation Instructions
    echo ========================================
    echo.
    echo 1. Download mpv-dev-x86_64-YYYYMMDD.7z from the page that just opened
    echo 2. Extract the .7z file (you may need 7-Zip: https://www.7-zip.org/)
    echo 3. From the extracted folder, copy:
    echo.
    echo    libmpv-2.dll  ==^>  %BUILD_DEPS_DIR%\mpv\libmpv-2.dll
    echo.
    echo 4. Copy the include folder:
    echo.
    echo    include\mpv\  ==^>  %BUILD_DEPS_DIR%\mpv\include\mpv\
    echo.
    echo Final structure should be:
    echo    %BUILD_DEPS_DIR%\mpv\libmpv-2.dll
    echo    %BUILD_DEPS_DIR%\mpv\include\mpv\client.h
    echo    %BUILD_DEPS_DIR%\mpv\include\mpv\... (other headers)
    echo.
    echo After downloading and extracting, press any key to continue...
    pause >nul

    if exist "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll" (
        echo [OK] libmpv installed successfully!
    ) else (
        echo [WARNING] libmpv still not found. Please install manually.
    )
)

if exist "%BUILD_DEPS_DIR%\mpv\include\mpv\client.h" (
    echo [OK] libmpv headers found
) else (
    echo [WARNING] libmpv headers not found at: %BUILD_DEPS_DIR%\mpv\include\mpv\
)

echo.
echo ========================================
echo Dependency 3: Visual C++ Redistributable
echo ========================================
echo.
echo The VC++ redistributable is bundled with the installer.
echo.

if exist "%BUILD_DEPS_DIR%\vc_redist.x64.exe" (
    echo [OK] vc_redist.x64.exe already downloaded: %BUILD_DEPS_DIR%\vc_redist.x64.exe
) else (
    echo vc_redist.x64.exe not found.
    echo.
    echo Opening download page in your browser...
    timeout /t 1 >nul
    start https://aka.ms/vs/17/release/vc_redist.x64.exe
    echo.
    echo ========================================
    echo VC++ Redistributable Installation Instructions
    echo ========================================
    echo.
    echo 1. Your browser should have started downloading vc_redist.x64.exe
    echo 2. Move the downloaded file to: %BUILD_DEPS_DIR%\vc_redist.x64.exe
    echo.
    echo After downloading, press any key to continue...
    pause >nul

    if exist "%BUILD_DEPS_DIR%\vc_redist.x64.exe" (
        echo [OK] vc_redist.x64.exe installed successfully!
    ) else (
        echo [WARNING] vc_redist.x64.exe still not found. Please install manually.
    )
)

echo.
echo ========================================
echo Dependency 4: Qt Framework
echo ========================================
echo.
echo Qt 6 is the GUI framework used by Jellyfin Desktop.
echo.
echo Please ensure Qt 6.x with MSVC 2022 is installed.
echo Common installation paths:
echo   - C:\Qt\6.10.1\msvc2022_64
echo   - C:\Qt\6.8.0\msvc2022_64
echo.
echo If not installed, download from:
echo   https://www.qt.io/download-open-source
echo.
echo Install Qt 6.x with these components:
echo   - Qt 6.x for desktop development (MSVC 2022 64-bit)
echo   - Qt WebEngine
echo   - Qt WebChannel
echo   - Qt Quick Controls
echo.

REM Try to detect Qt
set QT_FOUND=0
for %%Q in (
    "C:\Qt\6.10.1\msvc2022_64"
    "C:\Qt\6.10.0\msvc2022_64"
    "C:\Qt\6.9.0\msvc2022_64"
    "C:\Qt\6.8.0\msvc2022_64"
    "C:\Qt\6.8.0\msvc2019_64"
) do (
    if exist %%Q (
        echo [OK] Found Qt at: %%~Q
        set QT_FOUND=1
        goto :qt_check_done
    )
)

:qt_check_done
if !QT_FOUND!==0 (
    echo [WARNING] Qt not found in standard locations
    echo.
    echo You may need to specify QTROOT manually or install Qt.
)

echo.
echo ========================================
echo Dependency 5: Visual Studio 2022
echo ========================================
echo.
echo Visual Studio 2022 with C++ tools is required for compilation.
echo.

where cl >nul 2>&1
if %ERRORLEVEL%==0 (
    echo [OK] Visual Studio C++ compiler found in PATH
) else (
    REM Try to find VS
    set VS_FOUND=0
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        echo [OK] Found Visual Studio 2022 Community
        set VS_FOUND=1
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        echo [OK] Found Visual Studio 2022 Professional
        set VS_FOUND=1
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
        echo [OK] Found Visual Studio 2022 Build Tools
        set VS_FOUND=1
    ) else (
        echo [WARNING] Visual Studio 2022 not detected
        echo.
        echo Please install:
        echo   - Visual Studio 2022 Community (free) OR
        echo   - Visual Studio 2022 Build Tools
        echo.
        echo Download from:
        echo   https://visualstudio.microsoft.com/downloads/
        echo.
        echo Required components:
        echo   - Desktop development with C++
        echo   - MSVC v143 - VS 2022 C++ x64/x86 build tools
        echo   - Windows 10/11 SDK
        set VS_FOUND=0
    )
)

echo.
echo ========================================
echo Setup Summary
echo ========================================
echo.
echo Build dependencies directory: %BUILD_DEPS_DIR%
echo.

set ALL_READY=1

if exist "%BUILD_DEPS_DIR%\ninja.exe" (
    echo [OK] Ninja
) else (
    echo [MISSING] Ninja
    set ALL_READY=0
)

if exist "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll" (
    echo [OK] libmpv DLL
) else (
    echo [MISSING] libmpv DLL
    set ALL_READY=0
)

if exist "%BUILD_DEPS_DIR%\mpv\include\mpv\client.h" (
    echo [OK] libmpv headers
) else (
    echo [MISSING] libmpv headers
    set ALL_READY=0
)

if exist "%BUILD_DEPS_DIR%\vc_redist.x64.exe" (
    echo [OK] VC++ Redistributable
) else (
    echo [MISSING] VC++ Redistributable
    set ALL_READY=0
)

echo.
if !ALL_READY!==1 (
    echo All build dependencies are ready!
    echo.
    echo You can now run: build_windows.bat
) else (
    echo Some dependencies are missing. Please install them and run this script again.
)

echo.
pause
