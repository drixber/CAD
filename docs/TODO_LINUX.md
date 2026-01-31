# TODO: Linux-Unterstützung (im Hintergrund)

**Hinweis:** Linux-Support ist bewusst **nicht produktionsfertig** und **existiert im Hintergrund**. Es gibt keine offiziellen Linux-Releases, keine Download-Pakete auf der Release-Seite und keine Bewerbung als produktionsreif. Ziel: Build und Entwicklung unter Linux (Ubuntu, Debian, Fedora, Arch) ermöglichen, CI und Doku vorhalten, ohne den Fokus von Windows zu nehmen.

---

## Status (Übersicht)

| Bereich | Status | Details |
|--------|--------|---------|
| **Doku** | ✅ | BUILD_ARCH.md, BUILD_LINUX.md (Ubuntu/Debian/Fedora), INSTALLATION.md Linux-Abschnitt |
| **CMake** | ✅ | Unix/Apple bereits unterstützt (install, AppImage-ready); kein WIN32 auf Linux |
| **Packaging** | 🔶 | Arch: PKGBUILD-Vorlage. Ubuntu/Debian: nur Paketnamen/README, kein .deb-Build |
| **CI** | 🔶 | build-linux.yml nur per workflow_dispatch (im Hintergrund), kein Release-Upload |
| **README/Status** | ✅ | Linux als „experimentell / im Hintergrund“ gekennzeichnet |

---

## Erledigt

- [x] **Doku Arch:** BUILD_ARCH.md, Pacman-Abhängigkeiten, PKGBUILD-Vorlage (`packaging/arch/`)
- [x] **Doku Linux (allgemein):** BUILD_LINUX.md für Ubuntu/Debian/Fedora (Paketnamen, Build)
- [x] **INSTALLATION.md:** Linux-Abschnitt mit Verweisen auf BUILD_LINUX.md und BUILD_ARCH.md
- [x] **Linux-CI:** Workflow `build-linux.yml` (nur workflow_dispatch), baut unter Ubuntu, lädt Artifacts hoch (kein GitHub-Release)
- [x] **Packaging-Hinweise:** `packaging/ubuntu/README.md` / Debian mit Paketnamen, kein produktiver .deb-Build
- [x] **README:** Linux/Arch als experimentell bzw. im Hintergrund beschrieben

---

## Offen (optional, weiter im Hintergrund)

- [ ] **AppImage:** Optionales Ziel oder Script zum Erzeugen einer AppImage (nur Doku/Skript, kein automatischer Release)
- [ ] **Linux-CI bei Push:** Optional `build-linux.yml` bei Push auf Branch `linux` oder `develop` ausführen (weiterhin kein Release)
- [ ] **.deb/.rpm:** Nur wenn gewünscht: Vorlagen oder Skripte für lokales Bauen; keine Integration in Haupt-Release
- [ ] **FreeCAD unter Linux:** Doku prüfen, ob CAD_USE_FREECAD unter Ubuntu/Arch mit System-FreeCAD funktioniert

---

## Nicht geplant (Produktion)

- Kein offizieller Linux-Download auf der Release-Seite
- Kein „Download for Linux“ im Haupt-README als gleichwertig zu Windows
- Kein automatischer Linux-Build bei jedem Release-Tag

---

**Zuordnung:** Siehe auch [INSTALLATION.md](INSTALLATION.md) (Linux-Abschnitt), [BUILD_LINUX.md](BUILD_LINUX.md), [BUILD_ARCH.md](BUILD_ARCH.md).
