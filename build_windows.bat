@echo off
REM ========================================
REM Jellyfin Desktop - Windows Build Script
REM ========================================
REM
REM This script uses the new build_deps/ structure
REM All external dependencies are in build_deps/
REM All generated files are in build/
REM

setlocal enabledelayedexpansion

echo.
echo ========================================
echo Jellyfin Desktop - Windows Build Script
echo ========================================
echo.

REM Change to project root directory
cd /d "%~dp0"

REM Define directories
set BUILD_DEPS_DIR=%CD%\build_deps
set BUILD_DIR=%CD%\build
set OUTPUT_DIR=%BUILD_DIR%\output

REM Force VS 2022 toolset (v143) for Qt compatibility
echo Forcing VS 2022 toolset (v143) for Qt compatibility...
call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=14.44

echo Visual Studio tools configured with v143 toolset (14.44)!
echo.

REM Step 1: Check build dependencies
echo ========================================
echo Step 1/6: Checking Build Dependencies
echo ========================================
echo.

if not exist "%BUILD_DEPS_DIR%\ninja.exe" (
    echo.
    echo ERROR: ninja.exe not found in build_deps\
    echo.
    echo Please run: setup_build_deps.bat
    echo.
    pause
    exit /b 1
)

if not exist "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll" (
    echo.
    echo ERROR: MPV not found in build_deps\mpv\
    echo.
    echo Please run: setup_build_deps.bat
    echo.
    pause
    exit /b 1
)

if not exist "%BUILD_DEPS_DIR%\mpv\include\mpv\client.h" (
    echo.
    echo ERROR: MPV headers not found in build_deps\mpv\include\mpv\
    echo.
    echo Please run: setup_build_deps.bat
    echo.
    pause
    exit /b 1
)

echo All build dependencies found:
echo   - Ninja: %BUILD_DEPS_DIR%\ninja.exe
echo   - MPV DLL: %BUILD_DEPS_DIR%\mpv\libmpv-2.dll
echo   - MPV Headers: %BUILD_DEPS_DIR%\mpv\include\
echo.

REM Step 2: Generate/verify MPV import library
echo ========================================
echo Step 2/6: Verifying MPV Import Library
echo ========================================
echo.

if not exist "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll.lib" (
    echo Generating MPV import library from DLL...

    if not exist "scripts\generate_mpv_def.py" (
        echo ERROR: scripts\generate_mpv_def.py not found
        pause
        exit /b 1
    )

    python scripts\generate_mpv_def.py "%BUILD_DEPS_DIR%\mpv\libmpv-2.dll" "%BUILD_DEPS_DIR%\mpv\mpv.def"
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to generate mpv.def
        pause
        exit /b 1
    )

    lib /def:"%BUILD_DEPS_DIR%\mpv\mpv.def" /out:"%BUILD_DEPS_DIR%\mpv\libmpv-2.dll.lib" /MACHINE:X64
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to create import library
        pause
        exit /b 1
    )

    echo Import library created successfully!
) else (
    echo Import library already exists: %BUILD_DEPS_DIR%\mpv\libmpv-2.dll.lib
)

echo.

REM Step 3: Detect Qt
echo ========================================
echo Step 3/6: Detecting Qt Installation
echo ========================================
echo.

REM Try to detect Qt installation
set QTROOT=
for %%Q in (
    "C:\Qt\6.10.1\msvc2022_64"
    "C:\Qt\6.10.0\msvc2022_64"
    "C:\Qt\6.9.0\msvc2022_64"
    "C:\Qt\6.8.0\msvc2022_64"
    "C:\Qt\6.8.0\msvc2019_64"
    "C:\Qt\6.7.0\msvc2019_64"
) do (
    if exist %%Q (
        set QTROOT=%%~Q
        goto :qt_found
    )
)

