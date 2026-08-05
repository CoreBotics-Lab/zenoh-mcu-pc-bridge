#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

THIRDPARTY_DIR="${SCRIPT_DIR}/3rdparty"

echo "=== Setting up local C++ 3rdparty dependencies (nlohmann/json & zenoh-c) ==="
mkdir -p "${THIRDPARTY_DIR}/nlohmann"

# 1. Download nlohmann/json.hpp
if [ ! -f "${THIRDPARTY_DIR}/nlohmann/json.hpp" ]; then
    echo "Downloading nlohmann/json.hpp..."
    curl -L -o "${THIRDPARTY_DIR}/nlohmann/json.hpp" "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
else
    echo "nlohmann/json.hpp already exists."
fi

# 2. Download and extract zenoh-c standalone library
if [ ! -d "${THIRDPARTY_DIR}/zenoh-c" ]; then
    echo "Detecting system OS and architecture..."
    OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
    ARCH="$(uname -m)"

    # Map architecture
    if [ "$ARCH" = "x86_64" ]; then
        RUST_ARCH="x86_64"
    elif [ "$ARCH" = "aarch64" ] || [ "$ARCH" = "arm64" ]; then
        RUST_ARCH="aarch64"
    else
        echo "Error: Unsupported architecture $ARCH"
        exit 1
    fi

    # Map OS
    if [ "$OS" = "linux" ]; then
        PACKAGE_OS="unknown-linux-gnu-standalone"
    elif [ "$OS" = "darwin" ]; then
        PACKAGE_OS="apple-darwin-standalone"
    else
        echo "Error: Unsupported OS $OS"
        exit 1
    fi

    ZIP_NAME="zenoh-c-1.9.0-${RUST_ARCH}-${PACKAGE_OS}.zip"
    URL="https://github.com/eclipse-zenoh/zenoh-c/releases/download/1.9.0/${ZIP_NAME}"

    echo "Downloading zenoh-c standalone package from:"
    echo "  $URL"

    mkdir -p "${THIRDPARTY_DIR}/zenoh-c"
    curl -L -o "${THIRDPARTY_DIR}/zenoh_tmp.zip" "$URL"
    
    echo "Extracting zenoh-c package..."
    unzip -q "${THIRDPARTY_DIR}/zenoh_tmp.zip" -d "${THIRDPARTY_DIR}/zenoh-c"
    rm "${THIRDPARTY_DIR}/zenoh_tmp.zip"
else
    echo "${THIRDPARTY_DIR}/zenoh-c directory already exists."
fi

# 3. Build C++ PC nodes if zenoh_pc_nodes/cpp directory exists
PC_CPP_NODES_DIR="$(cd "${SCRIPT_DIR}/../../zenoh_pc_nodes/cpp" 2>/dev/null && pwd || true)"
if [ -n "$PC_CPP_NODES_DIR" ] && [ -f "${PC_CPP_NODES_DIR}/CMakeLists.txt" ]; then
    echo "=== Building C++ PC nodes ==="
    mkdir -p "${PC_CPP_NODES_DIR}/build"
    cd "${PC_CPP_NODES_DIR}/build"
    cmake ..
    make
fi

echo "=== Setup complete! ==="
