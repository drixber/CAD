# CMake Installation - Schnellhilfe

## Problem: "CMake nicht gefunden"

Wenn das Build-Script CMake nicht findet, haben Sie zwei Optionen:

## Option 1: CMake installieren (Empfohlen)

1. **Download:**
   - Gehen Sie zu: https://cmake.org/download/
   - Laden Sie "Windows x64 Installer" herunter

2. **Installation:**
   - Führen Sie den Installer aus
   - **WICHTIG:** Aktivieren Sie "Add CMake to system PATH"
   - Klicken Sie auf "Install"

3. **Terminal neu starten:**
   - Schließen Sie alle PowerShell/CMD Fenster
   - Öffnen Sie ein neues Terminal
   - Führen Sie `build_installer.bat` erneut aus

## Option 2: CMake zum PATH hinzufügen (falls bereits installiert)

### Automatisch (PowerShell als Admin):
```powershell
# Prüfe ob CMake installiert ist
$cmakePath = "C:\Program Files\CMake\bin"
if (Test-Path $cmakePath) {
    [Environment]::SetEnvironmentVariable("Path", $env:Path + ";$cmakePath", [EnvironmentVariableTarget]::Machine)
    Write-Host "CMake zum System-PATH hinzugefügt!" -ForegroundColor Green
    Write-Host "Bitte Terminal neu starten." -ForegroundColor Yellow
}
```

### Manuell:
1. Windows-Taste + Pause → "Erweiterte Systemeinstellungen"
2. "Umgebungsvariablen" klicken
3. Unter "Systemvariablen" → "Path" auswählen → "Bearbeiten"
4. "Neu" klicken
5. Einen dieser Pfade hinzufügen:
   - `C:\Program Files\CMake\bin`
   - `C:\Program Files (x86)\CMake\bin`
6. "OK" klicken
7. **Terminal neu starten**

## Prüfen ob CMake funktioniert

Nach der Installation/Änderung:
```powershell
cmake --version
```

Sollte die Version anzeigen, z.B.:
```
cmake version 3.26.0
```

## Alternative: CMake über vcpkg installieren

Falls Sie vcpkg verwenden:
```powershell
.\vcpkg\vcpkg install cmake
```

## Nach erfolgreicher CMake-Installation

Führen Sie erneut aus:
```powershell
.\build_installer.bat
```

