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

## 2. GitHub Release anlegen (optional)

Wenn du auf GitHub eine **Release-Seite** mit Beschreibung und Dateien haben willst:

1. Auf GitHub: **Releases** → **Draft a new release**
2. **Choose tag:** `v3.0.6` auswählen
3. **Release title:** z. B. `v3.0.6`
4. **Describe:** Changelog/Highlights eintragen (z. B. aus CHANGELOG.md)
5. **Assets:** Aus dem abgeschlossenen Workflow-Lauf die Artifacts **HydraCAD-Windows-Installer** und **HydraCAD-Windows-Portable** herunterladen und hier anhängen (oder einen Workflow nutzen, der bei Tag-Push automatisch ein Release mit Assets erstellt)
6. **Publish release**

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
