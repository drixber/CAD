# CADursor Installer - Schnellstart

## Einfache Installation

### Option 1: Automatisches Build-Script (Empfohlen)

**Windows Batch-Datei:**
1. Doppelklicken Sie auf `build_installer.bat`
2. Das Script baut automatisch das Projekt und erstellt den Installer
3. Nach Abschluss finden Sie `installer\CADursorSetup.exe`
4. Doppelklicken Sie auf `CADursorSetup.exe` zum Installieren

**PowerShell-Script:**
1. Rechtsklick auf `build_installer.ps1` > "Mit PowerShell ausführen"
2. Oder in PowerShell: `.\build_installer.ps1`
3. Nach Abschluss finden Sie `installer\CADursorSetup.exe`
4. Doppelklicken Sie auf `CADursorSetup.exe` zum Installieren

### Option 2: Manuell

1. **Projekt kompilieren:**
   ```powershell
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCAD_USE_QT=ON
   cmake --build build --config Release
   ```

2. **Installer erstellen:**
   ```powershell
   cd installer
   "C:\Program Files (x86)\NSIS\makensis.exe" cadursor.nsi
   cd ..
   ```

3. **Installieren:**
   - Doppelklick auf `installer\CADursorSetup.exe`

## Was wird installiert?

- **Core Application**: Hauptanwendung (immer installiert)
- **Python Bindings**: Python API (optional)
- **Example Files**: Beispiel-Dateien (optional)

## Installationsverzeichnis

Standard: `C:\Program Files\CADursor`

## Nach der Installation

- Desktop-Verknüpfung wird automatisch erstellt
- Startmenü-Eintrag wird erstellt
- Dateiassoziation für `.cad` Dateien wird erstellt

## Deinstallation

1. Systemsteuerung > Programme und Funktionen
2. "CADursor" auswählen
3. "Deinstallieren" klicken

Oder: `C:\Program Files\CADursor\Uninstall.exe` ausführen

## Voraussetzungen

- Windows 10/11 (64-bit)
- Visual Studio 2019+ (für Build)
- CMake 3.26+ (für Build)
- NSIS 3.0+ (für Installer-Erstellung)

## Hilfe

Bei Problemen siehe: `docs\INSTALLATION.md`

