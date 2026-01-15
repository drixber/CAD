Name "CADursor"
OutFile "CADursorSetup.exe"
InstallDir "$PROGRAMFILES64\\CADursor"
RequestExecutionLevel admin

Page directory
Page instfiles

Section "Install"
  SetOutPath "$INSTDIR"
  File "..\\build\\Release\\cad_desktop.exe"
  CreateShortCut "$DESKTOP\\CADursor.lnk" "$INSTDIR\\cad_desktop.exe"
SectionEnd
