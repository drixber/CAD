# Debian – Steuerdateien für .deb

Dieses Verzeichnis enthält die Debian-Packaging-Dateien zum Bauen eines lokalen .deb-Pakets.

## Dateien

- **control** – Paketmetadaten, Abhängigkeiten (Qt6, debhelper)
- **rules** – Build-Regeln (CMake, Eigen-Kern, Qt6)
- **changelog** – Paket-Changelog
- **compat** – debhelper-Compat-Level
- **copyright** – Lizenzhinweis
- **hydracad.install** – Liste der zu installierenden Dateien (Binary, Desktop, Icon, Lizenz)

## .deb bauen

Nicht direkt in diesem Ordner bauen; aus dem **Repository-Root**:

```bash
./packaging/ubuntu/build-deb.sh
```

Das Skript kopiert `packaging/debian/` nach `debian/` und startet `dpkg-buildpackage`. Siehe [packaging/ubuntu/README.md](../ubuntu/README.md).

## Abhängigkeiten (apt)

```bash
sudo apt install -y build-essential cmake pkg-config qt6-base-dev qt6-tools-dev qt6-tools-dev-tools debhelper dpkg-dev
```
