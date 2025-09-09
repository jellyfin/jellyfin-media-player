# Desktop Deeplink Support

Jellyfin Media Player supports custom URL scheme deeplinks to enable seamless OIDC (OpenID Connect) authentication flows.

## URL Scheme

**Scheme:** `jellyfinmp://`

### Supported Patterns

#### Authentication Callback (Primary Use Case)
- **Success:** `jellyfinmp://auth/callback?code=AUTHORIZATION_CODE&state=STATE_VALUE`
- **Error:** `jellyfinmp://auth/callback?error=ERROR_CODE&state=STATE_VALUE&error_description=DESCRIPTION`

#### Alternative URL Format
- `jellyfinmp:///callback?...` (same parameters as above)

### Reserved for Future Use
- `jellyfinmp://play?itemId=...` (media playback)
- `jellyfinmp://server/add?url=...` (server configuration)

## Platform Registration

### Windows
URL scheme registration is handled automatically during installation via Windows Registry entries:
- Protocol: `HKLM\Software\Classes\jellyfinmp`
- Command: `"C:\Program Files\Jellyfin Media Player\JellyfinMediaPlayer.exe" "%1"`

### macOS
URL scheme is registered in the application bundle's `Info.plist`:
```xml
<key>CFBundleURLTypes</key>
<array>
    <dict>
        <key>CFBundleURLName</key>
        <string>Jellyfin Media Player Auth Callback</string>
        <key>CFBundleURLSchemes</key>
        <array>
            <string>jellyfinmp</string>
        </array>
    </dict>
</array>
```

### Linux
URL scheme is registered via the `.desktop` file:
```ini
MimeType=x-scheme-handler/jellyfinmp;
```

After installation, run: `update-desktop-database`

## OIDC Authentication Flow

### 1. Initiate Authentication
The application constructs an authorization URL with:
- `redirect_uri=jellyfinmp://auth/callback`
- `state=RANDOM_STATE_VALUE` (for CSRF protection)

### 2. User Authentication
- Browser opens for user to authenticate with identity provider
- Application stores the `state` value temporarily (5-minute timeout)

### 3. Authorization Callback
- Identity provider redirects to `jellyfinmp://auth/callback?...`
- OS launches/focuses Jellyfin Media Player with the URL
- Application validates the `state` parameter
- Application processes the authorization code or error

### 4. Token Exchange
- If successful, application exchanges authorization code for tokens
- UI updates to reflect authentication status

## Security Features

### State Validation
- All pending states are stored with timestamps
- States expire after 5 minutes
- Unknown/expired states are rejected
- Used states are immediately removed

### URL Validation
- Only `jellyfinmp://` scheme accepted
- Path must be `/callback`
- Host must be empty or `auth`
- Malformed URLs are rejected

### Single Instance Handling
- If multiple instances are launched, deeplinks are forwarded to the running instance
- New instances exit after forwarding the deeplink

## Error Handling

### Common Error Scenarios
- **Invalid URL:** URL doesn't match expected pattern
- **Invalid State:** State parameter missing, unknown, or expired
- **Access Denied:** User cancelled authentication
- **Server Error:** Identity provider returned an error

### Error Messages
Errors are logged with appropriate detail level:
- Production: Error type and state (no sensitive data)
- Debug: Full URL and parameter details

## Developer Integration

### Registering Pending States
```cpp
DeepLinkHandler* handler = new DeepLinkHandler();
handler->registerPendingState("your-state-value", 5); // 5 minutes
```

### Handling Callbacks
```cpp
connect(uniqueApp, &UniqueApplication::deepLinkReceived, 
        [handler](const QString& url) {
    auto result = handler->handleDeepLink(url);
    switch (result.type) {
        case DeepLinkHandler::AuthCallbackResult::Success:
            // Process result.code and result.state
            break;
        case DeepLinkHandler::AuthCallbackResult::Error:
            // Handle result.error and result.errorDescription
            break;
        // ... other cases
    }
});
```

## Testing

### Manual Testing
Test deeplink handling by running these commands:

#### Windows
```cmd
start jellyfinmp://auth/callback?code=test-code&state=test-state
```

#### macOS
```bash
open "jellyfinmp://auth/callback?code=test-code&state=test-state"
```

#### Linux
```bash
xdg-open "jellyfinmp://auth/callback?code=test-code&state=test-state"
```

### Expected Behavior
1. If app is not running: Application launches and processes the deeplink
2. If app is running: Application receives deeplink and processes it
3. Invalid URLs or states are logged and rejected

## Build Configuration

No special build flags are required. The deeplink support is automatically included when building Jellyfin Media Player.

### Dependencies
- Qt5 Core (QUrl, QUrlQuery, QTimer)
- Qt5 Network (for local socket communication)

## Troubleshooting

### URL Scheme Not Registered
- **Windows:** Reinstall the application or run installer as administrator
- **macOS:** Check that the app bundle contains the correct `Info.plist`
- **Linux:** Run `update-desktop-database` and verify `.desktop` file installation

### Deeplinks Not Working
1. Check application logs for error messages
2. Verify URL format matches expected patterns
3. Ensure state was registered before callback
4. Check that single instance is running

### Multiple Instances
If multiple instances are starting instead of using the existing one:
- Check that the local socket communication is working
- Verify no firewall is blocking local connections
- Ensure adequate permissions for socket creation