# Release-Schritte (manuell)

Nach dem lokalen Commit und Tag musst du **Push und Release** mit deinen GitHub-Zugangsdaten ausführen.

## 1. Branch und Tag pushen

```bash
git push origin main
git push origin v3.0.6
```

(Falls du SSH nutzt und `origin` auf `git@github.com:...` zeigt, reicht das. Bei HTTPS ggf. Token oder Credential-Helper einrichten.)

- **Push zu main** löst den Workflow „Build Windows (Push)“ aus → Installer und Portable-ZIP werden gebaut und als Artifacts hochgeladen (7 Tage).
- **Push des Tags** markiert den Stand als Release-Version v3.0.6.

## 2. GitHub Release anlegen – EXE/ZIP automatisch anhängen

Damit die **neueste Release-Seite** wieder **HydraCADSetup.exe** und **app-windows.zip** (wie bei v3.0.4) anzeigt:

1. **Tag pushen** (falls noch nicht geschehen): `git push origin v3.0.7`
2. Auf GitHub: **Releases** → **Draft a new release** (oder **Create release from tag** beim Tag)
3. **Choose tag:** z. B. `v3.0.7` auswählen
4. **Release title:** z. B. `Hydra CAD 3.0.7`
5. **Describe:** Changelog/Highlights (z. B. aus CHANGELOG.md), Installation (Download `HydraCADSetup.exe`, Doppelklick, Assistent folgen), Features
6. **Publish release** klicken

**Danach:** Der Workflow **„Release – Assets anhängen“** (`release-attach-assets.yml`) startet automatisch. Er baut Windows (Installer + Portable-ZIP) und Linux (Tarball) und **hängt die Dateien an dieses Release** an. Nach wenigen Minuten erscheinen unter dem Release:

- **HydraCADSetup.exe**
- **app-windows.zip**
- **hydracad-linux-portable.tar.gz**

Du musst die EXE/ZIP nicht mehr manuell aus den Workflow-Artifacts holen – sie landen direkt auf der Release-Seite.

---

## 401 / „Authentifizierung fehlgeschlagen“ (HTTPS)

Wenn `git push origin main` oder `git push origin v3.0.6` mit **401** oder **Missing or invalid credentials** abbricht:

- **Ursache:** `origin` nutzt `https://github.com/drixber/CAD.git`. GitHub erlaubt für HTTPS **kein Passwort** mehr, nur noch einen **Personal Access Token (PAT)** oder **SSH**.

### Option A: Auf SSH umstellen (empfohlen, wenn du SSH-Keys hast)

```bash
cd /home/herd/GitHub/CAD
git remote set-url origin git@github.com:drixber/CAD.git
git push origin main
git push origin v3.0.6
```

Vorher prüfen: `ls -la ~/.ssh` – es sollte z. B. `id_ed25519` / `id_rsa` geben und der Key bei GitHub unter **Settings → SSH and GPG keys** eingetragen sein. Keinen Key? Erzeugen mit `ssh-keygen -t ed25519 -C "deine@email"` und den öffentlichen Key bei GitHub hinzufügen.

### Option B: HTTPS mit Personal Access Token (PAT)

1. Auf GitHub: **Settings → Developer settings → Personal access tokens → Tokens (classic)** → **Generate new token**. Scope mindestens **repo**.
2. Token kopieren (nur einmal sichtbar).
3. Beim nächsten `git push` Nutzername (dein GitHub-Username) und als Passwort den **Token** eingeben.
4. Oder URL mit Token setzen (Token liegt dann in der Config – nur auf sicherem Rechner):

   ```bash
   git remote set-url origin https://DEIN_GITHUB_USERNAME:DEIN_TOKEN@github.com/drixber/CAD.git
   ```

   Danach `git push origin main` und `git push origin v3.0.6`.

---

**Aktueller Stand:** Commit und Tag `v3.0.6` sind lokal erstellt. Sobald du mit gültigen Zugangsdaten (SSH oder PAT) `git push origin main` und `git push origin v3.0.6` ausführst, läuft der Windows-Build; danach kannst du bei Bedarf das Release wie oben anlegen.
