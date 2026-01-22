; CADursor Installer Script
; NSIS Installer with Update Support

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

; Installer Information
Name "CADursor"
OutFile "CADursorSetup.exe"
InstallDir "$PROGRAMFILES64\CADursor"
InstallDirRegKey HKCU "Software\CADursor" "InstallPath"
RequestExecutionLevel admin

; Version Information
!define VERSION "1.0.0"
!define VERSION_MAJOR "1"
!define VERSION_MINOR "0"
!define VERSION_PATCH "0"
VIProductVersion "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.0"
VIAddVersionKey "ProductName" "CADursor"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "CompanyName" "CADursor"
VIAddVersionKey "FileDescription" "CADursor CAD Application"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Installer Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller Pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language
!insertmacro MUI_LANGUAGE "English"

; Installer Sections
Section "Core Application" SecCore
    SectionIn RO  ; Read-only, always installed
    
    SetOutPath "$INSTDIR"
    
    ; Main executable
    File "..\build\Release\cad_desktop.exe"
    
    ; DLLs and dependencies
    File /nonfatal "..\build\Release\*.dll"
    
    ; Configuration files
    SetOutPath "$INSTDIR\config"
    File /nonfatal "..\config\*.json"
    File /nonfatal "..\config\*.xml"
    
    ; Resources
    SetOutPath "$INSTDIR\resources"
    File /r /nonfatal "..\resources\*.*"
    
    ; Documentation
    SetOutPath "$INSTDIR\docs"
    File /nonfatal "..\docs\*.md"
    File /nonfatal "..\docs\*.pdf"
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; Registry entries
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CADursor" "DisplayName" "CADursor"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CADursor" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CADursor" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CADursor" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CADursor" "Publisher" "CADursor"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CADursor" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CADursor" "NoRepair" 1
    
    ; User registry
    WriteRegStr HKCU "Software\CADursor" "InstallPath" "$INSTDIR"
    WriteRegStr HKCU "Software\CADursor" "Version" "${VERSION}"
    
    ; Start Menu shortcuts
    CreateDirectory "$SMPROGRAMS\CADursor"
    CreateShortcut "$SMPROGRAMS\CADursor\CADursor.lnk" "$INSTDIR\cad_desktop.exe"
    CreateShortcut "$SMPROGRAMS\CADursor\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    
    ; Desktop shortcut
    CreateShortcut "$DESKTOP\CADursor.lnk" "$INSTDIR\cad_desktop.exe"
    
    ; File associations
    WriteRegStr HKCR ".cad" "" "CADursor.Document"
    WriteRegStr HKCR "CADursor.Document" "" "CADursor Document"
    WriteRegStr HKCR "CADursor.Document\DefaultIcon" "" "$INSTDIR\cad_desktop.exe,0"
    WriteRegStr HKCR "CADursor.Document\shell\open\command" "" '"$INSTDIR\cad_desktop.exe" "%1"'
SectionEnd

Section "Python Bindings" SecPython
    SetOutPath "$INSTDIR\python"
    File /r /nonfatal "..\build\Release\python\*.*"
    
    ; Python package installation
    ExecWait 'python -m pip install "$INSTDIR\python\cadursor" --quiet'
SectionEnd

Section "Example Files" SecExamples
    SetOutPath "$INSTDIR\examples"
    File /r /nonfatal "..\examples\*.*"
SectionEnd

; Uninstaller
Section "Uninstall"
    ; Remove files
    Delete "$INSTDIR\cad_desktop.exe"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir /r "$INSTDIR\config"
    RMDir /r "$INSTDIR\resources"
    RMDir /r "$INSTDIR\docs"
    RMDir /r "$INSTDIR\python"
    RMDir /r "$INSTDIR\examples"
    RMDir "$INSTDIR"
    
    ; Remove shortcuts
    Delete "$SMPROGRAMS\CADursor\CADursor.lnk"
    Delete "$SMPROGRAMS\CADursor\Uninstall.lnk"
    RMDir "$SMPROGRAMS\CADursor"
    Delete "$DESKTOP\CADursor.lnk"
    
    ; Remove registry entries
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CADursor"
    DeleteRegKey HKCU "Software\CADursor"
    
    ; Remove file associations
    DeleteRegKey HKCR ".cad"
    DeleteRegKey HKCR "CADursor.Document"
SectionEnd

; Section Descriptions
LangString DESC_SecCore ${LANG_ENGLISH} "Core CADursor application files (required)"
LangString DESC_SecPython ${LANG_ENGLISH} "Python bindings and API"
LangString DESC_SecExamples ${LANG_ENGLISH} "Example CAD files and tutorials"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPython} $(DESC_SecPython)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} $(DESC_SecExamples)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
