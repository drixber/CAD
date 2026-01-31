# Hydra CAD Installation Guide

## Voraussetzungen

### Windows (empfohlen)
- Windows 10/11 (64-bit)
- Visual Studio 2019 oder 2022 (mit C++ Desktop Development Workload)
- CMake 3.26 oder höher
- **Qt 6.x** (z. B. 6.5.3, win64_msvc2019_64)
- NSIS 3.0+ (optional, nur für Installer-Erstellung)

### Optionale Abhängigkeiten
- Python 3.11+ (für Python-Bindings, `CAD_BUILD_PYTHON=ON`)
- vcpkg (optional, für weitere Bibliotheken)

**Hinweis:** Die Anwendung baut mit **Qt 6**; Qt 5 wird nicht mehr unterstützt.

## Build-Prozess (Windows)

### Option A: Automatischer Build inkl. Installer (empfohlen)

```powershell
.\build_installer.ps1
```

Oder per Batch:

```cmd
.\build_installer.bat
```

### Option B: Manueller Build

#### 1. Repository klonen

```bash
git clone https://github.com/drixber/CAD.git
cd CAD
```

#### 2. Qt 6 installieren

- Download von https://www.qt.io/download-qt-installer
- Oder über den Installer die Komponente **Qt 6.x für MSVC 2019 64-bit** auswählen
- Notieren Sie den Pfad (z. B. `C:\Qt\6.5.3\msvc2019_64`)

#### 3. CMake konfigurieren

```powershell
# Build-Verzeichnis anlegen
mkdir build
cd build

# Mit Qt 6 (Pfad anpassen)
$QtDir = "C:\Qt\6.5.3\msvc2019_64"
cmake -S .. -B . -G "Visual Studio 17 2022" -A x64 `
  -DCAD_USE_QT=ON `
  -DCMAKE_BUILD_TYPE=Release `
  -DQt6_DIR="$QtDir" `
  -DCMAKE_PREFIX_PATH="$QtDir"
```

#### 4. Kompilieren

```powershell
cmake --build . --config Release --parallel
```

Die ausführbare Datei liegt in `build\Release\cad_desktop.exe`.

#### 5. Qt-Deployment (für portable Nutzung)

```powershell
# Aus dem build-Verzeichnis, Qt bin-Pfad anpassen
& "C:\Qt\6.5.3\msvc2019_64\bin\windeployqt.exe" Release\cad_desktop.exe
```

Anschließend alle Dateien aus `build\Release\` (inkl. `platforms\`, Qt-DLLs etc.) zusammen mit der EXE verwenden oder als ZIP packen.

## Installer erstellen

### NSIS installieren

- Download: https://nsis.sourceforge.io/Download
- Standardpfad: `C:\Program Files (x86)\NSIS\makensis.exe`

### Installer bauen

Nach erfolgreichem Build und `windeployqt`:

```powershell
# Im Projektroot
& "C:\Program Files (x86)\NSIS\makensis.exe" /DINSTALLER_VERSION="2.0.0" /DPROJECT_ROOT="$PWD" installer\hydracad.nsi
```

Der Installer wird als `installer\HydraCADSetup.exe` erstellt.

## Installation (Endanwender)

### Automatische Installation (Windows)

1. **Download** von [GitHub Releases](https://github.com/drixber/CAD/releases)
2. **Installer:** `HydraCADSetup.exe` ausführen, Assistent folgen (Datenschutz, Pfad, Sprache)
3. **Portable:** `app-windows.zip` entpacken, `cad_desktop.exe` starten

### Erste Schritte

- Beim ersten Start: Account registrieren / einloggen
- Projekte: **File → Save Project** (Strg+S) / **Open Project** (Strg+O)
- Updates: **Settings → Check for Updates...**

## Build-Optionen (CMake)

| Option | Beschreibung | Default |
|--------|--------------|---------|
| `CAD_USE_QT` | Qt-UI aktivieren | OFF |
| `CAD_BUILD_TESTS` | Tests bauen | OFF |
| `CAD_BUILD_PYTHON` | Python-Bindings | OFF |
| `CAD_USE_FREECAD` | FreeCAD-Integration | OFF |
| `APP_VERSION` | Versionsstring (z. B. v1.0.0) | - |

Beispiel mit Tests:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCAD_USE_QT=ON -DCAD_BUILD_TESTS=ON -DQt6_DIR="$QtDir" -DCMAKE_PREFIX_PATH="$QtDir"
cmake --build build --config Release
ctest --test-dir build -C Release
```

## Linux / macOS

Aktuell wird der automatische Release-Build nur für Windows ausgeführt. Für Linux/macOS aus dem Quellcode bauen:

- **Linux (allgemein):** Qt 6 installieren (Paketmanager oder Qt Online Installer), dann CMake mit `-DCAD_USE_QT=ON` und passendem `Qt6_DIR`.
- **Arch Linux:** Ausführliche Anleitung inkl. Pacman-Abhängigkeiten, Build und Entwicklung: [docs/BUILD_ARCH.md](BUILD_ARCH.md). PKGBUILD-Vorlage für AUR/lokales Paket: `packaging/arch/PKGBUILD`.
- **macOS:** Qt 6 über den Qt Installer, dann CMake; optional `-DCAD_USE_QT=ON` und MACOSX_BUNDLE wird gesetzt.

## Troubleshooting

### "DLL nicht gefunden" (Windows)

- Nach dem Build `windeployqt` auf `cad_desktop.exe` ausführen
- Sicherstellen, dass `Qt6Core.dll`, `Qt6Gui.dll`, `platforms\qwindows.dll` neben der EXE liegen

### Anwendung startet nicht

- Visual C++ Redistributable (z. B. VC++ 2015–2022) installieren
- Logs prüfen: **Settings → Diagnostics → Open Logs Folder** bzw. **Show Startup Log**

### CMake findet Qt 6 nicht

- `Qt6_DIR` auf das Qt-Verzeichnis setzen, das die Datei `Qt6Config.cmake` enthält (z. B. `C:\Qt\6.5.3\msvc2019_64\lib\cmake\Qt6`)

## Weitere Informationen

- **Projekt-README:** [../README.md](../README.md)
- **Changelog:** [../CHANGELOG.md](../CHANGELOG.md)
