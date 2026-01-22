# Schnellstart - Terminal-Befehle

## Batch-Datei aus Terminal starten

### PowerShell:
```powershell
.\build_installer.bat
```

### CMD (Eingabeaufforderung):
```cmd
build_installer.bat
```

### Mit vollem Pfad:
```powershell
cd "C:\Users\Darius Herrmann\cad\CAD"
.\build_installer.bat
```

## PowerShell-Script starten

### Direkt:
```powershell
.\build_installer.ps1
```

### Falls Ausführungsrichtlinie blockiert:
```powershell
powershell -ExecutionPolicy Bypass -File .\build_installer.ps1
```

## Terminal öffnen

### PowerShell im Projektverzeichnis öffnen:
1. Im Windows Explorer zum Projektverzeichnis navigieren
2. In der Adressleiste `powershell` eingeben und Enter drücken
3. Oder Rechtsklick im Ordner > "PowerShell hier öffnen"

### CMD im Projektverzeichnis öffnen:
1. Im Windows Explorer zum Projektverzeichnis navigieren
2. In der Adressleiste `cmd` eingeben und Enter drücken
3. Oder Shift + Rechtsklick > "Eingabeaufforderung hier öffnen"

## Nach dem Build

Nach erfolgreichem Build finden Sie den Installer hier:
```
installer\CADursorSetup.exe
```

Dann einfach doppelklicken zum Installieren!

