# Kompletter Rundumflug bis zum neuen Release

Checkliste von **aktueller Codebasis** bis **neues Release** (z. B. v3.0.12) inkl. GitHub Release, Assets und AUR.

---

## 1. Version und Changelog setzen

- [ ] **1.1** `CMakeLists.txt`: `project(InventorStyleCAD VERSION X.Y.Z ...)` auf neue Version (z. B. `3.0.15`) setzen.
- [ ] **1.2** `installer/hydracad.nsi`: Fallback-Version in den `!ifndef`-Blöcken anpassen (optional; CI übergibt Version aus Tag automatisch).
- [ ] **1.3** `CHANGELOG.md`: Neue Sektion `## [X.Y.Z] - JJJJ-MM-TT` mit Änderungen (Added/Changed/Fixed) ergänzen.
- [ ] **1.4** `packaging/arch/aur-upload.sh`: `PKGVER="X.Y.Z"` setzen.
- [ ] **1.5** `packaging/arch/PKGBUILD`: `pkgver=X.Y.Z` setzen (falls du PKGBUILD manuell nutzt).

---

## 2. Lokal committen und pushen

- [ ] **2.1** Alle Änderungen committen:  
  `git add -A && git status` → prüfen → `git commit -m "Release vX.Y.Z: Changelog und Versionen"`
- [ ] **2.2** Branch **main** pushen:  
  `git push origin main`  
  (Bei 401: SSH oder PAT nutzen, siehe [RELEASE_STEPS.md](RELEASE_STEPS.md).)

---

## 3. Tag setzen und Release auslösen

- [ ] **3.1** Tag erstellen und pushen:  
  `git tag vX.Y.Z`  
  `git push origin vX.Y.Z`
- [ ] **3.2** Workflow **„Release bei Tag-Push erstellen“** erstellt automatisch das GitHub-Release (sofern auf main).
- [ ] **3.3** Durch das Erstellen des Releases wird das Event **release: published** ausgelöst; der Workflow **„Release – Assets anhängen“** startet automatisch und baut:
  - **HydraCADSetup.exe**, **app-windows.zip** (Windows)
  - **hydracad-linux-portable.tar.gz** (Linux)
  - **HydraCAD-macos.zip** (macOS, falls Build erfolgreich)  
  Diese Dateien werden bei **jedem** Release automatisch erzeugt und ans Release gehängt (Upload-Job hängt nur von Windows + Linux ab, damit EXE/ZIP/Tarball auch bei macOS-Fehler ankommen).

---

## 4. Release prüfen (GitHub)

- [ ] **4.1** Unter **Releases** erscheint „Hydra CAD X.Y.Z“ (Latest).
- [ ] **4.2** Unter dem Release stehen die **Assets**: **HydraCADSetup.exe**, **app-windows.zip**, **hydracad-linux-portable.tar.gz**.
- [ ] **4.3** Wenn **keine EXE/ZIP** am Release: [TODO_UPDATE_AND_RELEASE.md](TODO_UPDATE_AND_RELEASE.md) Phase C (Troubleshooting) durchgehen – z. B. **Release – Assets anhängen** → **Run workflow** → `tag_name: vX.Y.Z`.

---

## 5. In-App-Update testen (optional)

- [ ] **5.1** Ältere installierte Version starten → **Check for Updates**.
- [ ] **5.2** Update-Dialog zeigt neue Version und Changelog (aus Release-Body).
- [ ] **5.3** „Install Update“ → Download → unter Windows: Installer startet; sonst Hinweis „Paket heruntergeladen, bitte manuell entpacken/ausführen“.

---

## 6. AUR-Update (Arch Linux)

- [ ] **6.1** `PKGVER` in `packaging/arch/aur-upload.sh` ist bereits auf X.Y.Z gesetzt (Schritt 1.4).
- [ ] **6.2** Von **Projektroot** aus:  
  `./packaging/arch/aur-upload.sh`  
  (Voraussetzung: Arch, AUR-Account, SSH-Key bei AUR; siehe [INSTALL_ARCH_PACMAN_YAY.md](INSTALL_ARCH_PACMAN_YAY.md).)
- [ ] **6.3** AUR-Paket **hydracad** zeigt danach Version X.Y.Z.

---

## 7. Kurzreferenz

| Schritt | Aktion |
|--------|--------|
| Version | CMakeLists, hydracad.nsi, CHANGELOG, aur-upload.sh, ggf. PKGBUILD |
| Commit | `git commit -m "Release vX.Y.Z"` |
| Push | `git push origin main` |
| Tag | `git tag vX.Y.Z` → `git push origin vX.Y.Z` |
| Prüfung | Releases → Assets (EXE, ZIP, Tarball) |
| AUR | `./packaging/arch/aur-upload.sh` (vom Repo-Root) |

Weitere Details: [TODO_UPDATE_AND_RELEASE.md](TODO_UPDATE_AND_RELEASE.md), [RELEASE_STEPS.md](RELEASE_STEPS.md), [INSTALLATION.md](INSTALLATION.md).
