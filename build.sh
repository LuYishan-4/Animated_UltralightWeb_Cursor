#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"
ROOT_DIR="$PWD"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build-linux}"
PREFIX="${PREFIX:-/usr/local}"
ACTION="${1:-build}"
SDK_ARCHIVE="$ROOT_DIR/sdk/ultralight-free-sdk-1.4.0-linux-x64.7z"
SDK_DIR="$ROOT_DIR/sdk/ultralight-free-sdk-1.4.0-linux-x64"

usage() {
    cat <<'EOF'
Usage: ./build.sh [build|install|clean]

  build    Configure and compile the Linux X11/XWayland application (default)
  install  Build and install to PREFIX (/usr/local)
  clean    Remove generated Linux build directories

Environment:
  PREFIX=/usr       Choose an install prefix
  BUILD_DIR=path    Choose the CMake build directory
EOF
}

prepare_sdk() {
    if [ -f "$SDK_DIR/include/AppCore/App.h" ] &&
       [ -f "$SDK_DIR/resources/cacert.pem" ] &&
       [ -f "$SDK_DIR/bin/libUltralight.so" ]; then
        return
    fi

    if [ ! -f "$SDK_ARCHIVE" ]; then
        echo "error: missing $SDK_ARCHIVE" >&2
        exit 1
    fi
    command -v 7z >/dev/null 2>&1 || {
        echo "error: 7z is required to extract the Ultralight SDK" >&2
        exit 1
    }

    echo "Extracting Ultralight SDK..."
    rm -rf "$SDK_DIR"
    mkdir -p "$SDK_DIR"
    7z x "$SDK_ARCHIVE" "-o$SDK_DIR" -y >/dev/null

    if [ ! -f "$SDK_DIR/include/AppCore/App.h" ] ||
       [ ! -f "$SDK_DIR/resources/icudt67l.dat" ]; then
        echo "error: extracted Ultralight SDK is incomplete" >&2
        exit 1
    fi
}

configure_and_build() {
    prepare_sdk
    cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build "$BUILD_DIR" --parallel
    echo
    echo "Build complete:"
    echo "  $BUILD_DIR/bin/ultralightwebcursor-gui"
    echo "  $BUILD_DIR/bin/ultralightwebcursor-engine"
}


case "$ACTION" in
    build)
        configure_and_build
        ;;
    install)
        configure_and_build
        echo "Installing to $PREFIX..."
        sudo cmake --install "$BUILD_DIR"
        if command -v update-desktop-database >/dev/null 2>&1; then
            sudo update-desktop-database "$PREFIX/share/applications" \
                >/dev/null 2>&1 || true
        fi
        if command -v gtk-update-icon-cache >/dev/null 2>&1; then
            sudo gtk-update-icon-cache -q -t -f \
                "$PREFIX/share/icons/hicolor" >/dev/null 2>&1 || true
        fi
        rm -f "$HOME/.config/ultralightwebcursor/variant"
        echo
        echo "Installed. Search for 'Ultralight Web Cursor' or run:"
        echo "  $PREFIX/bin/ultralightwebcursor-gui"
        ;;
    clean)
        rm -rf "$BUILD_DIR" "$ROOT_DIR/build-windows"
        echo "Build directories removed."
        ;;
    -h|--help|help)
        usage
        ;;
    *)
        usage >&2
        exit 2
        ;;
esac
