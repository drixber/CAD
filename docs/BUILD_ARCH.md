# Hydra CAD – Build & Entwicklung unter Arch Linux

Diese Anleitung bereitet das Projekt so vor, dass Hydra CAD unter **Arch Linux** aus dem Quellcode gebaut und dort **weiterentwickelt** werden kann. Es wird kein zusätzlicher Linux-spezifischer Code eingebaut; die bestehende CMake- und Qt-Unterstützung wird genutzt.

---

## Voraussetzungen (Arch)

Installation der Abhängigkeiten mit Pacman:

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-tools qt6-translations
```

Optional (für Tests, Python-Bindings):

```bash
sudo pacman -S --needed qt6-network  # für CAD_USE_QT_NETWORK (Updates ohne curl)
# Optional:
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

### Ausprobieren (Test-Ablauf)

1. **Abhängigkeiten:** `sudo pacman -S --needed base-devel cmake qt6-base qt6-tools qt6-translations`
2. **Bauen:** Im Projektroot `mkdir -p build && cd build` → `cmake -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON ..` → `make -j$(nproc)`
3. **Starten (aus Build):** `./cad_desktop` (oder `./cad_desktop 2>&1` um Ausgabe zu sehen)
4. **Optional – als Paket installieren:** `cd ../packaging/arch && makepkg -si` → danach `cad_desktop` oder `hydracad` von überall bzw. aus dem Anwendungsmenü

---

## Optionale Build-Flags

| Option | Beschreibung |
|--------|--------------|
| `CAD_USE_QT=ON` | Qt-UI (empfohlen) |
| `CAD_BUILD_TESTS=ON` | Tests bauen und ausführen |
| `CAD_USE_QT_NETWORK=ON` | Wird gesetzt, wenn Qt6::Network gefunden wird – Updates ohne curl |
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

---

## Troubleshooting: App startet nicht

Mögliche Ursachen und was du prüfen kannst:

| Ursache | Symptom / Prüfung | Lösung |
|--------|--------------------|--------|
| **Qt-Plattform-Plugin** | Fehlermeldung „no Qt platform plugin could be loaded“ oder sofortiger Abbruch ohne Fenster. | Ab aktueller Version: Die App setzt `QT_PLUGIN_PATH` / `QT_QPA_PLATFORM_PLUGIN_PATH` nur, wenn neben der Binary ein `platforms/`-Verzeichnis existiert. Bei System-Install (`/usr/lib/hydracad`) nutzt Qt die System-Pfade. Falls du eine ältere Version hast: Paket neu bauen oder `cad_desktop` direkt aus der Konsole starten und Fehler lesen. |
| **Wayland** | Crash oder schwarzes Fenster unter Wayland. | Wrapper und App erzwingen bereits `QT_QPA_PLATFORM=xcb` (XWayland). Sicherstellen, dass XWayland läuft; ggf. `QT_QPA_PLATFORM=xcb cad_desktop` starten. |
| **Fehlende Qt-Abhängigkeiten** | Dynamischer Linker meldet fehlende `.so` (z. B. `libQt6Core.so.6`). | `sudo pacman -S qt6-base qt6-tools qt6-translations` (ggf. `qt6-network`). |
| **Wrapper-Pfad** | „cannot execute binary“ oder „No such file“. | Wrapper ruft `/usr/lib/hydracad/cad_desktop` auf. Prüfen: `ls -la /usr/lib/hydracad/cad_desktop` und ob die Datei ausführbar ist. |
| **Arbeitsverzeichnis / Logs** | App startet, aber Ressourcen fehlen oder Logs helfen bei Diagnose. | Logs: `~/.local/share/HydraCAD/Hydra CAD/logs/startup.log` und `last_crash.log`. In der App: **Settings → Diagnostics**. |
| **OpenGL / Grafik** | Schwarzes Fenster oder Crash beim Öffnen des 3D-Viewports. | Grafiktreiber und OpenGL 3.3+ prüfen; ggf. mit `LIBGL_ALWAYS_SOFTWARE=1 cad_desktop` testen (Software-Rendering). |
| **Desktop-Eintrag** | Klick im Menü tut nichts. | `TryExec=cad_desktop` – wenn `cad_desktop` nicht im PATH ist, wird der Eintrag ausgeblendet. Start aus Terminal: `cad_desktop` oder `hydracad`. |
| **Icon fehlt** | Nur Icon fehlt, App startet. | PKGBUILD installiert kein Icon; `Icon=HydraCAD` im Desktop-File kann zu generischem Symbol führen. Optional: Icon nach `/usr/share/icons/hicolor/` legen. |

**Schnellcheck:** In einem Terminal `cad_desktop` (oder `hydracad`) ausführen – dann siehst du Fehlermeldungen von Qt bzw. dem Linker direkt.

---

## Verweise

- Linux (Ubuntu/Debian/Fedora): [BUILD_LINUX.md](BUILD_LINUX.md)
- pacman/yay und AUR: [INSTALL_ARCH_PACMAN_YAY.md](INSTALL_ARCH_PACMAN_YAY.md)
- Allgemeine Installation: [INSTALLATION.md](INSTALLATION.md)
- Linux-Status: [TODO_LINUX.md](TODO_LINUX.md)
- Projekt-README: [../README.md](../README.md)
