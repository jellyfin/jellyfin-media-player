@echo off
REM Jellyfin Desktop - Run built executable
REM Run build.bat first

setlocal
call "%~dp0common.bat"

REM === Check build exists ===
if not exist "%BUILD_DIR%\src\%EXE_NAME%" (
    echo ERROR: Build not found. Run build.bat first
    exit /b 1
)

REM === Setup PATH for DLLs ===
set "PATH=%DEPS_DIR%\mpv;%PATH%"
set "PATH=%DEPS_DIR%\qt\%QT_VERSION%\msvc2022_64\bin;%PATH%"

REM === Run ===
"%BUILD_DIR%\src\%EXE_NAME%" %*
set EXIT_CODE=%ERRORLEVEL%

endlocal & exit /b %EXIT_CODE%
