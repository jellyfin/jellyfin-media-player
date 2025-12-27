#!/usr/bin/env sh
# Jellyfin Desktop - macOS build script
# Run setup.sh first to install dependencies
set -eu

SCRIPT_DIR="$(cd "$(dirname "${0}")" && pwd)"
. "${SCRIPT_DIR}/common.sh"

QTROOT="${DEPS_DIR}/qt/${QT_VERSION}/macos"

# Check dependencies
if [ ! -d "${QTROOT}" ]; then
    echo "error: Qt not found at ${QTROOT}" >&2
    echo "Run setup.sh first" >&2
    exit 1
fi

if ! command -v mpv > /dev/null; then
    echo "error: mpv not found. Run setup.sh first" >&2
    exit 1
fi

echo "Using Qt: ${QTROOT}"

# Configure
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "Configuring..."
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=output \
    -DQTROOT="${QTROOT}" \
    -DCMAKE_PREFIX_PATH="${QTROOT}" \
    -DUSE_STATIC_MPVQT=ON \
    "${PROJECT_ROOT}"

# Build
echo "Building..."
ninja

echo ""
echo "Build complete!"
echo "App bundle: ${BUILD_DIR}/src/${APP_NAME}"
