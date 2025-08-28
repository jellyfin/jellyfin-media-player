@echo off
REM Windows Registry Script to Register Jellyfin Media Player URL Protocols
REM Run as Administrator

echo Registering Jellyfin Media Player URL protocols...

REM Get the installation directory
set INSTALL_DIR=%~dp0
if "%INSTALL_DIR:~-1%"=="\" set INSTALL_DIR=%INSTALL_DIR:~0,-1%

REM Register jellyfin:// protocol
reg add "HKEY_CLASSES_ROOT\jellyfin" /ve /t REG_SZ /d "URL:Jellyfin Media Player Protocol" /f
reg add "HKEY_CLASSES_ROOT\jellyfin" /v "URL Protocol" /t REG_SZ /d "" /f
reg add "HKEY_CLASSES_ROOT\jellyfin\shell" /f
reg add "HKEY_CLASSES_ROOT\jellyfin\shell\open" /f
reg add "HKEY_CLASSES_ROOT\jellyfin\shell\open\command" /ve /t REG_SZ /d "\"%INSTALL_DIR%\JellyfinMediaPlayer.exe\" \"%%1\"" /f

REM Register jmp:// protocol  
reg add "HKEY_CLASSES_ROOT\jmp" /ve /t REG_SZ /d "URL:Jellyfin Media Player Protocol" /f
reg add "HKEY_CLASSES_ROOT\jmp" /v "URL Protocol" /t REG_SZ /d "" /f
reg add "HKEY_CLASSES_ROOT\jmp\shell" /f
reg add "HKEY_CLASSES_ROOT\jmp\shell\open" /f
reg add "HKEY_CLASSES_ROOT\jmp\shell\open\command" /ve /t REG_SZ /d "\"%INSTALL_DIR%\JellyfinMediaPlayer.exe\" \"%%1\"" /f

if %errorlevel% equ 0 (
    echo.
    echo URL protocols registered successfully!
    echo You can now use jellyfin:// and jmp:// URLs to launch Jellyfin Media Player.
    echo.
    echo Test with: jellyfin://connect?server=https://demo.jellyfin.org
) else (
    echo.
    echo ERROR: Failed to register URL protocols.
    echo Please run this script as Administrator.
)

pause