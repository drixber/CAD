# macOS – App-Bundle und Release

- **HydraCAD.png:** Icon (256×256) für die App; in CI mit `iconutil` zu `.icns` konvertiert und in `Hydra CAD.app/Contents/Resources` eingebunden.
- **CI:** `release.yml` und `release-attach-assets.yml` bauen unter macOS die App (MACOSX_BUNDLE), erzeugen das Icon, führen `macdeployqt` aus und packen **HydraCAD-macos.zip** (bzw. Tarball); das Archiv wird ans GitHub-Release gehängt.

Siehe [docs/BUILD_MACOS.md](../../docs/BUILD_MACOS.md) für den lokalen Build.
