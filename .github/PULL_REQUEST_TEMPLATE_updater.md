# Auto-Update (GitHub Releases)

## Zusammenfassung

- **GitHub Releases als einzige Quelle**: Die App prüft nur noch die GitHub-API `repos/drixber/CAD/releases/latest`. Ein neuer Tag `vX.Y.Z` löst die CI aus, die ein Release mit Assets (Installer, Portable, update.json, SHA256SUMS) erstellt.
- **UpdateChecker (core)**: Semver inkl. Pre-Release (z. B. v3.0.16-beta nicht neuer als v3.0.16), Erkennung von Rate-Limit/Not Found, `getAssetUrlFromReleaseJson`, `parseUpdateJsonChecksums`, `isVersionNewerThan`.
- **UpdateService (app)**: `checkForUpdatesFromGitHub(owner, repo)`, `getLastError()`, optionales Laden von update.json für SHA256, `verifyDownloadedUpdate()`, SHA256-Verifikation per Qt.
- **AppController**: „Check for Updates“ nur noch GitHub-Flow; Fehler über `getLastError()`; nach Download Verifikation (bei vorhandenem Checksum), sonst Hinweis.
- **CI**: Step „Checksums und update.json erzeugen“ baut update.json (version, notes_url, assets mit name/url/size/sha256) und SHA256SUMS.txt; beide werden mit den übrigen Assets hochgeladen.
- **Dokumentation**: docs/UPDATES.md (Verhalten, API, Assets, Semver, Checksum, Fehler, Release-Prozess, Testplan); README verweist auf docs/UPDATES.md.
- **Tests**: tests/core/UpdateCheckerTest.cpp (Semver, Pre-Release, leere Antwort, Rate-Limit, parseUpdateJsonChecksums, getAssetUrlFromReleaseJson).

## Geänderte / neue Dateien

| Datei | Änderung |
|-------|----------|
| src/core/updates/UpdateChecker.h, .cpp | Semver Pre-Release, Rate-Limit/Not-Found, getAssetUrlFromReleaseJson, parseUpdateJsonChecksums, isVersionNewerThan |
| src/app/UpdateService.h, .cpp | checkForUpdatesFromGitHub, getLastError, verifyDownloadedUpdate, SHA256 (Qt), optional update.json |
| src/app/AppController.cpp | Nur GitHub-Flow, getLastError, Verifikation nach Download |
| .github/workflows/release-attach-assets.yml | update.json + SHA256SUMS erzeugen und hochladen |
| docs/UPDATES.md | Neu: Technik, Verhalten, Testplan |
| README.md | Verweis auf docs/UPDATES.md |
| tests/core/UpdateCheckerTest.cpp | Neu; tests/CMakeLists.txt: update_checker_test |

## Setup & Benutzung

**Anwender**: Settings → Check for Updates… → bei neuer Version Dialog → Jetzt installieren → Download → optional SHA256-Prüfung → Installer starten, App beendet sich.

**Entwickler**: Tag pushen (z. B. `git tag v3.0.17 && git push origin v3.0.17`) → CI erstellt Release und hängt alle Assets inkl. update.json und SHA256SUMS.txt an.

## Testplan

- **Unit**: `./build/tests/update_checker_test` (Semver, Rate-Limit, update.json, Asset-URL).
- **Integration**: App mit niedriger Version → Check for Updates → Update-Dialog, Download, Verifikation, Installer-Start.
- **CI**: Tag-Push → release-attach-assets.yml → Release enthält HydraCADSetup.exe, app-windows.zip, update.json, SHA256SUMS.txt.
- **Edgecases**: Kein Netz → getLastError; Hash-Mismatch → Abbruch mit Hinweis.

## Checkliste

- [ ] Lokal: `cmake -S . -B build -DCAD_BUILD_TESTS=ON -DCAD_USE_QT=ON && cmake --build build && ./build/tests/update_checker_test`
- [ ] CI: Push Branch → Build + Tests grün
- [ ] E2E: Tag vX.Y.Z pushen → Release mit update.json + SHA256SUMS
- [ ] App: Ältere Version → Check for Updates → Update findet neue Version, Download, Verify, Installer-Start, App beendet sich
