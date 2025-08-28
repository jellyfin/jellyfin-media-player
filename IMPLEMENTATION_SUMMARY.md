# Jellyfin Media Player Deeplink Implementation Summary

## Implementation Overview

This implementation adds comprehensive deeplink support to Jellyfin Media Player, enabling external applications and web browsers to launch the player with specific actions.

## Key Features Implemented

### 1. Core Deeplink Handler (`src/system/DeepLinkHandler.h/cpp`)
- New component class that handles URL parsing and validation
- Supports multiple URL schemes: `jellyfin://` and `jmp://`
- Three main actions: `play`, `connect`, and `navigate`
- Comprehensive security validation and sanitization

### 2. Command Line Integration (`src/main.cpp`)
- Added `--deeplink` command line option
- Support for positional arguments (for OS protocol handlers)
- Automatic detection of deeplink URLs in command line arguments

### 3. Single Instance Support (`src/shared/UniqueApplication.h`)
- Extended UniqueApplication to pass deeplink URLs between instances
- IPC mechanism to send deeplinks to already running instances
- Graceful handling of multiple launch attempts

### 4. Component System Integration (`src/core/ComponentManager.cpp`)
- DeepLinkHandler registered as a system component
- Available to QML/JavaScript layer for web integration
- Proper initialization and lifecycle management

### 5. Platform-Specific URL Scheme Registration

#### Windows (`resources/jellyfin-protocol.reg`)
- Registry entries for both `jellyfin://` and `jmp://` schemes
- Installation script (`scripts/register-protocols-windows.bat`)

#### macOS (`bundle/osx/Info.plist.in`)
- CFBundleURLTypes configuration for URL scheme handling
- Support for both URL schemes in app bundle

#### Linux (`resources/meta/com.github.iwalton3.jellyfin-media-player.desktop`)
- MIME type associations for URL schemes
- Installation script (`scripts/register-protocols-linux.sh`)

## Security Features

### URL Validation
- Only approved URL schemes accepted (`jellyfin://`, `jmp://`)
- Malformed URL rejection with logging
- Invalid scheme detection and blocking

### Parameter Sanitization
- JavaScript injection prevention (`javascript:` scheme blocking)
- Data URI attack prevention (`data:` scheme blocking)  
- HTML/Script tag filtering (`<script>` detection)
- Length limits (2048 character maximum per parameter)
- Dangerous parameter removal with logging

### Input Validation
- URL encoding handling
- Special character sanitization
- Buffer overflow prevention

## Supported URL Formats

### Play Action
```
jellyfin://play?server=https://server.com&item=12345
jmp://play?server=https://demo.jellyfin.org&item=movie-id
```

### Connect Action
```
jellyfin://connect?server=https://server.com
jellyfin://connect?server=https://server.com&token=auth-token
```

### Navigate Action
```
jellyfin://navigate?path=/movies
jmp://navigate?path=/tv
```

## Technical Architecture

### Component Hierarchy
```
main() 
├── CommandLineParser (handles --deeplink and positional args)
├── UniqueApplication (handles instance management and IPC)
├── ComponentManager (registers DeepLinkHandler)
└── DeepLinkHandler (processes URLs and emits signals)
```

### Signal Flow
```
URL Input → Parsing → Validation → Action Handler → Signal Emission
```

### IPC Communication
```
Instance 1: URL received → UniqueApplication.ensureUnique(url)
Instance 2: Already running → Receive via LocalJsonServer → Process URL
```

## Testing Coverage

### Functional Tests
- ✅ Basic play URL processing
- ✅ Connect URL with server and token
- ✅ Navigate URL with path parameters
- ✅ Alternative `jmp://` scheme support
- ✅ Invalid scheme rejection

### Security Tests
- ✅ JavaScript injection blocking
- ✅ Data URI attack prevention
- ✅ Long parameter sanitization
- ✅ HTML tag filtering

### Integration Tests
- ✅ Command line argument processing
- ✅ Positional argument handling
- ✅ Component registration and initialization
- ✅ Single instance behavior

## Installation and Deployment

### Build Integration
- DeepLinkHandler automatically compiled with existing build system
- No additional dependencies required
- Cross-platform compatible

### Runtime Requirements
- Qt5 framework (existing dependency)
- Standard C++ libraries
- Platform-specific IPC mechanisms

### Installation Scripts
- Windows: `scripts/register-protocols-windows.bat`
- Linux: `scripts/register-protocols-linux.sh`
- macOS: Automatic via Info.plist in app bundle

## Documentation

### User Documentation
- `DEEPLINK_SUPPORT.md` - Comprehensive user guide
- URL format specifications
- Security considerations
- Troubleshooting information

### Developer Documentation
- Code comments and documentation
- API reference for component integration
- Test cases and examples

## Future Extensibility

### Framework for New Actions
The implementation provides a clean framework for adding new deeplink actions:

1. Add new action handler in `DeepLinkHandler::handleXxxxAction()`
2. Add corresponding signal in the header file
3. Update URL parsing logic
4. Add validation rules

### Plugin Integration
The component-based architecture allows for:
- Custom action handlers via plugins
- Extended parameter validation
- Additional URL schemes

### Web Integration
The QML/JavaScript exposure enables:
- Web client deeplink generation
- Browser integration
- Progressive web app support

## Conclusion

This implementation provides a robust, secure, and extensible deeplink system for Jellyfin Media Player that:

- Maintains security as a top priority
- Integrates cleanly with existing architecture
- Supports all major platforms
- Provides comprehensive error handling
- Includes thorough testing and documentation
- Follows Qt/C++ best practices

The minimal code changes ensure compatibility while adding powerful new functionality for external integrations and user convenience.