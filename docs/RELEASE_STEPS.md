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

**Aktueller Stand:** Commit und Tag `v3.0.6` sind lokal erstellt. Sobald du `git push origin main` und `git push origin v3.0.6` ausführst, läuft der Windows-Build; danach kannst du bei Bedarf das Release wie oben anlegen.
