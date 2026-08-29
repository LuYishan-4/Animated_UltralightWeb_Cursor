pkgname=ultralightwebcursor-git
_pkgname=Animated_UltralightWeb_Cursor
pkgver=1.0.2
pkgrel=1
pkgdesc="HTML/CSS/JS-based global animated cursor (KWin plugin + X11 standalone + settings GUI)"
arch=('x86_64')
url="https://github.com/LuYishan-4/Animated_UltralightWeb_Cursor"
license=('MIT')
depends=(
    'qt6-base'
    'qt6-declarative'
    'kwin'
    'kcoreaddons'
    'kconfig'
    'libx11'
    'libxfixes'
    'libepoxy'
)
makedepends=('git' 'cmake' 'extra-cmake-modules' 'p7zip' 'pkgconf')
provides=("ultralightwebcursor")
conflicts=("ultralightwebcursor")

install=ultralightwebcursor.install
source=("git+https://github.com/LuYishan-4/Animated_UltralightWeb_Cursor.git")
sha256sums=('SKIP')

pkgver() {
    cd "${srcdir}/${_pkgname}"
    (set -o pipefail; git describe --long --tags --abbrev=7 2>/dev/null | sed 's/\([^-]*-\)g/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short=7 HEAD)")
}

prepare() {
    cd "${srcdir}/${_pkgname}"

    # Unpack the Ultralight SDK if it has not been extracted yet.
    local sdk_target="sdk/ultralight-free-sdk-1.4.0-linux-x64"

    if [ ! -f "${sdk_target}/include/AppCore/App.h" ]; then
        rm -rf "${sdk_target}"
        cd sdk
        7z x ultralight-free-sdk-1.4.0-linux-x64.7z
        cd ..

        if [ -d "sdk/include" ] && [ -f "sdk/include/AppCore/App.h" ]; then
            mkdir -p "${sdk_target}"
            mv sdk/bin sdk/include sdk/layers "${sdk_target}/" 2>/dev/null || true
        fi

        if [ -d "${sdk_target}/ultralight-free-sdk-1.4.0-linux-x64" ]; then
            mv "${sdk_target}/ultralight-free-sdk-1.4.0-linux-x64" sdk/tmp_sdk
            rm -rf "${sdk_target}"
            mv sdk/tmp_sdk "${sdk_target}"
        fi
    fi

    if [ ! -f "${sdk_target}/include/AppCore/App.h" ]; then
        echo "==> ERROR: Ultralight SDK extraction layout is invalid."
        exit 1
    fi
}

build() {
    cd "${srcdir}/${_pkgname}"

    # KWin variant: effect plugin + GUI + variant selector.
    cmake -B build-kde -S . \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DFORCE_DESKTOP_ENVIRONMENT=kde \
        -DBUILD_TESTING=OFF \
        -Wno-dev
    cmake --build build-kde

    cmake -B build-x11 -S . \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DFORCE_DESKTOP_ENVIRONMENT=x11 \
        -DBUILD_TESTING=OFF \
        -Wno-dev
    cmake --build build-x11
}

package() {
    cd "${srcdir}/${_pkgname}"

    DESTDIR="${pkgdir}" cmake --install build-kde
    DESTDIR="${pkgdir}" cmake --install build-x11
}
