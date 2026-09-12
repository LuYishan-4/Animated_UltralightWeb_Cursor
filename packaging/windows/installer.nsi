; Ultralight Web Cursor - Windows installer (NSIS).
;
; Packages the staged tree produced by `cmake --install` followed by
; `windeployqt`, so the resulting .exe is a complete, self-contained installer.
;
; Build it with:
;   makensis /DSTAGE_DIR=<stage-dir> /DOUT_FILE=<output.exe> installer.nsi

Unicode true

!ifndef STAGE_DIR
  !error "STAGE_DIR must be defined (staged install directory)"
!endif
!ifndef OUT_FILE
  !error "OUT_FILE must be defined (output installer path)"
!endif

!include "MUI2.nsh"

!define APP_NAME "Ultralight Web Cursor"
!define APP_EXE "bin\ultralightwebcursor-gui.exe"
!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\UltralightWebCursor"

; Version metadata (also fed to the PE header) makes the installer look like a
; normal, well-formed product rather than a generic packer output, which helps
; with SmartScreen/Safe Browsing reputation.
!ifndef APP_VERSION
  !define APP_VERSION "1.0.2"
!endif

VIProductVersion "${APP_VERSION}.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "FileDescription" "${APP_NAME} Setup"
VIAddVersionKey "FileVersion" "${APP_VERSION}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "LegalCopyright" "MIT License"

Name "${APP_NAME}"
OutFile "${OUT_FILE}"
InstallDir "$PROGRAMFILES64\UltralightWebCursor"
InstallDirRegKey HKLM "${UNINST_KEY}" "InstallLocation"
RequestExecutionLevel admin
ManifestDPIAware true
SetCompressor /SOLID lzma

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${APP_NAME} settings"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "TradChinese"

Section "Ultralight Web Cursor" SecMain
  SetOutPath "$INSTDIR"
  File /r "${STAGE_DIR}\*.*"

  WriteUninstaller "$INSTDIR\uninstall.exe"

  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"

  WriteRegStr HKLM "${UNINST_KEY}" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayIcon" "$INSTDIR\${APP_EXE}"
  WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair" 1
SectionEnd

Section "Uninstall"
  ; Stop a running instance and drop the HKCU autostart entry the engine wrote.
  ExecWait '"$SYSDIR\taskkill.exe" /IM ultralightwebcursor_windows.exe /F' $0
  ExecWait '"$SYSDIR\taskkill.exe" /IM ultralightwebcursor-gui.exe /F' $0
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "UltralightWebCursor"

  Delete "$DESKTOP\${APP_NAME}.lnk"
  RMDir /r "$SMPROGRAMS\${APP_NAME}"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "${UNINST_KEY}"
SectionEnd
