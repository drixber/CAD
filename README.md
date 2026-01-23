# Hydra CAD 2.0.0 - Professional CAD Application

## Overview

Hydra CAD is a comprehensive Computer-Aided Design (CAD) application built with C++ and Qt, featuring advanced 3D modeling, simulation, and drawing generation capabilities.

**Status**: ✅ **Produktionsbereit** - Alle Kern-Features implementiert

## 🚀 Schnellstart

### Installation (Empfohlen - Fertiger Installer)

**Alle Plattformen - Einfachste Methode:**

1. **Download**: Gehen Sie zu [GitHub Releases](https://github.com/drixber/CAD/releases)
2. **Download**: Laden Sie den Installer für Ihre Plattform herunter:
   - **Windows**: `HydraCADSetup.exe`
   - **Linux**: `HydraCAD-*-x86_64.AppImage`
   - **macOS**: `HydraCAD-*-macOS.dmg`

**Windows:**
- Doppelklick auf `HydraCADSetup.exe`
- Installationsassistenten folgen
- Desktop-Shortcut wird automatisch erstellt

**Linux:**
```bash
chmod +x HydraCAD-*-x86_64.AppImage
./HydraCAD-*-x86_64.AppImage
```

**macOS:**
- DMG öffnen
- App in Applications-Ordner ziehen
- App aus Applications starten

**Erste Schritte:**
- Beim ersten Start: Account registrieren
- Nach Login: App ist bereit für die Verwendung
- Projekte speichern: File → Save Project (Strg+S / Cmd+S)
- Projekte öffnen: File → Open Project (Strg+O / Cmd+O)

### Eigenen Build erstellen (Optional)

**Windows:**
```powershell
.\build_installer.ps1
```

**Linux/macOS:**
Siehe [docs/INSTALLATION.md](docs/INSTALLATION.md) für detaillierte Anleitung.

## Features

### Kern-Features
- ✅ **3D Viewport Rendering**: Coin3D/SoQt Integration mit Echtzeit-3D-Visualisierung
- ✅ **FreeCAD Integration**: Vollständige Synchronisation von Sketches, Parts und Drawings
- ✅ **Import/Export**: Unterstützung für STEP, IGES, STL, DWG, DXF, OBJ, PLY, 3MF, GLTF
- ✅ **Constraint Solver**: Professioneller Newton-Raphson Solver
- ✅ **Simulation**: FEA, Motion, Deflection, Optimization, Thermal Analysis
- ✅ **Sheet Metal**: Flange, Bend, Cut, Unfold/Refold mit K-Faktor-Berechnungen
- ✅ **Routing**: A*-basierte Pfadfindung für Pipes, Hoses, Tubes
- ✅ **Direct Editing**: Face-Modifikation, Offset, Deletion, Freeform
- ✅ **Drawing Generation**: TechDraw Integration mit ISO/ANSI/JIS Templates
- ✅ **Project Management**: Save/Load, Checkpoints, Auto-Save
- ✅ **User Authentication**: Login, Registration, Session Management
- ✅ **AI Integration**: OpenAI/Anthropic Support für CAD-Assistenz
- ✅ **Auto-Updates**: In-Place Updates ohne Neuinstallation

## Installation & Build

### Voraussetzungen
- **Windows 10/11** (64-bit)
- **Visual Studio 2019+** oder **MinGW-w64**
- **CMake 3.26+**
- **Qt 5.15+** oder **Qt 6.x** (für UI)
- **NSIS 3.0+** (für Installer-Erstellung)

### Automatischer Build & Installer (Empfohlen)

**PowerShell**:
```powershell
.\build_installer.ps1
```

**Batch**:
```cmd
.\build_installer.bat
```

### Manueller Build

```powershell
# CMake konfigurieren
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCAD_USE_QT=ON

# Kompilieren
cmake --build build --config Release

# Installer erstellen
cd installer
"C:\Program Files (x86)\NSIS\makensis.exe" hydracad.nsi
```

### Installation

1. Doppelklick auf `HydraCADSetup.exe`
2. Installationspfad wählen (Standard: `C:\Program Files\Hydra CAD`)
3. Komponenten auswählen:
   - **Core Application** (immer installiert)
   - **Python Bindings** (optional)
   - **Example Files** (optional)
4. Installation durchführen

**Nach der Installation**:
- Desktop-Shortcut: `Hydra CAD.lnk`
- Startmenü: `Hydra CAD → Hydra CAD`
- File-Association: `.cad` Dateien öffnen mit Hydra CAD
- Uninstaller: `C:\Program Files\Hydra CAD\Uninstall.exe`

## 📚 Dokumentation

Detaillierte Dokumentation finden Sie im `docs/` Verzeichnis:
- **[PROJECT_FINAL.md](docs/PROJECT_FINAL.md)** - Vollständige Projekt-Dokumentation
- **[INSTALLATION.md](docs/INSTALLATION.md)** - Installations-Anleitung
- **[FINAL_RELEASE_NOTES.md](docs/FINAL_RELEASE_NOTES.md)** - Release Notes 2.0.0

Siehe auch: [docs/README.md](docs/README.md) für eine vollständige Übersicht.

## Projekt-Verwaltung

### Projekte speichern/laden
- **Speichern**: File → Save Project (Strg+S)
- **Laden**: File → Open Project (Strg+O)
- **Checkpoints**: Automatische Sicherungspunkte
- **Auto-Save**: Automatisches Speichern alle 5 Minuten

### User-Authentifizierung
- **Registrierung**: Beim ersten Start
- **Login**: Bei jedem Start (optional: Remember Me)
- **Session Management**: Automatische Session-Verwaltung

## System-Anforderungen

- **Windows**: Windows 10/11 (64-bit)
- **Linux**: Ubuntu 20.04+ / Debian 11+ / Fedora 34+ (64-bit)
- **macOS**: macOS 10.15+ (Intel/Apple Silicon)
- **RAM**: 4 GB minimum, 8 GB empfohlen
- **Festplatte**: 500 MB für Installation, zusätzlich für Projekte
- **Grafik**: OpenGL 3.3+ kompatible Grafikkarte

## Lizenz

Siehe [installer/license.txt](installer/license.txt)

## 📝 Changelog

Siehe [CHANGELOG.md](CHANGELOG.md) für detaillierte Änderungen.

## Support

Für Fragen und Support siehe die Dokumentation im `docs/` Verzeichnis.

---

**Version**: 2.0.0  
**Release Date**: 23. Januar 2026  
**Status**: ✅ **Produktionsbereit**
