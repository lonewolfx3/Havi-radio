Unicode True
!include "MUI2.nsh"
!include "x64.nsh"
!include "WinVer.nsh"
!ifndef PLUGIN_DIR
  !error "Pass /DPLUGIN_DIR with the signed AAX bundle path"
!endif
!ifndef OUTPUT_FILE
  !error "Pass /DOUTPUT_FILE with the installer output path"
!endif
Name "RadioBox AAX by UOC SOUND"
OutFile "${OUTPUT_FILE}"
RequestExecutionLevel admin
InstallDir "$PROGRAMFILES64\UOC SOUND\RadioBox"
SetCompressor /SOLID lzma
VIProductVersion "0.4.0.0"
VIAddVersionKey "ProductName" "RadioBox AAX"
VIAddVersionKey "FileVersion" "0.4.0"
!define MUI_WELCOMEPAGE_TEXT "Installs the signed 64-bit RadioBox AAX Native plug-in for Pro Tools on Windows.$\r$\n$\r$\nClose Pro Tools before continuing."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "64-bit Windows is required."
    Abort
  ${EndIf}
  SetRegView 64
FunctionEnd
Section "RadioBox AAX Native"
  SetOutPath "$COMMONFILES64\Avid\Audio\Plug-Ins\RadioBox.aaxplugin"
  File /r "${PLUGIN_DIR}\*"
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall-AAX.exe"
SectionEnd
Section "Uninstall"
  RMDir /r "$COMMONFILES64\Avid\Audio\Plug-Ins\RadioBox.aaxplugin"
  Delete "$INSTDIR\Uninstall-AAX.exe"
  RMDir "$INSTDIR"
SectionEnd
