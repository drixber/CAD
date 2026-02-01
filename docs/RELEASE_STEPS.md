# Release – Schritte (alle Versionen, inkl. AUR)

So erstellst du ein neues Release für **Windows, Linux, macOS** und aktualisierst **AUR** automatisch.

## Ablauf (empfohlen)

### 1. Alles auf main committen und pushen

```bash
cd /home/herd/GitHub/CAD
git status
git add -A
git commit -m "Release-Vorbereitung vX.Y.Z"   # oder konkrete Message
git push origin main
```

### 2. Neuen Tag setzen und pushen

```bash
# Beispiel: v3.0.21
git tag v3.0.21
git push origin v3.0.21
```

- Der Workflow **„Release bei Tag-Push erstellen“** läuft automatisch und erstellt das GitHub-Release für den Tag (mit generierten Release-Notes).
- Durch **Publish** des Releases wird das Event `release: published` ausgelöst.

### 3. Was danach automatisch läuft

| Workflow | Trigger | Aktion |
|----------|---------|--------|
| **Release – Assets anhängen** | `release: published` (oder manuell) | Baut Windows (Installer + Portable), Linux (Tarball), macOS; hängt HydraCADSetup.exe, app-windows.zip, hydracad-linux-portable.tar.gz, HydraCAD-macos.zip, update.json, SHA256SUMS.txt ans Release. |
| **AUR – Update** | `release: published` (oder manuell) | Erzeugt PKGBUILD + .SRCINFO für die Release-Version und pusht ins AUR (hydracad). |

**Voraussetzung für AUR:** Repository-Secret **AUR_SSH_PRIVATE_KEY** (privater SSH-Key für deinen AUR-Account) unter **Settings → Secrets and variables → Actions** anlegen. Ohne Secret überspringt der AUR-Workflow das Pushen (ohne Fehler).

### 4. Manuell nachziehen (falls nötig)

- **Assets fehlen am Release?**  
  **Actions** → **Release – Assets anhängen** → **Run workflow** → **tag_name:** z. B. `v3.0.21` → Run.

- **AUR nicht aktualisiert?**  
  **Actions** → **AUR – Update** → **Run workflow** → **tag_name:** z. B. `v3.0.21` → Run.  
  Oder lokal: `./packaging/arch/aur-upload.sh 3.0.21`

### 5. Prüfen

- **Release-Seite:** https://github.com/drixber/CAD/releases – HydraCADSetup.exe, app-windows.zip, hydracad-linux-portable.tar.gz, HydraCAD-macos.zip, update.json, SHA256SUMS.txt.
- **AUR:** https://aur.archlinux.org/packages/hydracad – Version und `hydracad`-Befehl (Symlink).

---

## Kurz-Checkliste

1. [ ] Alles auf main committen & pushen  
2. [ ] `git tag vX.Y.Z && git push origin vX.Y.Z`  
3. [ ] Release wird erstellt (Tag-Push-Workflow)  
4. [ ] „Release – Assets anhängen“ läuft (automatisch oder manuell)  
5. [ ] „AUR – Update“ läuft (automatisch wenn AUR_SSH_PRIVATE_KEY gesetzt; sonst manuell)  
6. [ ] Release-Seite und AUR prüfen  

---

## 401 / „Authentifizierung fehlgeschlagen“ (HTTPS)

Wenn `git push origin main` oder `git push origin vX.Y.Z` mit **401** oder **Missing or invalid credentials** abbricht:

- **Ursache:** `origin` nutzt HTTPS; GitHub erlaubt für HTTPS **kein Passwort** mehr, nur noch einen **Personal Access Token (PAT)** oder **SSH**.

### Option A: Auf SSH umstellen (empfohlen)

```bash
git remote set-url origin git@github.com:drixber/CAD.git
git push origin main
git push origin vX.Y.Z
```

SSH-Key bei GitHub unter **Settings → SSH and GPG keys** eintragen.

### Option B: HTTPS mit Personal Access Token (PAT)

1. GitHub: **Settings → Developer settings → Personal access tokens → Tokens (classic)** → **Generate new token** (Scope mind. **repo**).
2. Beim nächsten `git push`: Nutzername + **Token** als Passwort. Oder URL mit Token setzen:  
   `git remote set-url origin https://DEIN_USER:DEIN_TOKEN@github.com/drixber/CAD.git`
