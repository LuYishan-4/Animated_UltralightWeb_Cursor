pkgname=ultralightwebcursor-git
_pkgname=Animated_UltralightWeb_Cursor
pkgver=1.1.0.r121.g8dc45f0
pkgrel=1
pkgdesc="Programmable HTML/CSS/JS animated cursor for Linux X11 and XWayland"
arch=('x86_64')
url="https://github.com/LuYishan-4/Animated_UltralightWeb_Cursor"
license=('MIT' 'LicenseRef-Ultralight-Free-SDK')
depends=(
    'at-spi2-core'
    'bzip2'
    'cairo'
    'fontconfig'
    'gcc-libs'
    'gdk-pixbuf2'
    'glib2'
    'glibc'
    'gtk3'
    'harfbuzz'
    'hicolor-icon-theme'
    'libglvnd'
    'libx11'
    'libxfixes'
    'pango'
    'qt6-base'
    'qt6-declarative'
    'xcb-util-cursor'
)
makedepends=('cmake' 'git' '7zip')
checkdepends=('appstream' 'desktop-file-utils')
provides=("ultralightwebcursor=${pkgver}")
conflicts=('ultralightwebcursor')
options=('!strip')
source=("git+${url}.git")
sha256sums=('SKIP')

pkgver() {
    cd "${srcdir}/${_pkgname}"
    local base_version commit_count commit_hash
    base_version=$(sed -nE 's/.*project\(UltralightWebCursor VERSION ([0-9.]+).*/\1/p' CMakeLists.txt)
    commit_count=$(git rev-list --count HEAD)
    commit_hash=$(git rev-parse --short=7 HEAD)
    printf '%s.r%s.g%s' "${base_version}" "${commit_count}" "${commit_hash}"
}

prepare() {
    cd "${srcdir}/${_pkgname}"

    local sdk_archive="sdk/ultralight-free-sdk-1.4.0-linux-x64.7z"
    local sdk_target="sdk/ultralight-free-sdk-1.4.0-linux-x64"

    rm -rf "${sdk_target}"
    mkdir -p "${sdk_target}"
    7z x "${sdk_archive}" "-o${sdk_target}" -y >/dev/null

    local required=(
        'include/AppCore/App.h'
        'bin/libAppCore.so'
        'bin/libUltralight.so'
        'bin/libUltralightCore.so'
        'bin/libWebCore.so'
        'resources/cacert.pem'
        'resources/icudt67l.dat'
        'license/LICENSE.txt'
        'license/EULA.txt'
        'license/NOTICES.md'
    )
    local file
    for file in "${required[@]}"; do
        if [[ ! -f "${sdk_target}/${file}" ]]; then
            error "Ultralight SDK is incomplete: missing ${file}"
            return 1
        fi
    done
}

build() {
    cd "${srcdir}/${_pkgname}"
    cmake -S . -B build-linux \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DCMAKE_SKIP_RPATH=OFF \
        -Wno-dev
    cmake --build build-linux --parallel
}

check() {
    cd "${srcdir}/${_pkgname}"
    desktop-file-validate GUI/io.github.luyishan4.ultralightwebcursor.desktop
    appstreamcli validate --no-net GUI/io.github.luyishan4.ultralightwebcursor.metainfo.xml
}

package() {
    cd "${srcdir}/${_pkgname}"
    DESTDIR="${pkgdir}" cmake --install build-linux
}
