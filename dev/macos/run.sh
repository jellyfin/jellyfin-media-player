#!/usr/bin/env sh
# Jellyfin Desktop - Run built app
# Run build.sh first
set -eu

SCRIPT_DIR="$(cd "$(dirname "${0}")" && pwd)"
. "${SCRIPT_DIR}/common.sh"

QTROOT="${DEPS_DIR}/qt/${QT_VERSION}/macos"
APP_PATH="${BUILD_DIR}/src/${APP_NAME}"

# Check build exists
if [ ! -d "${APP_PATH}" ]; then
    echo "error: Build not found. Run build.sh first" >&2
    exit 1
fi

# Run with Qt libs from aqt installation (unbundled dev build)
export DYLD_FRAMEWORK_PATH="${QTROOT}/lib"
export QT_PLUGIN_PATH="${QTROOT}/plugins"
export QML_IMPORT_PATH="${QTROOT}/qml"
exec "${APP_PATH}/Contents/MacOS/Jellyfin Desktop" "${@}"
