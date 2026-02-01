# Hydra CAD – Build unter macOS

Diese Anleitung beschreibt den Build von Hydra CAD unter **macOS** (Intel und Apple Silicon).

---

## Installation von der Release-Seite (portable .app)

Wenn auf der [GitHub-Release-Seite](https://github.com/drixber/CAD/releases) ein **HydraCAD-macos.zip** oder **hydracad-macos-portable.tar.gz** liegt:

1. **Archiv herunterladen** und entpacken.
2. **Hydra CAD.app** in den Programme-Ordner verschieben (optional) oder direkt aus dem Entpack-Ordner starten.
3. **Erster Start:** Falls macOS die App blockiert („nicht verifizierter Entwickler“): **Systemeinstellungen → Datenschutz & Sicherheit** → „Trotzdem öffnen“ bei Hydra CAD.

Die App enthält Qt-Bibliotheken (macdeployqt); es ist keine separate Qt-Installation nötig.

---

## Aus dem Quellcode bauen (Voraussetzungen)

### Qt 6

- **Homebrew:** `brew install qt@6` (oder `qt6`) – danach `export Qt6_DIR=$(brew --prefix qt@6)/lib/cmake/Qt6`
- **Qt Online Installer:** [qt.io](https://www.qt.io/download) – Qt 6 für macOS installieren, danach `Qt6_DIR` auf das gewählte Kit setzen (z. B. `~/Qt/6.5.3/macos/lib/cmake/Qt6`).

### Xcode Command Line Tools (oder Xcode)

```bash
xcode-select --install
```

---

## Build

```bash
git clone https://github.com/drixber/CAD.git
cd CAD
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON ..
cmake --build build -j$(sysctl -n hw.ncpu)
```

Die App liegt als **Hydra CAD.app** in `build/cad_desktop.app` (CMake erzeugt das Bundle unter diesem Pfad). Mit Tests:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON -DCAD_BUILD_TESTS=ON ..
cmake --build build -j$(sysctl -n hw.ncpu)
ctest --output-on-failure
```

---

## Optionale Build-Flags

| Option | Beschreibung |
|--------|--------------|
| `CAD_USE_QT=ON` | Qt-UI (empfohlen) |
| `CAD_BUILD_TESTS=ON` | Tests bauen und ausführen |
| `CAD_BUILD_PYTHON=ON` | Python-Bindings |

**App-Bundle:** Unter macOS erzeugt CMake automatisch ein App-Bundle (`.app`) mit `MACOSX_BUNDLE`; Icon und Qt-Bibliotheken können mit `packaging/macos/` und `macdeployqt` ergänzt werden (siehe [packaging/macos/README.md](../packaging/macos/README.md)).

---

## Install (optional, lokal)

```bash
cd build
sudo cmake --install .
# Hydra CAD.app liegt dann z. B. unter /usr/local/bin/… oder im angegebenen Prefix
```

---

## Troubleshooting

- **„App ist beschädigt“ / Gatekeeper blockiert:** Rechtsklick → „Öffnen“ oder Systemeinstellungen → Datenschutz & Sicherheit → „Trotzdem öffnen“.
- **Qt nicht gefunden:** `Qt6_DIR` setzen (siehe oben) oder `CMAKE_PREFIX_PATH` auf das Qt-Installationsverzeichnis.
- **Kein Start / schwarzes Fenster:** Logs unter **Einstellungen → Diagnose** in der App; ggf. Grafiktreiber aktualisieren.

---

## Verweise

- [INSTALLATION.md](INSTALLATION.md) – Allgemeine Installation
- [TODO_MACOS.md](TODO_MACOS.md) – macOS-Status
- [BUILD_LINUX.md](BUILD_LINUX.md) – Linux-Build
