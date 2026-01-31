# Ubuntu / Debian – Paketnamen (nur Doku, kein Produktions-Release)

Linux-Support ist **im Hintergrund**; es gibt keine offiziellen .deb-Pakete. Diese Liste dient dem lokalen Bauen aus dem Quellcode.

## Abhängigkeiten (apt)

```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools
```

Optional: `qt6-network-dev` (Updates ohne curl), FreeCAD-Entwicklungspakete, Python-Dev für Bindings.

## Build

Siehe [docs/BUILD_LINUX.md](../../docs/BUILD_LINUX.md). Es wird **kein** .deb gebaut oder auf der Release-Seite bereitgestellt.
