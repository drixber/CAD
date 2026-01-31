# Hydra CAD – Build unter Linux (Ubuntu, Debian, Fedora)

Linux-Build ist **experimentell** und **im Hintergrund** – es gibt keine offiziellen Linux-Releases. Diese Anleitung ermöglicht das Bauen und Weiterentwickeln aus dem Quellcode.

**Arch Linux:** Siehe [BUILD_ARCH.md](BUILD_ARCH.md) (Pacman, PKGBUILD).

---

## Voraussetzungen

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

## Verweise

- [BUILD_ARCH.md](BUILD_ARCH.md) – Arch Linux (Pacman, PKGBUILD)
- [INSTALLATION.md](INSTALLATION.md) – Allgemeine Installation
- [TODO_LINUX.md](TODO_LINUX.md) – Linux-Status (im Hintergrund, nicht produktionsfertig)
