# Hydra CAD – Professional CAD Application

## Overview

Hydra CAD is a comprehensive Computer-Aided Design (CAD) application built with C++ and Qt 6, with 3D viewport rendering, project management, AI-assisted workflows, and an integrated CAD kernel (optional Coin3D for 3D display).

**Status**: ✅ Windows releases via GitHub (Installer + Portable ZIP). Version from Git tag (e.g. `v1.0.0`).

---

## 🚀 Schnellstart

### Installation (Windows)

| Option | Datei | Schritte |
|--------|------|----------|
| **Installer** | [HydraCADSetup.exe](https://github.com/drixber/CAD/releases) | Herunterladen → Ausführen → Datenschutz bestätigen, Pfad und Sprache wählen |
| **Portable** | [app-windows.zip](https://github.com/drixber/CAD/releases) | ZIP entpacken → `cad_desktop.exe` starten |

**Releases**: [GitHub Releases](https://github.com/drixber/CAD/releases). Ein **Tag** (z. B. `v3.0.10`) pushen → automatisch wird ein Release erstellt und der CI hängt Installer + Portable an. Ohne manuelles „Draft new release“.

**Linux / macOS**: Bei **Publish release** erscheinen **HydraCADSetup.exe**, **app-windows.zip**, **hydracad-linux-portable.tar.gz** (Linux: entpacken, Qt6 installieren, `./run.sh`) und **HydraCAD-macos.zip** (macOS: entpacken, **Hydra CAD.app** starten). Aus Quellcode bauen: [docs/INSTALLATION.md](docs/INSTALLATION.md), [docs/BUILD_LINUX.md](docs/BUILD_LINUX.md), [docs/BUILD_ARCH.md](docs/BUILD_ARCH.md), [docs/BUILD_MACOS.md](docs/BUILD_MACOS.md). Status: [docs/TODO_LINUX.md](docs/TODO_LINUX.md), [docs/TODO_MACOS.md](docs/TODO_MACOS.md).

### Erste Schritte

- **Erster Start**: Account registrieren, danach einloggen (optional: „Remember me“).
- **Projekt**: File → New Project / Open Project (Strg+O) / Save Project (Strg+S).
- **Sprache**: Settings → Language → Deutsch, English, 中文, 日本語 (wirkt nach Neustart).
- **Updates**: Settings → Check for Updates… (öffnet die Release-Seite bei neuer Version).

---

## Features

### Kern-Features

- **3D Viewport**: Coin3D/SoQt-Integration, Echtzeit-Darstellung.
- **Projekt-Verwaltung**: Speichern/Laden (`.cad`), Auto-Save (z. B. alle 5 Min.), Checkpoints.
- **Import/Export**: File-Dialog für STEP, IGES, STL, OBJ, DXF (Ribbon oder Befehl „Import“/„Export“).
- **Undo/Redo**: Ribbon oder Strg+Z / Strg+Y.
- **User & Profil**: Login, Registrierung, Session, User → Profile (Anzeige Nutzer/E-Mail).
- **Checkpoints**: File → Manage Checkpoints… – Liste, Öffnen, Löschen.
- **Sprachen**: Deutsch, English, 中文, 日本語 (Übersetzungen in der App).
- **Automatische Updates**: Settings → Check for Updates… – lädt bei neuer Version den Installer herunter und startet ihn; danach Setup abschließen, App neu starten. Kein manuelles Neu-Installieren nötig.

### Weitere Module (Build-Optionen)

- **Eigen-Kern**: Sketches, Parts, Extrude/Revolve, Drawings.
- **Constraint Solver**, **Simulation** (FEA, Motion, etc.), **Sheet Metal**, **Routing**, **Direct Editing**, **Drawing/TechDraw**.
- **AI**: OpenAI, Grok und Anthropic (Claude) für CAD-Assistenz.

---

## Installation & Build

### Voraussetzungen

- **Windows 10/11** (64-bit)
- **Visual Studio 2019/2022** (C++ Desktop) oder MinGW-w64
- **CMake 3.26+**
- **Qt 6.x** (für die UI)
- **NSIS 3.0+** (optional, nur für den Installer)

### Build & Installer (empfohlen)

```powershell
.\build_installer.ps1
```

Alternativ: `.\build_installer.bat`

### Manueller Build

```powershell
# Qt-Pfad anpassen
$QtDir = "C:\Qt\6.5.3\msvc2019_64"

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCAD_USE_QT=ON -DQt6_DIR="$QtDir" -DCMAKE_PREFIX_PATH="$QtDir"
cmake --build build --config Release

# Qt-DLLs für portable Nutzung
& "$QtDir\bin\windeployqt.exe" build\Release\cad_desktop.exe
```

EXE liegt in `build\Release\cad_desktop.exe`. Übersetzungen (`.qm`) werden beim Build nach `build\Release\i18n\` erzeugt.

**Installer**: Siehe [docs/INSTALLATION.md](docs/INSTALLATION.md) (NSIS, Projekt-Pfade).

---

## Projekt & Einstellungen

| Aktion | Menü / Tastatur |
|--------|------------------|
| Projekt speichern | File → Save Project (Strg+S) |
| Projekt öffnen | File → Open Project (Strg+O) |
| Checkpoints verwalten | File → Manage Checkpoints… |
| Sprache | Settings → Language |
| Updates prüfen | Settings → Check for Updates… |
| Profil anzeigen | User → Profile |
| Logs/Diagnose | Settings → Diagnostics |

Beim Öffnen eines anderen Projekts erscheint bei ungespeicherten Änderungen ein Dialog (Speichern / Verwerfen / Abbrechen).

---

## Entwicklung vs. installierte App / CAD Cursor

- **Installierte Hydra CAD** (z. B. nach Setup oder aus `app-windows.zip`) ist eine **standalone** C++/Qt-Anwendung. Sie ruft **kein** Python und **kein** `cadursor` auf.
- **CAD Cursor** (`python/cadursor/`) ist ein **separates** Python-Tool für Agent-Workflows (Planner, Executor, Historian, …). Es läuft nur, wenn du es im Repo explizit startest, z. B. `python -m cadursor.cli --goal "..." --rules .cursorcad`.
- **Es gibt keine automatische Einpflegung**: Was du in der installierten App machst (Projekte, Einstellungen), wird **nicht** automatisch ins Repo oder in die Cursor-Umgebung übernommen. Umgekehrt werden Änderungen im Repo erst nach neuem Build/Release in der installierten Version sichtbar.

---

## Systemanforderungen

- **Windows**: 10/11 (64-bit)
- **Linux** (vollständig unterstützt): Ubuntu 20.04+ / Debian 11+ / Fedora 34+ / Arch Linux (64-bit); Release: hydracad-linux-portable.tar.gz, alternativ aus Quellcode
- **macOS** (vollständig unterstützt): 10.15+ (Intel/Apple Silicon); Release: HydraCAD-macos.zip (Hydra CAD.app), alternativ aus Quellcode ([docs/BUILD_MACOS.md](docs/BUILD_MACOS.md))
- **RAM**: 4 GB mind., 8 GB empfohlen
- **Festplatte**: ca. 500 MB + Platz für Projekte
- **Grafik**: OpenGL 3.3+

---

## Dokumentation & Lizenz

- **Installation/Build**: [docs/INSTALLATION.md](docs/INSTALLATION.md)
- **Code-Signing (Windows)**: [docs/CODE_SIGNING.md](docs/CODE_SIGNING.md)
- **Changelog**: [CHANGELOG.md](CHANGELOG.md)
- **Lizenz**: [installer/license.txt](installer/license.txt)

---

**Version**: Aus Git-Tag (z. B. `v1.0.0`).  
**Releases**: ✅ Push eines Tags `v*` erstellt automatisch das GitHub-Release; CI baut Windows (Installer + Portable) und hängt die Assets an.
