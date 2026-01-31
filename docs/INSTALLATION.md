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

## Releases erstellen (Maintainer)

- **Neuer Release:** Tag pushen, z. B. `git tag v3.0.10 && git push origin v3.0.10`. Der Workflow **Release bei Tag-Push erstellen** (`.github/workflows/create-release-on-tag.yml`) muss dafür **bereits auf GitHub im Branch `main` liegen**. Er erstellt dann automatisch das GitHub-Release; **Release – Assets anhängen** baut Windows (+ Linux) und hängt Installer/ZIP an.
- **Bereits gepushte Tags ohne Release:** In GitHub unter **Releases → Draft a new release** den gewünschten Tag (z. B. v3.0.10) auswählen, Titel/Notizen eintragen, **Publish release** klicken. Danach baut der CI die Assets und hängt sie an.

### Warum hat sich nach „git push origin v3.0.10“ nichts geändert?

- Der Workflow **create-release-on-tag.yml** läuft nur, wenn er auf GitHub im Branch **main** existiert. Wenn du den Tag gepusht hast, **bevor** dieser Workflow nach GitHub gepusht wurde, wird kein Release erzeugt.
- **Jetzt beheben:**  
  1. Alle Änderungen (inkl. `.github/workflows/create-release-on-tag.yml`) nach `main` pushen: `git push origin main`.  
  2. Für den **bereits gepushten** Tag v3.0.10 einmal manuell ein Release anlegen: GitHub → **Releases** → **Draft a new release** → Tag **v3.0.10** auswählen → **Publish release**. Danach baut der CI und hängt die Assets an.  
- **Ab dem nächsten Tag:** Nach dem Push von `main` mit dem Workflow reicht für neue Versionen wieder: `git tag v3.0.11 && git push origin v3.0.11` – dann wird das Release automatisch erstellt.

### Release ohne EXE/ZIP (nur Source code)

Wenn ein Release nur **Source code (zip/tar.gz)** hat und **keine** HydraCADSetup.exe oder app-windows.zip:

1. **Actions prüfen:** GitHub → **Actions** → Workflow **„Release – Assets anhängen“** öffnen → nach dem Lauf suchen, der beim Veröffentlichen des Releases gestartet wurde. Wenn der Lauf **fehlgeschlagen** ist: **Re-run all jobs** ausführen (kann bei temporären Fehlern helfen).
2. **Manuell nachziehen:** Workflow **„Release – Assets anhängen“** hat einen **manuellen Trigger**. GitHub → **Actions** → **Release – Assets anhängen** → **Run workflow** → Branch **main**, Eingabe **tag_name** z. B. `v3.0.11` → **Run workflow**. Der Lauf baut Windows + Linux und hängt EXE/ZIP/Tarball am angegebenen Release an.
3. **Änderung pushen:** Die Workflow-Datei muss mit dem manuellen Trigger auf **main** liegen; danach unter Actions den Lauf wie unter 2. starten.

## Checkliste: Was du jetzt machen musst

Damit **Releases** und **AUR** stimmen, reicht diese Reihenfolge:

1. **Änderungen nach GitHub pushen**  
   ```bash
   git add -A
   git status   # prüfen
   git commit -m "Doku: Checkliste, AUR-SSH, Release-Workflow"
   git push origin main
   ```
   So ist u. a. der Workflow **Release bei Tag-Push erstellen** (mit Checkout) auf GitHub.

2. **GitHub Releases**  
   - Wenn ein Tag (z. B. v3.0.10 oder v3.0.11) schon existiert, aber **kein** Release:  
     **Releases** → **Draft a new release** → Tag auswählen → **Publish release**.  
   - Für **neue** Versionen danach: `git tag vX.Y.Z && git push origin vX.Y.Z` – Release wird automatisch erstellt.

