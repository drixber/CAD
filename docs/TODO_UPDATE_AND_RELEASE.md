# Update-Funktion und Release (EXE + GitHub) – komplette Anleitung

Damit die **In-App-Update-Funktion** („Check for Updates“ → Download → Installer starten) und die **GitHub-Releases** mit **HydraCADSetup.exe** zuverlässig funktionieren, muss die folgende Kette stimmen.

**Status:** Phase D (Code) ist umgesetzt. Phasen A/B/C (Workflows, Release erzeugen, Troubleshooting) und E (Prüfung) führt man manuell durch.

---

## 1. Übersicht: Von Tag bis Update in der App

```
Tag pushen (vX.Y.Z)
    → Workflow "Release bei Tag-Push erstellen" erstellt GitHub-Release
    → Workflow "Release – Assets anhängen" wird getriggert (release: published)
    → Windows-Build: HydraCADSetup.exe + app-windows.zip
    → Linux-Build: hydracad-linux-portable.tar.gz
    → Assets werden ans Release gehängt
    → App ruft GitHub API auf → bekommt assetDownloadUrl (z. B. HydraCADSetup.exe)
    → App lädt EXE in Temp herunter → startet Installer → Benutzer schließt Setup ab
```

Wenn **irgendwo** in dieser Kette etwas fehlt (z. B. kein Release, keine EXE am Release, falscher Asset-Name), landet der Benutzer bei „Release-Seite wurde geöffnet“ oder „Kein Download möglich“.

---

## 2. TODO: Release mit EXE möglich machen

### Phase A: Workflows auf GitHub (einmalig / pro Repo)

- [ ] **A1** `.github/workflows/create-release-on-tag.yml` ist auf Branch **main** gepusht.  
  Ohne diesen Workflow erzeugt ein Tag-Push **kein** GitHub-Release.
- [ ] **A2** `.github/workflows/release-attach-assets.yml` ist auf **main** und enthält den **manuellen Trigger** (`workflow_dispatch` mit `tag_name`).  
  So können Assets nachträglich für ein Release gebaut werden, wenn der automatische Lauf fehlschlägt.
- [ ] **A3** Beim ersten Mal: Für bereits existierende Tags **ohne** Release einmal manuell ein Release anlegen (Releases → Draft a new release → Tag wählen → Publish).  
  Danach für neue Versionen nur noch Tag pushen.

### Phase B: Neues Release mit EXE erzeugen (pro Version)

- [ ] **B1** Version im Projekt setzen (z. B. in `CMakeLists.txt` / `installer/hydracad.nsi` / CHANGELOG), committen, auf **main** pushen.
- [ ] **B2** Tag setzen und pushen:  
  `git tag vX.Y.Z`  
  `git push origin vX.Y.Z`
- [ ] **B3** Prüfen: Unter **Releases** erscheint „Hydra CAD X.Y.Z“ als Latest.
- [ ] **B4** Prüfen: Unter dem Release stehen **Assets**: z. B. **HydraCADSetup.exe**, **app-windows.zip**, **hydracad-linux-portable.tar.gz**.  
  Wenn nur „Source code (zip/tar.gz)“ vorhanden ist → Phase C (Troubleshooting).

### Phase C: Wenn am Release keine EXE hängt (Troubleshooting)

- [ ] **C1** **Actions** öffnen → Workflow **„Release – Assets anhängen“** → letzten Lauf für dieses Release prüfen.  
  Wenn **fehlgeschlagen**: **Re-run all jobs**.
- [ ] **C2** Wenn kein Lauf existiert (Release wurde manuell erstellt, bevor Workflow da war):  
  **Actions** → **Release – Assets anhängen** → **Run workflow** → Branch **main**, Eingabe **tag_name**: `vX.Y.Z` → **Run workflow**.  
  Warten, bis Windows- und Linux-Jobs durch sind; danach sollten EXE/ZIP/Tarball am Release erscheinen.
