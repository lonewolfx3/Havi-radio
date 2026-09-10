Unicode True
!include "MUI2.nsh"
!include "x64.nsh"
!include "WinVer.nsh"
!ifndef PLUGIN_DIR
  !error "Pass /DPLUGIN_DIR with the compiled VST3 bundle path"
!endif
!ifndef OUTPUT_FILE
  !error "Pass /DOUTPUT_FILE with the installer output path"
!endif
Name "Havi Radio / Radio Box"
OutFile "${OUTPUT_FILE}"
RequestExecutionLevel admin
InstallDir "$PROGRAMFILES64\Havi Radio"
SetCompressor /SOLID lzma
VIProductVersion "0.2.0.0"
VIAddVersionKey "ProductName" "Havi Radio"
VIAddVersionKey "FileDescription" "Radio Box VST3 Installer"
VIAddVersionKey "FileVersion" "0.2.0"
!define MUI_WELCOMEPAGE_TEXT "This installs Havi Radio for 64-bit Windows 10/11.$\r$\n$\r$\nClose FL Studio before continuing. After installation, scan for plugins in FL Studio's Plugin Manager."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "64-bit Windows 10 or 11 is required."
    Abort
  ${EndIf}
  ${IfNot} ${AtLeastWin10}
    MessageBox MB_ICONSTOP "Windows 10 or later is required."
    Abort
  ${EndIf}
  SetRegView 64
FunctionEnd
Section "Havi Radio VST3"
  SetOutPath "$COMMONFILES64\VST3\Havi Radio.vst3"
  File /r "${PLUGIN_DIR}\*"
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HaviRadio" "DisplayName" "Havi Radio"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HaviRadio" "DisplayVersion" "0.2.0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HaviRadio" "UninstallString" '$"$INSTDIR\Uninstall.exe$"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HaviRadio" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HaviRadio" "NoRepair" 1
SectionEnd
Section "Uninstall"
  SetRegView 64
  RMDir /r "$COMMONFILES64\VST3\Havi Radio.vst3"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HaviRadio"
SectionEnd
