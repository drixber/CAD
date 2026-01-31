# Debian – Paketnamen (nur Doku, kein Produktions-Release)

Linux-Support ist **im Hintergrund**; es gibt keine offiziellen .deb-Pakete. Diese Liste dient dem lokalen Bauen aus dem Quellcode.

## Abhängigkeiten (apt)

Unter Debian 11+ (Bullseye bzw. Bookworm) ggf. Backports oder Qt aus dem Repo:

```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools
```

Falls Qt 6 nicht im Standard-Repo ist: Qt Online Installer oder Backports nutzen.

## Build

Siehe [docs/BUILD_LINUX.md](../../docs/BUILD_LINUX.md). Es wird **kein** .deb gebaut oder auf der Release-Seite bereitgestellt.
