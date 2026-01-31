# TODO: Linux-Unterstützung (im Hintergrund)

**Hinweis:** Linux-Support ist bewusst **nicht produktionsfertig** und **existiert im Hintergrund**. Es gibt keine offiziellen Linux-Releases, keine Download-Pakete auf der Release-Seite und keine Bewerbung als produktionsreif. Ziel: Build und Entwicklung unter Linux (Ubuntu, Debian, Fedora, Arch) ermöglichen, CI und Doku vorhalten, ohne den Fokus von Windows zu nehmen.

---

## Status (Übersicht)

| Bereich | Status | Details |
|--------|--------|---------|
| **Doku** | ✅ | BUILD_ARCH.md, BUILD_LINUX.md (Ubuntu/Debian/Fedora), INSTALLATION.md Linux-Abschnitt |
| **CMake** | ✅ | Unix/Apple bereits unterstützt (install, AppImage-ready); kein WIN32 auf Linux |
| **Packaging** | ✅ | Arch: PKGBUILD. Ubuntu/Debian: README. Linux: HydraCAD.desktop, AppImage/Tarball in release.yml |
| **CI** | ✅ | release.yml: Linux-Job baut bei Push main/master (wie Windows) AppImage + portable Tarball, Artifacts 7 Tage. build-linux.yml weiterhin manuell. **Nicht releast:** Kein automatischer Upload ins GitHub-Release, Linux wird nicht beworben. |
| **README/Status** | ✅ | Linux als „experimentell / im Hintergrund“ gekennzeichnet |

---

## Erledigt

- [x] **Doku Arch:** BUILD_ARCH.md, Pacman-Abhängigkeiten, PKGBUILD-Vorlage (`packaging/arch/`)
- [x] **Doku Linux (allgemein):** BUILD_LINUX.md für Ubuntu/Debian/Fedora (Paketnamen, Build)
- [x] **INSTALLATION.md:** Linux-Abschnitt mit Verweisen auf BUILD_LINUX.md und BUILD_ARCH.md
- [x] **Linux-CI:** Workflow `build-linux.yml` (nur workflow_dispatch), baut unter Ubuntu, lädt Artifacts hoch (kein GitHub-Release)
- [x] **Packaging-Hinweise:** `packaging/ubuntu/README.md` / Debian mit Paketnamen, kein produktiver .deb-Build
- [x] **README:** Linux/Arch als experimentell bzw. im Hintergrund beschrieben
- [x] **Linux in release.yml:** Job `build-linux` baut bei gleichem Trigger wie Windows (push main/master, workflow_dispatch); erzeugt AppImage (linuxdeploy + Qt-Plugin) und portables Tarball; lädt Artifacts hoch (HydraCAD-Linux-AppImage, HydraCAD-Linux-Portable). **Ohne Release:** Kein Upload ins GitHub-Release, Linux wird nicht offiziell beworben.
- [x] **Packaging Linux:** `packaging/linux/HydraCAD.desktop` für AppImage; README mit Hinweis auf Icon (optional).
- [x] **Arch pacman/yay:** PKGBUILD AUR-ready (Desktop-Datei, Lizenz, qt6-translations, optdepends qt6-network). Doku [INSTALL_ARCH_PACMAN_YAY.md](INSTALL_ARCH_PACMAN_YAY.md): Installation mit pacman (lokal `makepkg -si`), mit yay (AUR), Anleitung zum Anlegen des AUR-Pakets.

---

## Offen (optional, weiter im Hintergrund)

- [ ] **Release-Upload:** Wenn gewünscht: Linux-Artifacts automatisch ans GitHub-Release hängen (z. B. Workflow bei `release: published` oder Download/Upload aus Artifacts).
- [ ] **Icon:** `packaging/linux/HydraCAD.png` (256×256) für AppImage/Desktop (optional).
- [ ] **.deb/.rpm:** Nur wenn gewünscht: Vorlagen oder Skripte für lokales Bauen; keine Integration in Haupt-Release.
- [ ] **FreeCAD unter Linux:** Doku prüfen, ob CAD_USE_FREECAD unter Ubuntu/Arch mit System-FreeCAD funktioniert.

---

## Nicht geplant (Produktion) – unverändert

- Kein offizieller Linux-Download auf der Release-Seite (Artifacts nur im Workflow-Run)
- Kein „Download for Linux“ im Haupt-README als gleichwertig zu Windows
- Kein automatischer Upload der Linux-Dateien ins GitHub-Release (kann bei Bedarf ergänzt werden)

---

**Zuordnung:** Siehe auch [INSTALLATION.md](INSTALLATION.md) (Linux-Abschnitt), [BUILD_LINUX.md](BUILD_LINUX.md), [BUILD_ARCH.md](BUILD_ARCH.md).
