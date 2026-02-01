# Linux-Unterstützung – abgeschlossen

Linux ist **vollständig unterstützt**: Build, portable Tarballs, AppImage, CI und Release-Assets sind eingerichtet.

---

## Status (Übersicht)

| Bereich | Status | Details |
|--------|--------|---------|
| **Doku** | ✅ | BUILD_ARCH.md, BUILD_LINUX.md (Ubuntu/Debian/Fedora), INSTALLATION.md Linux-Abschnitt |
| **CMake** | ✅ | Unix/Apple unterstützt (install, AppImage-ready); kein WIN32 auf Linux |
| **Packaging** | ✅ | Arch: PKGBUILD. Ubuntu/Debian: README + optionale .deb-Hinweise. Linux: HydraCAD.desktop, HydraCAD.png, AppImage/Tarball in release.yml |
| **CI** | ✅ | release.yml: Linux-Job baut AppImage + portable Tarball; release-attach-assets.yml hängt hydracad-linux-portable.tar.gz ans Release |
| **Icon** | ✅ | packaging/linux/HydraCAD.png (256×256); Desktop-Eintrag Icon=HydraCAD; in AppDir und Tarball eingebunden |
---

## Erledigt

- [x] **Doku Arch:** BUILD_ARCH.md, Pacman-Abhängigkeiten, PKGBUILD-Vorlage (`packaging/arch/`)
- [x] **Doku Linux (allgemein):** BUILD_LINUX.md für Ubuntu/Debian/Fedora (Paketnamen, Build)
- [x] **INSTALLATION.md:** Linux-Abschnitt mit Verweisen auf BUILD_LINUX.md und BUILD_ARCH.md
- [x] **Linux-CI:** release.yml und release-attach-assets.yml bauen Linux (AppImage + Tarball) und hängen den Tarball ans Release
- [x] **Packaging Linux:** HydraCAD.desktop, HydraCAD.png; Icon in AppDir und portable Tarball; linuxdeploy --icon-file
- [x] **Icon:** packaging/linux/HydraCAD.png (256×256) für AppImage/Desktop
- [x] **.deb/.rpm:** Optionale Hinweise in packaging/ubuntu/README.md und packaging/debian/README.md (kein offizielles .deb-Release)

---

## Release-Assets

Bei **Publish release** werden automatisch bereitgestellt:

- **hydracad-linux-portable.tar.gz** – entpacken, Qt6 installieren, `./run.sh`; enthält HydraCAD.desktop und HydraCAD.png
- Optional: **HydraCAD-Linux.AppImage** (Artifact aus release.yml, wenn linuxdeploy erfolgreich)

Doku: [BUILD_LINUX.md](BUILD_LINUX.md), [BUILD_ARCH.md](BUILD_ARCH.md), [INSTALLATION.md](INSTALLATION.md).