3. **AUR (Arch)**  
   - Einmalig: Auf [aur.archlinux.org](https://aur.archlinux.org) Account anlegen, SSH-Key hinterlegen, Paket **hydracad** anlegen (Submit Package).  
   - Auf **Arch Linux**: Zuerst ins **Projektroot** (dort, wo z. B. `README.md` und der Ordner `packaging` liegen), dann:
     ```bash
     cd packaging/arch
     # ggf. PKGVER in aur-upload.sh anpassen
     ./aur-upload.sh
     ```
   - Beim ersten SSH-Kontakt Fingerabdruck mit der Tabelle in `packaging/arch/README.md` abgleichen.

Danach stimmen Releases (Tag-Push → automatisches Release + Assets) und AUR (Paket hydracad, Updates per `./aur-upload.sh`).

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

Aktuell wird der automatische Release-Build nur für Windows ausgeführt. **Linux-Support ist experimentell und existiert im Hintergrund** – keine offiziellen Linux-Releases, siehe [docs/TODO_LINUX.md](TODO_LINUX.md). Für Linux/macOS aus dem Quellcode bauen:

- **Linux (Ubuntu, Debian, Fedora):** Download von der Release-Seite: **hydracad-linux-portable.tar.gz** entpacken, Qt6 installieren (apt/dnf), dann `./run.sh` starten. Aus dem Quellcode bauen: [docs/BUILD_LINUX.md](BUILD_LINUX.md). Paketnamen: `packaging/ubuntu/README.md`, `packaging/debian/README.md`.
- **Arch Linux:** [docs/BUILD_ARCH.md](BUILD_ARCH.md). Installation mit **pacman** (lokal) oder **yay** (AUR): [docs/INSTALL_ARCH_PACMAN_YAY.md](INSTALL_ARCH_PACMAN_YAY.md). PKGBUILD: `packaging/arch/PKGBUILD`.
- **Linux-CI (im Hintergrund):** Workflow `build-linux.yml` nur per manuellem Auslösen (workflow_dispatch); baut unter Ubuntu, Artifacts für Entwickler, kein Release-Upload.
- **macOS:** Qt 6 über den Qt Installer, dann CMake; optional `-DCAD_USE_QT=ON` und MACOSX_BUNDLE wird gesetzt.

## Troubleshooting

### "DLL nicht gefunden" (Windows)

- Nach dem Build `windeployqt` auf `cad_desktop.exe` ausführen
- Sicherstellen, dass `Qt6Core.dll`, `Qt6Gui.dll`, `platforms\qwindows.dll` neben der EXE liegen

### Anwendung startet nicht

- Visual C++ Redistributable (z. B. VC++ 2015–2022) installieren
- Logs prüfen: **Settings → Diagnostics → Open Logs Folder** bzw. **Show Startup Log**

### Schwarzes Fenster / nur Terminal nach Update – „Als Administrator“ nötig

Wenn nach einem Update nur ein schwarzes Fenster erscheint und die Anwendung erst nach „Als Administrator ausführen“ startet, lag das an einem falschen Arbeitsverzeichnis beim Start. Ab der aktuellen Version:

- Das Arbeitsverzeichnis wird beim Start auf das Installationsverzeichnis gesetzt (unabhängig davon, ob Sie per Verknüpfung oder über den Installer starten).
- Updates werden in einen benutzerbeschreibbaren Ordner (Temp) heruntergeladen, sodass keine Schreibrechte in „Programme“ nötig sind.

Falls das Problem bei einer älteren Installation weiterhin auftritt: einmal **Als Administrator ausführen**, danach normal starten; oder die Anwendung neu installieren.

### CMake findet Qt 6 nicht

- `Qt6_DIR` auf das Qt-Verzeichnis setzen, das die Datei `Qt6Config.cmake` enthält (z. B. `C:\Qt\6.5.3\msvc2019_64\lib\cmake\Qt6`)

## Weitere Informationen

- **Projekt-README:** [../README.md](../README.md)
- **Changelog:** [../CHANGELOG.md](../CHANGELOG.md)
