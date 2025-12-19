# Building Jellyfin Desktop on Windows

## Quick Start

```cmd
setup_build_deps.bat    # First time: download dependencies
build_windows.bat       # Build
```

Output: `build\output\Jellyfin Desktop.exe`

## Prerequisites

- **Visual Studio 2022/2026** with C++ tools
  - Install "MSVC v143 - VS 2022 C++ x64/x86 build tools" (required for Qt 6.10.1)
- **Qt 6.10.1** MSVC 2022 64-bit at `C:\Qt\6.10.1\msvc2022_64`
- **Build dependencies**: Ninja, libmpv, VC++ redistributable

## Directory Structure

- `build_deps/` - External dependencies (persisted)
- `build/` - Generated files (safe to delete)
- `build_vs_debug/` - VS debug build

## Scripts

- `build_windows.bat` - Release build + installer
- `build_vs.bat` - Debug build for Visual Studio
- `setup_build_deps.bat` - Download dependencies
- `rebuild.bat` - Quick rebuild

## Clean Build

```cmd
rmdir /s /q build
build_windows.bat
```

## Debugging

```cmd
build_vs.bat
```

Opens VS solution at `build_vs_debug\jellyfin-desktop.sln`

Requires Qt debug libraries (install via Qt Maintenance Tool).

## Troubleshooting

### Black Screen / GPU Issues

If the app opens but shows a black screen instead of the web interface:

1. **Try software rendering** (disables GPU acceleration):
   ```cmd
   launch_software_mode.bat
   ```
   Or rebuild with the fix and run:
   ```cmd
   "build\output\Jellyfin Desktop.exe" --software-rendering
   ```

2. **Test different GPU configurations**:
   ```cmd
   try_gpu_workarounds.bat
   ```
   This interactive script tests various rendering modes to find what works.

3. **Gather diagnostics**:
   ```cmd
   diagnose_black_screen.bat
   ```
   Collects logs, GPU info, and process details to `diagnostics/` folder.

Common causes:
- Outdated GPU drivers
- Missing DirectX components
- Hardware acceleration incompatibility
- Older/unsupported graphics cards

### Log Files

Application logs are located at:
```
%LOCALAPPDATA%\Jellyfin Desktop\logs\jellyfin-desktop.log
```

## Notes

- Qt 6.10.1 requires VS 2022 toolset (v143) for ABI compatibility
- QtWebEngine resources auto-copied by build scripts
- Installer created after 5-second countdown (press key to skip)
