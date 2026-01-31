# Linux – AppImage / portable

- **HydraCAD.desktop:** Desktop-Eintrag für AppImage/Desktop-Integration.
- **Icon:** Optional – `HydraCAD.png` (z. B. 256×256) hier ablegen; wird von linuxdeploy fürs AppImage genutzt. Ohne Icon läuft das AppImage trotzdem.
- **CI:** `release.yml` baut bei Push auf `main`/`master` (oder workflow_dispatch) neben Windows auch Linux: AppImage (linuxdeploy + Qt-Plugin) und portables Tarball (`hydracad-linux-portable.tar.gz`). Beides wird als Artifact hochgeladen; kein automatischer Upload ins GitHub-Release.
