#!/bin/bash

# Test script for Jellyfin Media Player deeplink support
# This script tests the deeplink URL scheme handling

echo "=== Jellyfin Media Player Deeplink Test ==="
echo

# Check if application is installed
if ! command -v jellyfinmediaplayer >/dev/null 2>&1; then
    echo "❌ jellyfinmediaplayer command not found"
    echo "Please ensure Jellyfin Media Player is installed and in PATH"
    exit 1
fi

echo "✅ Jellyfin Media Player found"
echo

# Test URLs
declare -a test_urls=(
    "jellyfinmp://auth/callback?code=test-authorization-code&state=test-state-123"
    "jellyfinmp://auth/callback?error=access_denied&state=test-state-456&error_description=User%20cancelled%20login"
    "jellyfinmp:///callback?code=alternative-format&state=test-state-789"
)

echo "🔗 Testing deeplink URLs:"
echo

for url in "${test_urls[@]}"; do
    echo "Testing: $url"
    
    # Platform-specific URL opening
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        xdg-open "$url" 2>/dev/null &
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        open "$url" 2>/dev/null &
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        start "$url" 2>/dev/null &
    else
        echo "  ⚠️  Unknown platform, cannot test URL opening"
        continue
    fi
    
    echo "  ✅ URL sent to system"
    echo "  💡 Check application logs for processing details"
    echo
    
    # Small delay between tests
    sleep 2
done

echo "📋 Test Summary:"
echo "• Success callback: Should log successful code reception"
echo "• Error callback: Should log error details"
echo "• Alternative format: Should work same as standard format"
echo
echo "📊 Check application output for:"
echo "• 'DeepLinkHandler: Processing URL' messages"
echo "• 'DeepLink processed' with result types"
echo "• Any validation errors for invalid states"
echo
echo "🎯 Expected behavior:"
echo "• First launch: App starts and processes deeplink"
echo "• Subsequent launches: Running app receives deeplink"
echo "• Invalid URLs: Logged and rejected"
echo
echo "=== Test completed ==="