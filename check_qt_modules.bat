@echo off
echo Checking Qt installation for required modules...
echo.

set QTROOT=C:/Qt/6.10.1/msvc2022_64

echo Qt Root: %QTROOT%
echo.

echo Required Qt Modules:
echo ====================
echo.

set MISSING=0

echo Checking Qt6Core...
if exist "%QTROOT%\lib\cmake\Qt6Core" (
    echo [OK] Qt6Core
) else (
    echo [MISSING] Qt6Core
    set MISSING=1
)

echo Checking Qt6WebEngineCore...
if exist "%QTROOT%\lib\cmake\Qt6WebEngineCore" (
    echo [OK] Qt6WebEngineCore
) else (
    echo [MISSING] Qt6WebEngineCore - REQUIRED FOR JELLYFIN DESKTOP
    set MISSING=1
)

echo Checking Qt6WebEngineQuick...
if exist "%QTROOT%\lib\cmake\Qt6WebEngineQuick" (
    echo [OK] Qt6WebEngineQuick
) else (
    echo [MISSING] Qt6WebEngineQuick - REQUIRED FOR JELLYFIN DESKTOP
    set MISSING=1
)

echo Checking Qt6WebChannel...
if exist "%QTROOT%\lib\cmake\Qt6WebChannel" (
    echo [OK] Qt6WebChannel
) else (
    echo [MISSING] Qt6WebChannel - REQUIRED FOR JELLYFIN DESKTOP
    set MISSING=1
)

echo Checking Qt6Quick...
if exist "%QTROOT%\lib\cmake\Qt6Quick" (
    echo [OK] Qt6Quick
) else (
    echo [MISSING] Qt6Quick
    set MISSING=1
)

echo Checking Qt6Qml...
if exist "%QTROOT%\lib\cmake\Qt6Qml" (
    echo [OK] Qt6Qml
) else (
    echo [MISSING] Qt6Qml
    set MISSING=1
)

echo Checking Qt6Network...
if exist "%QTROOT%\lib\cmake\Qt6Network" (
    echo [OK] Qt6Network
) else (
    echo [MISSING] Qt6Network
    set MISSING=1
)

echo Checking Qt6Widgets...
if exist "%QTROOT%\lib\cmake\Qt6Widgets" (
    echo [OK] Qt6Widgets
) else (
    echo [MISSING] Qt6Widgets
    set MISSING=1
)

echo.
echo.

if %MISSING%==1 (
    echo ========================================
    echo WARNING: Some required Qt modules are missing!
    echo ========================================
    echo.
    echo Please install the missing modules using Qt Maintenance Tool:
    echo   1. Run Qt Maintenance Tool
    echo   2. Select "Add or remove components"
    echo   3. Navigate to Qt ^> Qt 6.10.1 ^> MSVC 2022 64-bit
    echo   4. Make sure these are checked:
    echo      - Qt WebEngine
    echo      - Qt WebChannel
    echo      - Qt Quick
    echo   5. Click Update/Install
    echo.
    echo After installing, rebuild Jellyfin Desktop with build_windows.bat
    echo.
) else (
    echo ========================================
    echo All required Qt modules are installed!
    echo ========================================
)

echo.
pause
