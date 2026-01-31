# macOS-Unterstützung – abgeschlossen

macOS ist **vollständig unterstützt**: Build, App-Bundle, CI und Release-Assets sind eingerichtet.

---

## Status (Übersicht)

| Bereich | Status | Details |
|--------|--------|---------|
| **Doku** | ✅ | BUILD_MACOS.md (Qt, Xcode, Build, Install); INSTALLATION.md macOS-Abschnitt |
| **CMake** | ✅ | MACOSX_BUNDLE (Bundle-Name, Identifier, Version); optional Icon aus packaging/macos |
| **Packaging** | ✅ | packaging/macos: HydraCAD.png, README; CI erzeugt .icns und hängt Icon an .app |
| **CI** | ✅ | release.yml: macOS-Job baut .app + portable Archiv; release-attach-assets.yml hängt HydraCAD-macos.zip ans Release |
| **Icon** | ✅ | packaging/macos/HydraCAD.png (256×256); in CI zu .icns konvertiert und in .app/Contents/Resources eingebunden |

---

## Erledigt

- [x] **Doku:** BUILD_MACOS.md (Voraussetzungen, Qt, Build, optionale Flags, Troubleshooting)
- [x] **INSTALLATION.md:** macOS-Abschnitt mit Verweis auf BUILD_MACOS.md
- [x] **Packaging macOS:** packaging/macos/ mit HydraCAD.png und README; CI erzeugt .icns und portable .zip/.tar.gz
- [x] **macOS-CI:** release.yml und release-attach-assets.yml bauen macOS (.app), macdeployqt, Icon; Upload als HydraCAD-macos.zip
- [x] **Icon:** packaging/macos/HydraCAD.png; in CI mit iconutil zu .icns, in .app/Contents/Resources kopiert

---

## Release-Assets

Bei **Publish release** werden automatisch bereitgestellt:

- **HydraCAD-macos.zip** (oder hydracad-macos-portable.tar.gz) – entpacken, **Hydra CAD.app** starten; enthält Qt-Bibliotheken und Icon.

Doku: [BUILD_MACOS.md](BUILD_MACOS.md), [INSTALLATION.md](INSTALLATION.md).