:qt_found
if "%QTROOT%"=="" (
    echo ERROR: Could not find Qt installation
    echo.
    echo Please install Qt 6.x with MSVC 2022 or set QTROOT environment variable
    echo Example: C:\Qt\6.10.1\msvc2022_64
    echo.
    echo Download from: https://www.qt.io/download-open-source
    echo.
    pause
    exit /b 1
)

echo Found Qt at: %QTROOT%
echo.

REM Step 4: Create build directory
echo ========================================
echo Step 4/6: Configuring Build with CMake
echo ========================================
echo.

if not exist "%BUILD_DIR%" (
    echo Creating build directory...
    mkdir "%BUILD_DIR%"
)

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

REM Configure with CMake - IMPORTANT: Use .lib file, not .dll
echo Running CMake configuration...
cmake -GNinja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=output ^
    -DCMAKE_MAKE_PROGRAM="%BUILD_DEPS_CMAKE%/ninja.exe" ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL" ^
    -DCMAKE_CXX_FLAGS="/MD" ^
    -DCMAKE_C_FLAGS="/MD" ^
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
    echo Check the error messages above for details
    cd /d "%~dp0"
    pause
    exit /b 1
)

echo.

REM Step 5: Build
echo ========================================
echo Step 5/6: Building with Ninja
echo ========================================
echo.

"%BUILD_DEPS_DIR%\ninja.exe"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed
    echo.
    echo Check the error messages above for details
    cd /d "%~dp0"
    pause
    exit /b 1
)

echo.

REM Step 6: Install
echo ========================================
echo Step 6/6: Installing to Output Directory
echo ========================================
echo.

"%BUILD_DEPS_DIR%\ninja.exe" install
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Install failed
    echo.
    cd /d "%~dp0"
    pause
    exit /b 1
)

REM Copy VC++ redistributable (for installer packaging)
echo.
echo Copying VC++ redistributable for installer...
if exist "%BUILD_DEPS_DIR%\vc_redist.x64.exe" (
    echo   - vc_redist.x64.exe
    copy /Y "%BUILD_DEPS_DIR%\vc_redist.x64.exe" "%OUTPUT_DIR%\" >nul
) else (
    echo   [WARNING] vc_redist.x64.exe not found - installer build will fail
)

echo.
echo Qt dependencies copied by windeployqt (CMake install step)

cd /d "%~dp0"

echo.
echo ========================================
echo BUILD COMPLETED SUCCESSFULLY!
echo ========================================
echo.
echo Build output: %OUTPUT_DIR%
echo Executable: %OUTPUT_DIR%\Jellyfin Desktop.exe
echo.
echo.
echo Building installer in 5 seconds...
echo Press any key to skip installer build.
echo.

REM Wait 5 seconds or until key is pressed
timeout /t 5 >nul
set TIMEOUT_RESULT=%ERRORLEVEL%

if %TIMEOUT_RESULT% EQU 0 (
    REM Timeout expired - build installer
    echo.
    echo ========================================
    echo Building Installer
    echo ========================================
    echo.

    cd /d "%BUILD_DIR%"
    "%BUILD_DEPS_DIR%\ninja.exe" windows_package

    if %ERRORLEVEL% NEQ 0 (
        echo.
        echo WARNING: Installer build failed
        echo.
        cd /d "%~dp0"
    ) else (
        echo.
        echo ========================================
        echo INSTALLER BUILD COMPLETED!
        echo ========================================
        echo.
        REM Find the installer file
        for %%f in ("%BUILD_DIR%\*.exe") do (
            echo Installer created: %%f
        )
        if not exist "%BUILD_DIR%\*.exe" (
            echo NOTE: Installer executable not found in %BUILD_DIR%
            echo Check the Ninja output above for details.
        )
        echo.
        cd /d "%~dp0"
    )
) else (
    REM Key was pressed - skip installer
    echo.
    echo Skipping installer build.
    echo To build installer later, run:
    echo   cd build
    echo   ..\build_deps\ninja.exe windows_package
    echo.
    cd /d "%~dp0"
)

pause
