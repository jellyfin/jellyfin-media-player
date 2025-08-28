#!/bin/bash

# Linux Script to Register Jellyfin Media Player URL Protocols
# Registers jellyfin:// and jmp:// URL schemes

echo "Registering Jellyfin Media Player URL protocols..."

# Get the directory where jellyfinmediaplayer is installed
INSTALL_DIR=""
if command -v jellyfinmediaplayer >/dev/null 2>&1; then
    INSTALL_DIR=$(dirname $(which jellyfinmediaplayer))
    echo "Found Jellyfin Media Player at: $INSTALL_DIR/jellyfinmediaplayer"
else
    echo "Warning: jellyfinmediaplayer not found in PATH"
    echo "Please ensure Jellyfin Media Player is installed and accessible"
fi

# Create the desktop entry for protocol handling
DESKTOP_FILE="$HOME/.local/share/applications/jellyfin-media-player-url-handler.desktop"

mkdir -p "$(dirname "$DESKTOP_FILE")"

cat > "$DESKTOP_FILE" << EOF
[Desktop Entry]
Version=1.0
Name=Jellyfin Media Player URL Handler
Comment=Handle jellyfin:// and jmp:// URLs
Exec=jellyfinmediaplayer %u
Icon=com.github.iwalton3.jellyfin-media-player
Terminal=false
NoDisplay=true
Type=Application
MimeType=x-scheme-handler/jellyfin;x-scheme-handler/jmp;
StartupNotify=true
EOF

# Update the desktop database
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$HOME/.local/share/applications"
    echo "Updated desktop database"
fi

# Set default handler for the URL schemes
if command -v xdg-mime >/dev/null 2>&1; then
    xdg-mime default jellyfin-media-player-url-handler.desktop x-scheme-handler/jellyfin
    xdg-mime default jellyfin-media-player-url-handler.desktop x-scheme-handler/jmp
    echo "Set default URL handlers"
fi

echo ""
echo "URL protocols registered successfully!"
echo "Desktop entry created at: $DESKTOP_FILE"
echo ""
echo "You can now use jellyfin:// and jmp:// URLs to launch Jellyfin Media Player."
echo ""
echo "Test with:"
echo "  xdg-open 'jellyfin://connect?server=https://demo.jellyfin.org'"
echo "  xdg-open 'jmp://navigate?path=/movies'"
echo ""
echo "To verify registration:"
echo "  xdg-mime query default x-scheme-handler/jellyfin"
echo "  xdg-mime query default x-scheme-handler/jmp"