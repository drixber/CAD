# Linux – AppImage / portable

- **HydraCAD.desktop:** Desktop-Eintrag für Paketinstallation (Exec=/usr/bin/cad_desktop).
- **HydraCAD-Build.desktop:** Ein Eintrag „Hydra CAD“, der direkt aus dem Build-Verzeichnis startet (für Entwicklung ohne Installation). Nach `cp HydraCAD-Build.desktop ~/.local/share/applications/HydraCAD.desktop` erscheint nur „Hydra CAD“ im Menü und startet den Build.
- **cad_desktop.wrapper:** Launcher-Skript für Paketinstallationen (z. B. AUR): setzt unter Wayland `QT_QPA_PLATFORM=xcb`, damit der 3D-Viewport (SoQt/Coin3D) startet. Paketierer installieren die echte Binary z. B. nach `/usr/lib/hydracad/cad_desktop` und dieses Skript als `/usr/bin/cad_desktop`.
- **Icon:** `HydraCAD.png` (256×256) – wird von linuxdeploy fürs AppImage und im portable Tarball für Desktop-Einträge genutzt.
- **CI:** `release.yml` baut bei Push auf `main`/`master` (oder workflow_dispatch) neben Windows auch Linux: AppImage (linuxdeploy + Qt-Plugin) und portables Tarball (`hydracad-linux-portable.tar.gz`). Beides wird als Artifact hochgeladen; kein automatischer Upload ins GitHub-Release.