- [ ] **C3** Asset-Namen müssen **genau** so heißen, wie die App sie erwartet:  
  **HydraCADSetup.exe**, **app-windows.zip**, **hydracad-linux-portable.tar.gz**.  
  Abweichungen (z. B. HydraCAD-Setup-3.0.11.exe) führen dazu, dass die App keine direkte Download-URL findet und die Release-Seite öffnet.

### Phase D: In-App-Update-Funktion (Code-Seite) – **umgesetzt**

- [x] **D1** Update-Check ruft GitHub API auf:  
  `GET https://api.github.com/repos/drixber/CAD/releases/latest`  
  (mit Qt Network oder curl, je nach Build).
- [x] **D2** Aus der API-Antwort wird die **direkte Download-URL** eines Assets ermittelt (`browser_download_url`), bevorzugt **HydraCADSetup.exe**, Fallback **app-windows.zip** / **hydracad-linux-portable.tar.gz**.
- [x] **D3** Wenn eine solche URL gefunden wird (typisch: `.../releases/download/...`), lädt die App die Datei in ein **benutzerbeschreibbares Verzeichnis** (Temp/HydraCAD) mit dem **richtigen Dateinamen** (aus der URL, z. B. HydraCADSetup.exe).
- [x] **D4** Unter **Windows**: Nur wenn die heruntergeladene Datei eine **.exe** ist, wird der Installer automatisch gestartet; sonst Hinweis „Update-Paket heruntergeladen nach … Bitte manuell entpacken/ausführen.“
- [x] **D5** **Arbeitsverzeichnis** der App beim Start auf das Installationsverzeichnis setzen, damit nach dem Update kein „schwarzes Fenster“ / Admin-Pflicht entsteht.
- [x] **D6** **Changelog** im Update-Dialog aus dem Release-**body** der API füllen, Fallback: „Siehe Release-Seite“.

### Phase E: Regelmäßige Prüfung (Optional)

- [ ] **E1** Nach jedem neuen Release: In der **älteren** installierten Version „Check for Updates“ ausführen → Update-Dialog mit neuer Version und Changelog → „Install Update“ → Download → Installer startet.
- [ ] **E2** Wenn stattdessen „Die Release-Seite wurde geöffnet“ erscheint:  
  Prüfen, ob am **neuesten** Release die Assets **HydraCADSetup.exe** / **app-windows.zip** vorhanden sind und exakt so heißen; ggf. Phase C durchgehen.

---

## 3. Kurz-Checkliste: „EXE und Release funktionieren“

1. **Tag** für neue Version pushen (z. B. `v3.0.12`).
2. **Release** erscheint automatisch (dank create-release-on-tag) oder wurde manuell erstellt.
3. **Assets** erscheinen am Release (dank release-attach-assets): **HydraCADSetup.exe**, **app-windows.zip**, ggf. **hydracad-linux-portable.tar.gz**.
4. **App** (ältere Version): „Check for Updates“ → erkennt neue Version → zeigt Changelog → „Install Update“ → lädt **HydraCADSetup.exe** in Temp → startet Installer.
5. Benutzer schließt Setup ab → App neu starten → neue Version läuft.

Wenn Schritt 3 fehlt (keine EXE am Release), Phase C ausführen.  
Wenn Schritt 4 fehlschlägt („Release-Seite geöffnet“), Phase C und D prüfen (Asset-Namen, API, Download-URL, Dateiname beim Speichern).

---

## 4. Referenzen

- **Workflows:** `.github/workflows/create-release-on-tag.yml`, `.github/workflows/release-attach-assets.yml`
- **Update-Logik:** `src/app/AppController.cpp` (Update-Check, installUpdate), `src/core/updates/UpdateChecker.cpp` (API-Parsing, Asset-URL)
- **Allgemeine Installation/Release:** [INSTALLATION.md](INSTALLATION.md)
