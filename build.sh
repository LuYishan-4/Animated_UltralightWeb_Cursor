#!/usr/bin/env bash
set -euo pipefail

#  just for me test
cd "$(dirname "$0")"

ROOT_DIR="$PWD"

if [ ! -d "sdk" ]; then
    mkdir -p sdk
fi

cd sdk
if [ ! -d "ultralight-free-sdk-1.4.0-linux-x64" ]; then
    7z x ultralight-free-sdk-1.4.0-linux-x64.7z
fi
cd "$ROOT_DIR"

configure_variant() {
    local build_dir="$1"
    local desktop_env="${2:-}"
    local cmake_args=(
        -S "$ROOT_DIR"
        -B "$ROOT_DIR/$build_dir"
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=/usr
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    )

    if [ -n "$desktop_env" ]; then
        cmake_args+=("-DFORCE_DESKTOP_ENVIRONMENT=$desktop_env")
    fi

    echo "Configuring $build_dir..."
    if cmake "${cmake_args[@]}"; then
        if [ -f "$ROOT_DIR/$build_dir/compile_commands.json" ]; then
            echo "compile_commands.json generated: $build_dir/compile_commands.json"
        fi
    else
        echo "warning: failed to configure $build_dir"
    fi
}

rm -rf build
mkdir -p build

configure_variant "build" ""

cmake --build build

cd build
cmake --install .
