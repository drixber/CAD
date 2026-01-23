# Hydra CAD Installation Guide

## Voraussetzungen

### Windows
- Windows 10 oder höher (64-bit)
- Visual Studio 2019 oder höher (mit C++ Desktop Development Workload)
- CMake 3.15 oder höher
- vcpkg (für Dependency Management)
- NSIS 3.0 oder höher (für Installer-Erstellung)

### Optionale Abhängigkeiten
- Python 3.8+ (für Python Bindings)
- Qt 5.15+ oder Qt 6.x

## Build-Prozess

### 1. Repository klonen
```bash
git clone <repository-url>
cd CAD
```

### 2. vcpkg einrichten
```bash
# vcpkg installieren (falls noch nicht vorhanden)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Abhängigkeiten installieren
.\vcpkg install qt5-base qt5-tools --triplet x64-windows
```

### 3. Projekt konfigurieren
```bash
# Build-Verzeichnis erstellen
mkdir build
cd build

# CMake konfigurieren
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
```

### 4. Projekt kompilieren
```bash
# Kompilieren
cmake --build . --config Release

# Oder mit Visual Studio
cmake --build . --config Release --target cad_desktop
```

Die ausführbare Datei wird in `build/Release/cad_desktop.exe` erstellt.

## Installer erstellen

### 1. NSIS installieren
- Download von: https://nsis.sourceforge.io/Download
- Installation durchführen

### 2. Installer-Skript anpassen
Öffnen Sie `installer/hydracad.nsi` und passen Sie die Pfade an:

```nsis
; Pfade anpassen
File "..\build\Release\cad_desktop.exe"
File /nonfatal "..\build\Release\*.dll"
```

### 3. Installer kompilieren
```bash
# Im installer-Verzeichnis
cd installer

# NSIS ausführen (GUI)
# Oder über Kommandozeile:
"C:\Program Files (x86)\NSIS\makensis.exe" hydracad.nsi
```

Der Installer wird als `HydraCADSetup.exe` im `installer`-Verzeichnis erstellt.

## Installation

### Automatische Installation
1. Doppelklicken Sie auf `HydraCADSetup.exe`
2. Folgen Sie dem Installationsassistenten
3. Wählen Sie die gewünschten Komponenten:
   - **Core Application** (immer erforderlich)
   - **Python Bindings** (optional)
   - **Example Files** (optional)
4. Wählen Sie das Installationsverzeichnis (Standard: `C:\Program Files\Hydra CAD`)
5. Klicken Sie auf "Install"

### Manuelle Installation
Falls Sie keinen Installer verwenden möchten:

1. Kopieren Sie `build/Release/cad_desktop.exe` in ein Verzeichnis Ihrer Wahl
2. Kopieren Sie alle benötigten DLLs (Qt, etc.) in dasselbe Verzeichnis
3. Erstellen Sie eine Verknüpfung auf dem Desktop

## Erste Schritte

### Starten der Anwendung
- Doppelklicken Sie auf das Desktop-Symbol
- Oder starten Sie `cad_desktop.exe` aus dem Installationsverzeichnis

### Updates
Die Anwendung prüft automatisch auf Updates:
- Automatische Prüfung: Alle 7 Tage (konfigurierbar)
- Manuelle Prüfung: Über das Menü "Help" > "Check for Updates"

### Deinstallation
1. Öffnen Sie "Systemsteuerung" > "Programme und Funktionen"
2. Wählen Sie "Hydra CAD"
3. Klicken Sie auf "Deinstallieren"
4. Oder verwenden Sie `Uninstall.exe` im Installationsverzeichnis

## Troubleshooting

### Problem: "DLL nicht gefunden"
**Lösung:** Stellen Sie sicher, dass alle Qt-DLLs im gleichen Verzeichnis wie die .exe sind.

### Problem: Installer erstellt keine Datei
**Lösung:** 
- Prüfen Sie, ob der Build erfolgreich war
- Prüfen Sie die Pfade im NSIS-Skript
- Führen Sie NSIS als Administrator aus

### Problem: Anwendung startet nicht
**Lösung:**
- Prüfen Sie die Event Viewer auf Fehler
- Stellen Sie sicher, dass Visual C++ Redistributables installiert sind
- Prüfen Sie die Log-Dateien im Installationsverzeichnis

## Build-Optionen

### Python Bindings aktivieren
```bash
cmake .. -DCAD_BUILD_PYTHON=ON
```

### Debug-Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

### Spezifische Komponenten
```bash
# Nur bestimmte Module bauen
cmake --build . --target cad_desktop
```

## Weitere Informationen

- **Vollständige Dokumentation**: [PROJECT_FINAL.md](PROJECT_FINAL.md)
- **Release Notes**: [FINAL_RELEASE_NOTES.md](FINAL_RELEASE_NOTES.md)
- **Dokumentations-Übersicht**: [README.md](README.md)

