# Hydra CAD – Build unter Linux (Ubuntu, Debian, Fedora)

**Arch Linux:** Siehe [BUILD_ARCH.md](BUILD_ARCH.md) (Pacman, PKGBUILD).

---

## Installation von der Release-Seite (portable Tarball)

Wenn auf der [GitHub-Release-Seite](https://github.com/drixber/CAD/releases) ein **hydracad-linux-portable.tar.gz** liegt:

1. **Tarball herunterladen** und entpacken:
   ```bash
   tar xzf hydracad-linux-portable.tar.gz
   cd hydracad-linux-portable   # oder der entpackte Ordner
   ```
2. **Qt6 installieren** (falls noch nicht vorhanden):
   - **Ubuntu/Debian:** `sudo apt install qt6-base-dev` (oder `libqt6core6` für nur Laufzeit)
   - **Fedora:** `sudo dnf install qt6-qtbase`
   - **Arch:** `sudo pacman -S qt6-base`
3. **Starten:** `./run.sh` (oder `usr/bin/cad_desktop`)

Die Datei **README.txt** im Ordner enthält dieselbe Kurzanleitung.

---

## Aus dem Quellcode bauen (Voraussetzungen)

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools
```

Optional (Tests, Qt Network für Updates ohne curl):

```bash
sudo apt install -y qt6-network-dev  # CAD_USE_QT_NETWORK
# Optional: FreeCAD-Entwicklungspakete, Python-Dev für Bindings
```

### Fedora

```bash
sudo dnf install -y gcc-c++ cmake qt6-qtbase-devel qt6-tools-devel
```

Optional:

```bash
sudo dnf install -y qt6-qtnetwork  # CAD_USE_QT_NETWORK
```

---

## Build

```bash
git clone https://github.com/drixber/CAD.git
cd CAD
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON ..
make -j$(nproc)
```

Ausführbare Datei: `build/cad_desktop`. Mit Tests:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON -DCAD_BUILD_TESTS=ON ..
make -j$(nproc)
ctest --output-on-failure
```

---

## Optionale Build-Flags

| Option | Beschreibung |
|--------|--------------|
| `CAD_USE_QT=ON` | Qt-UI (empfohlen) |
| `CAD_BUILD_TESTS=ON` | Tests bauen und ausführen |
| `CAD_USE_FREECAD=ON` | FreeCAD-Integration (Entwicklungspakete nötig) |
| `CAD_BUILD_PYTHON=ON` | Python-Bindings |

Unter Ubuntu/Debian/Fedora setzt CMake Qt in der Regel automatisch voraus; `Qt6_DIR` nur bei manueller Qt-Installation nötig.

---

## Install (optional, lokal)

```bash
cd build
sudo cmake --install .
# cad_desktop liegt dann z. B. unter /usr/local/bin
```

---

## Troubleshooting

- **„cannot open shared object file“ / Qt-Bibliotheken fehlen:** Qt6 installieren (siehe oben). `./run.sh` setzt `LD_LIBRARY_PATH` auf typische Lib-Pfade; bei eigener Qt-Installation ggf. `export LD_LIBRARY_PATH=/pfad/zu/qt/lib` vor `./run.sh`.
- **Kein Start / schwarzes Fenster:** Grafiktreiber und OpenGL prüfen; Logs unter **Settings → Diagnostics** in der App.

---

## Verweise

- [BUILD_ARCH.md](BUILD_ARCH.md) – Arch Linux (Pacman, PKGBUILD)
- [INSTALLATION.md](INSTALLATION.md) – Allgemeine Installation
- [TODO_LINUX.md](TODO_LINUX.md) – Linux-Status
