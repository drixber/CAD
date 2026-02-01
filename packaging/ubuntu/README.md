# Ubuntu / Debian – .deb bauen

Lokales Bauen eines .deb-Pakets aus dem Quellcode (kein offizielles Release-Paket).

## Abhängigkeiten (apt)

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
  debhelper dpkg-dev
```

Optional: `qt6-network-dev` (falls Build mit Qt-Netzwerk gewünscht).

## .deb bauen

**Skript aus dem Repository-Root ausführen** (nicht aus `packaging/ubuntu/`):

```bash
./packaging/ubuntu/build-deb.sh
```

Das Skript kopiert `packaging/debian/` nach `debian/`, baut mit CMake (Qt6, Eigen-Kern) und erzeugt ein binäres Paket. Die .deb-Datei liegt im übergeordneten Verzeichnis des Repos (z. B. `../hydracad_3.0.14-1_amd64.deb`). Die Versionsnummer steht in `packaging/debian/changelog`.

## Manuell (ohne Skript)

```bash
cp -r packaging/debian debian
chmod +x debian/rules
dpkg-buildpackage -b -us -uc -rfakeroot
```

## Installieren des .deb

```bash
sudo dpkg -i ../hydracad_3.0.14-1_amd64.deb
# Abhängigkeiten ggf. nachziehen:
sudo apt-get install -f
```

## Fehlerbehebung

- **„packaging/debian not found“:** Skript aus dem **Repository-Root** ausführen (nicht aus `packaging/ubuntu/`).
- **Fehlende Abhängigkeiten:** `sudo apt-get build-dep .` im Repo-Root (nach `cp -r packaging/debian debian`) installiert die in `debian/control` gelisteten Build-Depends.

## Verweise

- [docs/BUILD_LINUX.md](../../docs/BUILD_LINUX.md) – allgemeiner Linux-Build
- [packaging/debian/](../debian/) – Debian-Steuerdateien (control, rules, changelog, …)
