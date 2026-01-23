; Hydra CAD Installer Script
; NSIS Installer with Update Support

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

; Project root (can be overridden via /DPROJECT_ROOT)
!ifndef PROJECT_ROOT
  !define PROJECT_ROOT ".."
!endif

; Installer Information
Name "Hydra CAD"
OutFile "HydraCADSetup.exe"
InstallDir "$PROGRAMFILES64\Hydra CAD"
InstallDirRegKey HKCU "Software\HydraCAD" "InstallPath"
RequestExecutionLevel admin

; Version Information
!define VERSION "2.0.0"
!define VERSION_MAJOR "2"
!define VERSION_MINOR "0"
!define VERSION_PATCH "0"
VIProductVersion "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.0"
VIAddVersionKey "ProductName" "Hydra CAD"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "CompanyName" "Hydra CAD"
VIAddVersionKey "FileDescription" "Hydra CAD Application"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Application Icon (will use executable icon if available)
!define APP_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"

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
    
    ; Main executable (with icon embedded if available)
    File "${PROJECT_ROOT}\build\Release\cad_desktop.exe"
    
    ; Set application icon for shortcuts
    !ifdef APP_ICON
        !if "${APP_ICON}" != ""
            ; Icon will be extracted from executable if embedded
        !endif
    !endif
    
    ; DLLs and dependencies (Qt, etc.)
    File /nonfatal "${PROJECT_ROOT}\build\Release\*.dll"
    
    ; Create data directory for user files
    CreateDirectory "$INSTDIR\data"
    
    ; Configuration files (if exist)
    SetOutPath "$INSTDIR\config"
    File /nonfatal "${PROJECT_ROOT}\config\*.json"
    File /nonfatal "${PROJECT_ROOT}\config\*.xml"
    
    ; Resources (if exist)
    SetOutPath "$INSTDIR\resources"
    File /r /nonfatal "${PROJECT_ROOT}\resources\*.*"
    
    ; Documentation (essential docs only)
    SetOutPath "$INSTDIR\docs"
    File /nonfatal "${PROJECT_ROOT}\docs\PROJECT_FINAL.md"
    File /nonfatal "${PROJECT_ROOT}\docs\INSTALLATION.md"
    File /nonfatal "${PROJECT_ROOT}\README.md"
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; Registry entries
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HydraCAD" "DisplayName" "Hydra CAD"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HydraCAD" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HydraCAD" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HydraCAD" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HydraCAD" "Publisher" "Hydra CAD"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HydraCAD" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HydraCAD" "NoRepair" 1
    
    ; User registry
    WriteRegStr HKCU "Software\HydraCAD" "InstallPath" "$INSTDIR"
    WriteRegStr HKCU "Software\HydraCAD" "Version" "${VERSION}"
    
    ; Start Menu shortcuts
    CreateDirectory "$SMPROGRAMS\Hydra CAD"
    ; Note: CreateShortcut for Hydra CAD.lnk is done later with icon
    CreateShortcut "$SMPROGRAMS\Hydra CAD\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    
    ; Desktop shortcut with icon
    CreateShortcut "$DESKTOP\Hydra CAD.lnk" "$INSTDIR\cad_desktop.exe" "" "$INSTDIR\cad_desktop.exe" 0
    
    ; Start Menu shortcuts with icons
    CreateShortcut "$SMPROGRAMS\Hydra CAD\Hydra CAD.lnk" "$INSTDIR\cad_desktop.exe" "" "$INSTDIR\cad_desktop.exe" 0
    
    ; File associations
    WriteRegStr HKCR ".cad" "" "HydraCAD.Document"
    WriteRegStr HKCR "HydraCAD.Document" "" "Hydra CAD Document"
    WriteRegStr HKCR "HydraCAD.Document\DefaultIcon" "" "$INSTDIR\cad_desktop.exe,0"
    WriteRegStr HKCR "HydraCAD.Document\shell\open\command" "" '"$INSTDIR\cad_desktop.exe" "%1"'
    
    ; Refresh shell to show file associations
    System::Call 'shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
SectionEnd

Section "Python Bindings" SecPython
    SetOutPath "$INSTDIR\python"
    File /r /nonfatal "${PROJECT_ROOT}\build\Release\python\*.*"
    
    ; Python package installation
    ExecWait 'python -m pip install "$INSTDIR\python\cadursor" --quiet'
SectionEnd

Section "Example Files" SecExamples
    SetOutPath "$INSTDIR\examples"
    File /r /nonfatal "${PROJECT_ROOT}\examples\*.*"
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
    ; Note: Don't remove $INSTDIR\data (user data should be preserved)
    RMDir "$INSTDIR"
    
    ; Remove shortcuts
    Delete "$SMPROGRAMS\Hydra CAD\Hydra CAD.lnk"
    Delete "$SMPROGRAMS\Hydra CAD\Uninstall.lnk"
    RMDir "$SMPROGRAMS\Hydra CAD"
    Delete "$DESKTOP\Hydra CAD.lnk"
    
    ; Remove registry entries
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HydraCAD"
    DeleteRegKey HKCU "Software\HydraCAD"
    
    ; Remove file associations
    DeleteRegKey HKCR ".cad"
    DeleteRegKey HKCR "HydraCAD.Document"
SectionEnd

; Section Descriptions
LangString DESC_SecCore ${LANG_ENGLISH} "Core Hydra CAD application files (required)"
LangString DESC_SecPython ${LANG_ENGLISH} "Python bindings and API"
LangString DESC_SecExamples ${LANG_ENGLISH} "Example CAD files and tutorials"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPython} $(DESC_SecPython)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} $(DESC_SecExamples)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
