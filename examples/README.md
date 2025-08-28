# SSO Deeplink Integration Guide

This guide explains how to integrate Single Sign-On (SSO) authentication with Jellyfin Media Player using deeplinks.

## Overview

The SSO deeplink integration allows external applications, web pages, and system integrations to trigger SSO authentication flows in Jellyfin Media Player. This enables seamless authentication experiences where users can log in through their preferred SSO provider and have the results automatically applied to the media player.

## Features

- **Deeplink SSO URLs**: `jellyfin://sso?server=<url>&return_url=<optional>`
- **Enhanced SSO Script**: Automatic detection of deeplink capabilities with fallback support
- **Cross-Platform Support**: Works on Windows, macOS, and Linux
- **Security**: URL validation, parameter sanitization, and secure authentication handling
- **Multiple Integration Methods**: Command line, web links, JavaScript APIs, and system calls

## Files Included

- `resources/sso-deeplink-integration.js` - Enhanced SSO plugin script with deeplink support
- `examples/sso-deeplink-demo.html` - Interactive demo and integration examples
- `test-sso-deeplink.sh` - Test script for validating SSO deeplink functionality

## Quick Start

### 1. Install Jellyfin Media Player with Deeplink Support

Ensure you have Jellyfin Media Player installed with deeplink support enabled and protocol handlers registered.

### 2. Include the SSO Integration Script

Add the enhanced SSO script to your Jellyfin web interface:

```html
<script src="resources/sso-deeplink-integration.js"></script>
```

### 3. Configure SSO Settings

Edit the script configuration at the top of `sso-deeplink-integration.js`:

```javascript
// Configure your SSO endpoint
const SSO_AUTH_URL = 'https://jellyfin.yourdomain.com/sso/OID/start/';
```

### 4. Test the Integration

Open the demo page in your browser:
```bash
open examples/sso-deeplink-demo.html
```

Or test directly with a deeplink URL:
```bash
# Linux
xdg-open 'jellyfin://sso?server=https://your-server.com'

# Windows  
start jellyfin://sso?server=https://your-server.com

# macOS
open 'jellyfin://sso?server=https://your-server.com'
```

## Usage Examples

### HTML Integration

```html
<!-- Basic SSO link -->
<a href="jellyfin://sso?server=https://my-server.com">
    Login with SSO in Jellyfin Media Player
</a>

<!-- SSO with return URL -->
<a href="jellyfin://sso?server=https://my-server.com&return_url=https://my-server.com/web/dashboard">
    Login and go to Dashboard
</a>
```

### JavaScript Integration

```javascript
// Using the provided utility functions
if (typeof window.generateSsoDeeplink === 'function') {
    const ssoUrl = window.generateSsoDeeplink('https://my-server.com', window.location.href);
    window.location.href = ssoUrl;
}

// Programmatic SSO authentication  
if (typeof window.handleSsoViaDeeplink === 'function') {
    window.handleSsoViaDeeplink('https://my-server.com', 'https://success-page.com')
        .then(result => console.log('SSO successful:', result))
        .catch(error => console.error('SSO failed:', error));
}
```

### Command Line Usage

```bash
# Direct deeplink processing
jellyfinmediaplayer --deeplink "jellyfin://sso?server=https://my-server.com"

# System protocol handler
jellyfinmediaplayer "jellyfin://sso?server=https://my-server.com&return_url=https://my-server.com/web"
```

### Node.js/Electron Integration

```javascript
const { shell } = require('electron');

function launchJellyfinSSO(serverUrl, returnUrl) {
    const ssoUrl = `jellyfin://sso?server=${encodeURIComponent(serverUrl)}`;
    const fullUrl = returnUrl ? `${ssoUrl}&return_url=${encodeURIComponent(returnUrl)}` : ssoUrl;
    
    shell.openExternal(fullUrl);
}

// Usage
launchJellyfinSSO('https://my-jellyfin-server.com', 'https://success-page.com');
```

## Advanced Configuration

### Custom SSO Flow

You can customize the SSO authentication flow by modifying the script's behavior:

```javascript
// Override the SSO URL generation
function generateSsoDeeplink(serverUrl, returnUrl) {
    // Add custom parameters or modify URL structure
    const baseUrl = `jellyfin://sso?server=${encodeURIComponent(serverUrl)}`;
    if (returnUrl) {
        return `${baseUrl}&return_url=${encodeURIComponent(returnUrl)}&custom=value`;
    }
    return baseUrl;
}
```

### Authentication Status Detection

Customize how the script detects successful authentication:

```javascript
function checkAuthenticationStatus() {
    // Check your specific authentication indicators
    const hasToken = localStorage.getItem('your_auth_token');
    const isLoggedIn = !document.querySelector('.login-required');
    return hasToken && isLoggedIn;
}
```

## Troubleshooting

### Common Issues

1. **Deeplinks not working**: Ensure protocol handlers are registered
   - Windows: Run `scripts/register-protocols-windows.bat` as Administrator
   - Linux: Run `scripts/register-protocols-linux.sh`
   - macOS: Automatic via app bundle

2. **SSO overlay not appearing**: Check browser console for errors and ensure the script is loaded

3. **Authentication not persisting**: Verify that your SSO provider is correctly setting authentication cookies/tokens

### Debug Mode

Enable debug logging by adding this to your script:

```javascript
// Add at the beginning of the SSO script
window.SSO_DEBUG = true;
```

### Testing

Run the provided test script to validate functionality:

```bash
./test-sso-deeplink.sh
```

## Security Considerations

- **URL Validation**: All deeplink URLs are validated for proper scheme and format
- **Parameter Sanitization**: Dangerous parameters are filtered out
- **Length Limits**: Parameters are limited to 2048 characters
- **Origin Validation**: PostMessage events are validated for proper origin
- **Token Handling**: Authentication tokens are handled securely

## Browser Compatibility

- **Chrome/Chromium**: Full support for protocol handlers
- **Firefox**: Full support with manual protocol registration
- **Safari**: Full support on macOS  
- **Edge**: Full support on Windows
- **Mobile Browsers**: Limited support (depends on OS integration)

## Contributing

When contributing to the SSO deeplink integration:

1. Test across all supported platforms
2. Validate security measures
3. Update documentation and examples
4. Run the test suite before submitting changes

## Support

For issues and questions:

1. Check the [troubleshooting section](#troubleshooting)
2. Run the test script to identify configuration issues
3. Check Jellyfin Media Player logs for deeplink processing errors
4. Verify SSO provider configuration and endpoints

## License

This SSO deeplink integration follows the same license as Jellyfin Media Player.