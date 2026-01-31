# Ubuntu / Debian – Paketnamen (nur Doku, kein Produktions-Release)

Es gibt keine offiziellen .deb-Pakete auf der Release-Seite. Diese Liste dient dem lokalen Bauen aus dem Quellcode (Linux wird vollständig unterstützt).

## Abhängigkeiten (apt)

```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools
```

Optional: `qt6-network-dev` (Updates ohne curl), FreeCAD-Entwicklungspakete, Python-Dev für Bindings.

## Build

Siehe [docs/BUILD_LINUX.md](../../docs/BUILD_LINUX.md). Es wird **kein** .deb auf der Release-Seite bereitgestellt. **Optional:** Ein lokales .deb kann bei Bedarf mit eigenen Skripten (z. B. `dpkg-buildpackage` oder Vorlage in einem separaten Packaging-Branch) gebaut werden – keine Integration in den Haupt-Release.
