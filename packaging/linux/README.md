# Linux – AppImage / portable

- **HydraCAD.desktop:** Desktop-Eintrag für AppImage/Desktop-Integration.
- **Icon:** `HydraCAD.png` (256×256) – wird von linuxdeploy fürs AppImage und im portable Tarball für Desktop-Einträge genutzt.
- **CI:** `release.yml` baut bei Push auf `main`/`master` (oder workflow_dispatch) neben Windows auch Linux: AppImage (linuxdeploy + Qt-Plugin) und portables Tarball (`hydracad-linux-portable.tar.gz`). Beides wird als Artifact hochgeladen; kein automatischer Upload ins GitHub-Release.
