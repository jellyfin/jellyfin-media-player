#!/usr/bin/env sh
# Jellyfin Desktop - Common variables
# Sourced by other scripts

QT_VERSION=6.10.1

SCRIPT_DIR="$(cd "$(dirname "${0}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
DEPS_DIR="${SCRIPT_DIR}/deps"
BUILD_DIR="${PROJECT_ROOT}/build"
APP_NAME="Jellyfin Desktop.app"
