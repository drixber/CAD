# Hydra CAD – Build & Entwicklung unter Arch Linux

Diese Anleitung bereitet das Projekt so vor, dass Hydra CAD unter **Arch Linux** aus dem Quellcode gebaut und dort **weiterentwickelt** werden kann. Es wird kein zusätzlicher Linux-spezifischer Code eingebaut; die bestehende CMake- und Qt-Unterstützung wird genutzt.

---

## Voraussetzungen (Arch)

Installation der Abhängigkeiten mit Pacman:

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-tools qt6-translations
```

Optional (für Tests, FreeCAD-Integration, Python-Bindings):

```bash
sudo pacman -S --needed qt6-network  # für CAD_USE_QT_NETWORK (Updates ohne curl)
# Optional:
# sudo pacman -S freecad  # wenn FreeCAD-Integration gebaut werden soll
# sudo pacman -S python python-pip  # für CAD_BUILD_PYTHON
```

---

## Build (Arch)

```bash
git clone https://github.com/drixber/CAD.git
cd CAD
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON ..
make -j$(nproc)
```

Die ausführbare Datei liegt in `build/cad_desktop`. Optional mit Tests:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON -DCAD_BUILD_TESTS=ON ..
make -j$(nproc)
ctest --output-on-failure
```

**Hinweis:** Unter Arch findet CMake Qt 6 in der Regel automatisch über die Systeminstallation (`/usr`). Ein explizites `Qt6_DIR` ist nur nötig, wenn Qt manuell installiert wurde.

---

## Optionale Build-Flags

| Option | Beschreibung |
|--------|--------------|
| `CAD_USE_QT=ON` | Qt-UI (empfohlen) |
| `CAD_BUILD_TESTS=ON` | Tests bauen und ausführen |
| `CAD_USE_QT_NETWORK=ON` | Wird gesetzt, wenn Qt6::Network gefunden wird – Updates ohne curl |
| `CAD_USE_FREECAD=ON` | FreeCAD-Integration (FreeCAD-Entwicklungspakete nötig) |
| `CAD_BUILD_PYTHON=ON` | Python-Bindings |

---

## Entwicklung unter Arch

- **Quellcode:** Wie oben bauen; Änderungen im Repo, dann `make -j$(nproc)` im `build`-Verzeichnis.
- **Paketierung:** Eine PKGBUILD-Vorlage liegt unter `packaging/arch/PKGBUILD`. Sie dient als Basis für AUR oder lokale Pakete; sie wird nicht automatisch gepflegt und kann bei Bedarf angepasst werden.
- **CI:** Derzeit läuft der automatische Release-Build nur für Windows. Linux/Arch-Builds können später in CI ergänzt werden (z. B. `ubuntu-latest` oder Arch-Container).

---

## Installation mit pacman / yay

- **Lokal bauen und mit pacman installieren:** Siehe [INSTALL_ARCH_PACMAN_YAY.md](INSTALL_ARCH_PACMAN_YAY.md) (Abschnitt „Installation mit pacman (lokal bauen)“).
- **Mit yay installieren:** Sobald ein AUR-Paket `hydracad` existiert: `yay -S hydracad`. Anleitung zum Anlegen des AUR-Pakets: [INSTALL_ARCH_PACMAN_YAY.md](INSTALL_ARCH_PACMAN_YAY.md).

## Verweise

- Linux (Ubuntu/Debian/Fedora): [BUILD_LINUX.md](BUILD_LINUX.md)
- pacman/yay und AUR: [INSTALL_ARCH_PACMAN_YAY.md](INSTALL_ARCH_PACMAN_YAY.md)
- Allgemeine Installation: [INSTALLATION.md](INSTALLATION.md)
- Linux-Status (im Hintergrund): [TODO_LINUX.md](TODO_LINUX.md)
- Projekt-README: [../README.md](../README.md)
