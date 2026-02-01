# Hydra CAD – Auto-Update (GitHub Releases)

## Übersicht

Die App prüft auf **GitHub Releases** als einzige Quelle für Updates. Ein neuer Git-Tag `vX.Y.Z` löst die CI aus, die ein Release mit Assets (Installer, Portable, update.json, SHA256SUMS) erstellt.

## Verhalten in der App

- **Settings → Check for Updates…**: Ruft die GitHub-API `releases/latest` auf, vergleicht die Version (Semver inkl. `v`-Prefix) mit der installierten Version. Pre-Releases (z. B. `v3.0.16-beta`) gelten nicht als neuer als die stabile Version `v3.0.16`.
- **Update verfügbar**: Dialog mit Version und Changelog; „Jetzt installieren“ startet den Download (z. B. `HydraCADSetup.exe` unter Windows), optional SHA256-Prüfung (falls `update.json`-Asset vorhanden), dann Start des Installers und Beenden der App.
- **Kein Update**: Statusleiste „No updates available“ oder konkrete Fehlermeldung (z. B. Rate-Limit, keine Verbindung).

## Technische Details

### API & Endpunkte

- **Releases**: `GET https://api.github.com/repos/drixber/CAD/releases/latest`
- **Headers**: `Accept: application/vnd.github.v3+json`, `User-Agent: HydraCAD-UpdateCheck`
- **Asset-URLs**: Direktdownload z. B. `https://github.com/drixber/CAD/releases/download/vX.Y.Z/HydraCADSetup.exe`
- **update.json**: Wird von der CI pro Release erzeugt und als Release-Asset hochgeladen. Enthält `version`, `notes_url`, `assets[]` mit `name`, `url`, `size`, `sha256` (Hex). Die App lädt dieses Asset optional und nutzt die SHA256-Werte zur Verifikation nach dem Download.

### Asset-Namen (plattformspezifisch)

| Plattform | Bevorzugter Asset-Name |
|-----------|------------------------|
| Windows   | `HydraCADSetup.exe`, Fallback `app-windows.zip` |
| Linux     | `hydracad-linux-portable.tar.gz` |
| macOS     | `HydraCAD-macos.zip` |

### Semver & Pre-Release

- Tags: `vX.Y.Z` oder `X.Y.Z`; Suffix wie `-beta`, `-rc.1` wird für den Vergleich abgeschnitten.
- Pre-Release: Wenn die neueste Version dieselbe X.Y.Z hat wie die aktuelle, aber einen Suffix (z. B. `v3.0.16-beta`), wird **kein** Update angezeigt (Pre-Release gilt nicht als neuer als Stable).

### Checksum (SHA256)

- Wenn das Release ein **update.json**-Asset hat, lädt die App es und liest `sha256` für den gewählten Asset-Namen. Nach dem Download wird die Datei mit Qt `QCryptographicHash::Sha256` geprüft.
- Wenn **kein** update.json oder kein Eintrag für den Asset vorhanden ist: Download wird nicht verifiziert; die App zeigt ggf. einen Hinweis und startet den Installer trotzdem („best effort“).
- Bei **Hash-Mismatch**: Download wird verworfen, Fehlermeldung, kein Start des Installers.

### Fehlerbehandlung

- **Keine Verbindung**: Status „No connection. Check your network.“
- **GitHub API Rate-Limit (403)**: „GitHub API rate limit exceeded. Please try again later.“
- **Release/Repo nicht gefunden**: „Release or repository not found.“
- **Asset nicht gefunden**: Sinnvolle Meldung, ggf. Öffnen der Release-Seite zum manuellen Download.

## Release-Prozess (Dev)

1. **Tag setzen und pushen**: `git tag vX.Y.Z && git push origin vX.Y.Z`
2. **Workflow „Release bei Tag-Push erstellen“**: Erstellt das GitHub-Release für den Tag.
3. **Workflow „Release – Assets anhängen“**: Baut Windows (Installer + Portable), Linux (Tarball), optional macOS; erzeugt **update.json** und **SHA256SUMS.txt**; lädt alle Assets an das Release hoch.
4. **Workflow „AUR – Update“**: Bei `release: published` wird automatisch ins AUR (hydracad) gepusht, sofern das Secret **AUR_SSH_PRIVATE_KEY** gesetzt ist. Siehe **docs/RELEASE_STEPS.md**.

### CI-Asset-Namen (konstant)

- `HydraCADSetup.exe`
- `app-windows.zip`
- `hydracad-linux-portable.tar.gz`
- `HydraCAD-macos.zip`
- `update.json`
- `SHA256SUMS.txt`

