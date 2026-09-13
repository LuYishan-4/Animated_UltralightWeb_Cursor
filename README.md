<div align="center">


# Ultralight Web Cursor

**Programmable animated desktop cursors made with HTML, CSS and JavaScript.**

[![AUR](https://img.shields.io/badge/AUR-ultralightwebcursor--git-1793d1?logo=arch-linux&logoColor=white)](https://aur.archlinux.org/packages/ultralightwebcursor-git)
[![Platforms](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-4fc3f7)](#platform-support)
[![C++](https://img.shields.io/badge/C%2B%2B-20-00599c?logo=cplusplus)](#)


**English** · [繁體中文](#繁體中文)

</div>

---

## English

Ultralight Web Cursor renders a transparent, click-through desktop overlay and
places an Ultralight HTML view at the pointer position. Themes can react to
movement and mouse clicks without requiring a browser or Electron.

### Features

- HTML/CSS/JavaScript cursor themes
- Transparent, click-through desktop overlay
- Bundled theme gallery and trusted-folder import
- Live theme, size and enable/disable updates
- Per-user launch-on-login setting
- Multi-monitor virtual desktop support
- Standalone Qt Quick settings application
- Single Windows setup executable produced by CI

### Platform support

| Platform | Status | Notes |
|---|---|---|
| Linux X11 | Supported | Uses Qt xcb, Xlib and XFixes |
| Linux Wayland | XWayland compatibility | Requires XWayland; behavior depends on the compositor |
| Windows 10/11 x64 | Supported | Installed with the CI-generated setup executable |
| Native Wayland | Not supported | Standard Wayland does not expose the global pointer APIs this overlay needs |

### Install on Arch Linux

```bash
yay -S ultralightwebcursor-git
```

Open **Ultralight Web Cursor** from the application launcher. The GUI starts the
engine when the cursor is enabled. To remove the AUR package:

```bash
yay -Rns ultralightwebcursor-git
```

Linux packages are intentionally never removed from inside the GUI because
package-owned files must be managed by pacman.

### Manual Linux build

Requirements include CMake, Qt 6, X11/XFixes, 7-Zip and the dependencies of the
bundled Ultralight runtime.

```bash
git clone https://github.com/LuYishan-4/Animated_UltralightWeb_Cursor.git
cd Animated_UltralightWeb_Cursor
./build.sh                 # build only
./build.sh install         # install to /usr/local (sudo)
```

Use `PREFIX=/usr ./build.sh install` to install under `/usr`. Build-tree binaries
are placed in `build-linux/bin/` and include a staged data directory for direct
testing.

### Windows

Every supported push builds `ultralightwebcursor-windows-x64-setup.exe` in
GitHub Actions. The installer contains the engine, settings application, Qt
runtime, Ultralight runtime, themes, licenses and an uninstaller.

Unsigned executables can trigger SmartScreen or Safe Browsing. Release builds
can be Authenticode-signed by setting the repository secrets
`WINDOWS_PFX_BASE64` and `WINDOWS_PFX_PASSWORD`. A SHA-256 checksum is published
next to every installer.

### Data and configuration

| Data | Linux | Windows |
|---|---|---|
| Settings | `~/.config/ultralightwebcursor/config.ini` | `%LOCALAPPDATA%/ultralightwebcursor/config.ini` |
| Writable themes/resources | `~/.local/share/ultralightwebcursor/` | `%LOCALAPPDATA%/ultralightwebcursor/` |
| Bundled Linux data | `/usr/share/ultralightwebcursor/` | Installed beside the executables |

Bundled resources are synchronized into the writable per-user data directory.
Imported themes are never written into `/usr` or `Program Files`.

### Theme format

A theme is a folder containing at least:

```text
my-theme/
├── CursorData.json
└── index.html
```

CSS, JavaScript, images and audio can live beside those files. See
`WebCursor/variant1-neon/` for a minimal example.

> [!WARNING]
> Themes execute JavaScript. Import themes only from authors you trust.

### Development

```bash
./build.sh build
ULTRALIGHTWEBCURSOR_DEBUG=1 ./build-linux/bin/ultralightwebcursor-engine
./build-linux/bin/ultralightwebcursor-gui
```

The source tree is intentionally split into four focused areas:

```text
src/config/      configuration, paths, migration and theme metadata
src/engine/      overlay, IPC, autostart and platform mouse providers
src/renderer/    Ultralight HTML renderer and JavaScript bridge
GUI/             standalone Qt Quick settings application
```

### License

Project source code is MIT licensed. The bundled Ultralight SDK/runtime is
proprietary and distributed under Ultralight's license and EULA. Both license
sets are installed with the application; see `sdk/.../license/` after extracting
the SDK archive.

---

## 繁體中文

Ultralight Web Cursor 使用透明、可穿透點擊的桌面覆蓋視窗，在滑鼠位置繪製
Ultralight HTML 畫面。游標主題可以使用 HTML、CSS 與 JavaScript，並回應移動與
點擊事件，不需要瀏覽器或 Electron。

### 功能

- HTML/CSS/JavaScript 動畫游標主題
- 透明、可穿透點擊的桌面覆蓋視窗
- 內建主題與可信任資料夾匯入
- 即時切換主題、尺寸與啟用狀態
- 使用者層級的登入自動啟動
- 多螢幕虛擬桌面支援
- 獨立 Qt Quick 設定程式
- Windows CI 產生單一安裝用 `.exe`

### 平台

- **Linux X11：**正式支援。
- **Linux Wayland：**透過 XWayland 相容執行，效果取決於 compositor。
- **Windows 10/11 x64：**正式支援。
- **原生 Wayland：**目前不支援；Linux 版本需要 X11 或 XWayland。

### Arch Linux 安裝

```bash
yay -S ultralightwebcursor-git
```

安裝後從應用程式選單搜尋 **Ultralight Web Cursor**。移除套件請使用：

```bash
yay -Rns ultralightwebcursor-git
```

GUI 不會自行刪除 pacman 管理的檔案，避免破壞套件資料庫。

### 手動編譯

```bash
git clone https://github.com/LuYishan-4/Animated_UltralightWeb_Cursor.git
cd Animated_UltralightWeb_Cursor
./build.sh                 # 只編譯
./build.sh install         # 安裝到 /usr/local
```

開發測試：

```bash
ULTRALIGHTWEBCURSOR_DEBUG=1 ./build-linux/bin/ultralightwebcursor-engine
./build-linux/bin/ultralightwebcursor-gui
```

### 資料位置

Linux 設定檔位於 `~/.config/ultralightwebcursor/config.ini`，可寫入的資源與
主題位於 `~/.local/share/ultralightwebcursor/`。套件內建資料放在
`/usr/share/ultralightwebcursor/`，程式會同步到使用者資料夾，不會嘗試寫入
`/usr`。

> [!WARNING]
> 游標主題可以執行 JavaScript，請只匯入你信任的主題。

### 授權

專案原始碼採 MIT 授權；內附的 Ultralight SDK/runtime 為專有授權，安裝時會一併
放入 Ultralight 的 LICENSE、EULA 與 NOTICES。
