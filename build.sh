#!/usr/bin/env bash
set -euo pipefail

# Unified build/install entry point for every Linux variant.
#
#   ./build.sh            # ask which variant to build (KDE or X11)
#   ./build.sh kde        # build + install the KWin plugin variant
#   ./build.sh x11        # build + install the standalone X11 variant
#
# The interactive prompt defaults to KDE on a KDE/Wayland session and to X11
# otherwise, so pressing Enter picks the sensible choice for the machine.

cd "$(dirname "$0")"
ROOT_DIR="$PWD"

# ---- Ultralight SDK -------------------------------------------------------
if [ ! -d "sdk" ]; then
    mkdir -p sdk
fi

if [ ! -f "sdk/ultralight-free-sdk-1.4.0-linux-x64/include/AppCore/App.h" ]; then
    if [ -f "sdk/ultralight-free-sdk-1.4.0-linux-x64.7z" ]; then
        echo "Extracting Ultralight SDK..."
        (cd sdk && 7z x ultralight-free-sdk-1.4.0-linux-x64.7z \
            -oultralight-free-sdk-1.4.0-linux-x64 >/dev/null)
    else
        echo "error: sdk/ultralight-free-sdk-1.4.0-linux-x64.7z not found" >&2
        exit 1
    fi
fi

# ---- variant selection ----------------------------------------------------
detect_default_variant() {
    local session="${XDG_SESSION_TYPE:-}"
    local desktop="${XDG_CURRENT_DESKTOP:-${XDG_SESSION_DESKTOP:-}}"
    if [ "$(echo "$session" | tr '[:upper:]' '[:lower:]')" = "wayland" ] ||
       echo "$desktop" | grep -qi "kde"; then
        echo "kde"
    else
        echo "x11"
    fi
}

choose_variant() {
    local default_variant="$1"
    local default_choice answer
    if [ "$default_variant" = "kde" ]; then default_choice=1; else default_choice=2; fi

    echo "Which variant do you want to build and install?"
    echo "  [1] KDE  (KWin effect, Wayland / Plasma)"
    echo "  [2] X11  (standalone overlay)"
    printf "Selection (default %s): " "$default_choice"
    read -r answer || answer=""
    answer="${answer:-$default_choice}"

    case "$answer" in
        1|kde|KDE) echo "kde" ;;
        2|x11|X11) echo "x11" ;;
        *) echo "Invalid selection: $answer" >&2; exit 2 ;;
    esac
}

VARIANT="${1:-}"
if [ -z "$VARIANT" ]; then
    VARIANT="$(choose_variant "$(detect_default_variant)")"
else
    VARIANT="$(echo "$VARIANT" | tr '[:upper:]' '[:lower:]')"
fi

if [ "$VARIANT" != "kde" ] && [ "$VARIANT" != "x11" ]; then
    echo "error: unknown variant '$VARIANT' (expected 'kde' or 'x11')" >&2
    exit 2
fi

# ---- configure / build / install -----------------------------------------
BUILD_DIR="build-$VARIANT"

echo "Configuring $VARIANT variant in $BUILD_DIR..."
cmake -S "$ROOT_DIR" -B "$ROOT_DIR/$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DFORCE_DESKTOP_ENVIRONMENT="$VARIANT" \
    -DBUILD_INSTALLER=OFF

cmake --build "$BUILD_DIR" --parallel

echo "Installing $VARIANT variant (sudo required)..."
sudo cmake --install "$BUILD_DIR"

echo
echo "Done. Installed the $VARIANT variant."
if [ "$VARIANT" = "kde" ]; then
    echo "The KWin effect will be available after a re-login (or load it from System Settings)."
else
    echo "Run 'ultralightwebcursor-install' to finish the X11 setup, or launch the
 settings app."
fi