### Secrets (optional)

- **WINDOWS_PFX_BASE64** / **WINDOWS_PFX_PASSWORD**: Nur für Code-Signing des Installers und der EXE. Ohne diese läuft der Build weiter, nur ohne Signatur.

## Debugging

- **Logs**: Settings → Diagnostics → Show Startup Log / Last Crash Log; oder Log-Ordner über „Open Logs Folder“.
- **Typische Ursachen**:  
  - Rate-Limit: Weniger oft prüfen oder mit Token (nicht in der App, nur für manuelle API-Tests).  
  - Kein Asset für die Plattform: Release muss die passenden Dateien enthalten (Windows-Build muss erfolgreich sein).  
  - Alte Version wird angezeigt: Sicherstellen, dass `APP_VERSION` beim Build aus dem Tag gesetzt wird (`-DAPP_VERSION=vX.Y.Z` in CI).

## Update lokal testen

- **„Kein Update“**: Aktuelle App-Version mit `CAD_APP_VERSION` bzw. `APP_VERSION` auf einen hohen Wert setzen (z. B. `v99.0.0`), dann „Check for Updates“ → erwartete Meldung „No updates available“.
- **„Update verfügbar“**: App mit niedriger Version (z. B. `v0.0.1`) bauen, echten Tag (z. B. `v3.0.16`) auf GitHub haben → Dialog mit neuer Version.
- **Hash-Mismatch**: update.json mit falschem SHA256 für einen Asset (z. B. manuell geändert) oder heruntergeladene Datei beschädigt → nach Download sollte „Checksum verification failed“ erscheinen.

## Referenz: update.json-Format

```json
{
  "version": "v3.0.16",
  "notes_url": "https://github.com/drixber/CAD/releases/tag/v3.0.16",
  "assets": [
    { "name": "HydraCADSetup.exe", "url": "https://.../HydraCADSetup.exe", "size": 12345678, "sha256": "abc123..." },
    { "name": "app-windows.zip", "url": "https://.../app-windows.zip", "size": 12345678, "sha256": "def456..." }
  ]
}
```

- **sha256**: Hex-String (64 Zeichen), Kleinbuchstaben üblich.
- **url**: Direkt-Download-URL des Release-Assets.

---

## Testplan

### Unit-Tests (lokal)

- **Semver-Vergleich** (`tests/core/UpdateCheckerTest.cpp`):
  - `isVersionNewerThan`: v2.0.0 > v1.0.0, v1.0.1 > v1.0.0, v1.0.0 == v1.0.0, v3.0.16-beta nicht neuer als v3.0.16.
  - `parseGithubReleaseResponse`: Leere Antwort → Fehler; Body mit "rate limit" → Fehlermeldung mit "rate limit".
  - `parseUpdateJsonChecksums`: Leerer Input → leere Map; gültiges JSON mit assets mit name/sha256 → Map name→sha256.
  - `getAssetUrlFromReleaseJson`: Release-JSON mit asset "update.json" → URL enthält update.json.
- **UpdateService** (`tests/app/UpdateServiceTest.cpp`): Bestehende Tests (CheckForUpdates, GetLatestUpdateInfo, DownloadUpdate, CurrentVersion); optional mit gesetztem `update_server_url_` (alter Server-Pfad) oder ohne Netzwerk-Tests.

Ausführung (Beispiel):

```bash
cmake -S . -B build -DCAD_BUILD_TESTS=ON -DCAD_USE_QT=ON
cmake --build build
./build/update_checker_test
./build/update_service_test   # ggf. Netzwerk/Server abhängig
```

### Integration (manuell / simuliert)

- **Kein Update**: App mit hoher Version (z. B. APP_VERSION=v99.0.0) bauen → „Check for Updates“ → „No updates available“.
- **Update verfügbar**: App mit niedriger Version (z. B. v0.0.1), echten Release (z. B. v3.0.16) auf GitHub → Dialog mit neuer Version, Download und Installer-Start (Windows).
- **Hash-Mismatch**: Nach Download prüft die App SHA256 (wenn update.json vorhanden); beschädigte Datei oder falscher Eintrag in update.json → „Checksum verification failed“.

### CI

- **Tag vX.Y.Z** (z. B. auf Test-Branch oder Hauptbranch) pushen → Workflow „Release bei Tag-Push erstellen“ erstellt Release → „Release – Assets anhängen“ baut Windows/Linux/macOS, erzeugt **update.json** und **SHA256SUMS.txt**, lädt alle Assets hoch. Prüfung: Release-Seite enthält HydraCADSetup.exe, app-windows.zip, update.json, SHA256SUMS.txt.
