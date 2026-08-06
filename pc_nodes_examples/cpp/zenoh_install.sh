#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SHARED_CPP_INSTALL="$(cd "${SCRIPT_DIR}/../../shared_libraries/cpp" && pwd)/zenoh_install.sh"

if [ -f "$SHARED_CPP_INSTALL" ]; then
    exec "$SHARED_CPP_INSTALL" "$@"
else
    echo "Error: Could not find $SHARED_CPP_INSTALL"
    exit 1
fi