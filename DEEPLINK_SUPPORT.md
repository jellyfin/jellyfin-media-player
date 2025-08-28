# Jellyfin Media Player Deeplink Support

This document describes the deeplink support implementation for Jellyfin Media Player, allowing the application to be launched from external sources with specific parameters or actions.

## Overview

Deeplink support enables external applications, web browsers, and system integrations to launch Jellyfin Media Player with predefined actions such as:

- Playing specific media items
- Connecting to Jellyfin servers
- Navigating to specific sections of the interface

## Supported URL Schemes

Jellyfin Media Player supports two URL schemes:

- `jellyfin://` - Primary scheme
- `jmp://` - Alternative scheme

## URL Format

The general format for deeplink URLs is:

```
jellyfin://action?parameter1=value1&parameter2=value2
```

## Supported Actions

### 1. Play Action

Starts playback of a specific media item.

**Format:** `jellyfin://play?server=<server_url>&item=<item_id>`

**Parameters:**
- `server` (required): The Jellyfin server URL
- `item` (required): The media item ID to play
- Additional parameters are passed through to the playback system

**Examples:**
```
jellyfin://play?server=https://demo.jellyfin.org&item=12345
jmp://play?server=https://my-server.com:8096&item=movie-abc123
```

### 2. Connect Action

Connects to a Jellyfin server, optionally with authentication.

**Format:** `jellyfin://connect?server=<server_url>&token=<auth_token>`

**Parameters:**
- `server` (required): The Jellyfin server URL  
- `token` (optional): Authentication token for automatic login

**Examples:**
```
jellyfin://connect?server=https://my-server.com
jellyfin://connect?server=https://demo.jellyfin.org&token=abc123xyz789
```

### 3. Navigate Action

Navigates to a specific section or path within the application.

**Format:** `jellyfin://navigate?path=<navigation_path>`

**Parameters:**
- `path` (required): The navigation path within the application

**Examples:**
```
jellyfin://navigate?path=/movies
jellyfin://navigate?path=/tv
jellyfin://navigate?path=/music
```

## Security Features

The deeplink implementation includes several security measures:

1. **URL Scheme Validation**: Only `jellyfin://` and `jmp://` schemes are accepted
2. **Parameter Sanitization**: Dangerous content is detected and removed
3. **Length Limits**: Parameters longer than 2048 characters are rejected
4. **Script Injection Prevention**: JavaScript and data URIs are blocked
5. **HTML Tag Filtering**: Script tags and other dangerous HTML is filtered out

## Platform Integration

### Windows

URL scheme registration is handled through Windows Registry entries. The installer should register the protocol handlers:

```reg
[HKEY_CLASSES_ROOT\jellyfin]
@="URL:Jellyfin Media Player Protocol"
"URL Protocol"=""

[HKEY_CLASSES_ROOT\jellyfin\shell\open\command]
@="\"C:\\Program Files\\Jellyfin Media Player\\JellyfinMediaPlayer.exe\" \"%1\""
```

### macOS

URL schemes are registered in the application's `Info.plist`:

```xml
<key>CFBundleURLTypes</key>
<array>
    <dict>
        <key>CFBundleURLName</key>
        <string>Jellyfin Media Player Protocol</string>
        <key>CFBundleURLSchemes</key>
        <array>
            <string>jellyfin</string>
        </array>
    </dict>
</array>
```

### Linux

Protocol handling is registered through desktop entries with MIME type associations:

```desktop
[Desktop Entry]
Name=Jellyfin Media Player
Exec=jellyfinmediaplayer %u
MimeType=x-scheme-handler/jellyfin;x-scheme-handler/jmp;
```

## Command Line Usage

Deeplinks can also be processed via command line:

```bash
# Using the --deeplink option
jellyfinmediaplayer --deeplink "jellyfin://play?server=https://demo.jellyfin.org&item=12345"

# As a positional argument (for protocol handlers)
jellyfinmediaplayer "jellyfin://connect?server=https://my-server.com"
```

## Single Instance Behavior

When Jellyfin Media Player is already running and a deeplink is triggered:

1. The new instance detects the existing instance
2. The deeplink URL is passed to the running instance via IPC
3. The running instance processes the deeplink and performs the requested action
4. The new instance exits, leaving the original instance active

## Implementation Details

The deeplink functionality is implemented through several key components:

- **DeepLinkHandler**: Core component that parses and processes deeplink URLs
- **UniqueApplication**: Extended to support passing deeplink data between instances
- **Command Line Parser**: Extended to accept deeplink URLs as arguments
- **Component Manager**: Registers the DeepLinkHandler component

## Error Handling

The system handles various error conditions gracefully:

- Invalid URL schemes are rejected with appropriate logging
- Malformed URLs are parsed safely with error reporting
- Security violations are logged and blocked
- Missing required parameters generate warning messages

## Testing

The deeplink functionality can be tested using the provided test scripts:

```bash
# Basic functionality tests
./test-deeplinks.sh

# Security validation tests  
./test-deeplinks-security.sh
```

## Future Enhancements

Potential future improvements could include:

- Additional action types (queue, shuffle, etc.)
- Playlist management via deeplinks
- User account switching
- Advanced navigation paths
- Custom plugin actions

## Troubleshooting

### Common Issues

1. **Protocol not registered**: Ensure the URL scheme is properly registered with the operating system
2. **Security blocks**: Check logs for security violations if deeplinks are being rejected
3. **Parameter encoding**: Ensure special characters in URLs are properly encoded
4. **Server connectivity**: Verify that server URLs are accessible and valid

### Debug Information

Enable terminal logging to see detailed deeplink processing:

```bash
jellyfinmediaplayer --terminal --deeplink "your-url-here"
```

This will output detailed information about URL parsing, validation, and action handling.