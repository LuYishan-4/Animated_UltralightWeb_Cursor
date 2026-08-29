@echo off
setlocal EnableExtensions

rem ---------------------------------------------------------------------------
rem  Ultralight Web Cursor - Windows setup
rem
rem  Copies the built Windows variant (engine + settings GUI + Ultralight
rem  runtime), the cursor themes and the Ultralight resources into the
rem  per-user locations the engine expects, then registers autostart.
rem
rem  Prerequisite: build the Windows variant first, e.g.
rem      cmake -B build-windows -S . -DFORCE_DESKTOP_ENVIRONMENT=windows
rem      cmake --build build-windows --config Release
rem ---------------------------------------------------------------------------

set "INSTALL_DIR=%LOCALAPPDATA%\Programs\UltralightWebCursor"

set "ROOT=%~dp0.."
set "BIN_DIR=%ROOT%\build-windows\bin"
set "THEMES_DIR=%ROOT%\WebCursor"

if not exist "%BIN_DIR%\ultralightwebcursor_windows.exe" (
    echo [ERROR] Build output not found:
    echo         %BIN_DIR%
    echo.
    echo Build the Windows variant first, for example:
    echo     cmake -B build-windows -S . -DFORCE_DESKTOP_ENVIRONMENT=windows
    echo     cmake --build build-windows --config Release
    exit /b 1
)

echo === Ultralight Web Cursor - Windows setup ===
echo.

echo [1/4] Installing binaries to:
echo       %INSTALL_DIR%
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
copy /Y "%BIN_DIR%\ultralightwebcursor_windows.exe" "%INSTALL_DIR%\" >nul
copy /Y "%BIN_DIR%\ultralightwebcursor-gui.exe"    "%INSTALL_DIR%\" >nul
copy /Y "%BIN_DIR%\*.dll"                          "%INSTALL_DIR%\" >nul

echo [2/4] Installing data (resources + themes) next to the engine:
echo       %INSTALL_DIR%
if exist "%BIN_DIR%\resources" (
    xcopy /E /I /Y "%BIN_DIR%\resources" "%INSTALL_DIR%\resources" >nul
)
if exist "%THEMES_DIR%" (
    xcopy /E /I /Y "%THEMES_DIR%\*" "%INSTALL_DIR%\" >nul
)

echo [3/4] Registering autostart
reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v UltralightWebCursor /t REG_SZ /d "\"%INSTALL_DIR%\ultralightwebcursor_windows.exe\" --silent" /f >nul

echo [4/4] Creating Start Menu shortcut
powershell -NoProfile -Command "$ws = New-Object -ComObject WScript.Shell; $lnk = $ws.CreateShortcut([Environment]::GetFolderPath('StartMenu') + '\Programs\Ultralight Web Cursor.lnk'); $lnk.TargetPath = '%INSTALL_DIR%\ultralightwebcursor-gui.exe'; $lnk.WorkingDirectory = '%INSTALL_DIR%'; $lnk.Save()" >nul 2>&1

echo.
echo Setup complete. Start the cursor engine now? [Y/n]
set /p START_NOW=
if /i "%START_NOW%"=="n"  exit /b 0
if /i "%START_NOW%"=="no" exit /b 0

start "" "%INSTALL_DIR%\ultralightwebcursor_windows.exe" --silent

echo.
echo Done. You can open settings from the Start Menu shortcut.
exit /b 0
