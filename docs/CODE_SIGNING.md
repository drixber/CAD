# Code-Signing (Windows)

Optional: Signierung der Windows-Builds (EXE und Installer) für Vertrauen und SmartScreen.

---

## Voraussetzung

- **Signing-Zertifikat** im PFX-Format (z. B. von DigiCert, Sectigo, oder selbst signiert für Tests).
- Die GitHub-Actions-Workflows nutzen **Secrets**, um das Zertifikat und das Passwort bereitzustellen.

---

## GitHub Secrets

Im Repository unter **Settings → Secrets and variables → Actions** folgende Secrets anlegen:

| Secret | Beschreibung |
|--------|--------------|
| `WINDOWS_PFX_BASE64` | PFX-Datei als **Base64**-String (z. B. `certutil -encode cert.pfx cert.b64`, Inhalt von cert.b64 ohne Kopfzeilen). |
| `WINDOWS_PFX_PASSWORD` | Passwort der PFX-Datei. |

Ohne diese Secrets wird der Code-Signing-Schritt im Workflow **übersprungen** (Build läuft trotzdem durch).

---

## Ablauf im Workflow

- Im Workflow **Build Windows (Push)** (`release.yml`):
  - Schritt **Code Sign (optional)** läuft nur, wenn `WINDOWS_PFX_BASE64` gesetzt ist.
  - Es werden signiert:
    - `build\Release\cad_desktop.exe`
    - `installer\HydraCADSetup.exe`
  - Optional kann später auch das Portable-ZIP (`app-windows.zip`) oder darin enthaltene EXE signiert werden.

---

## Lokales Signieren (optional)

Mit **signtool** (Windows SDK) und PFX-Datei:

```cmd
signtool sign /f "path\to\cert.pfx" /p "PASSWORD" /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 "path\to\HydraCADSetup.exe"
```

---

## Hinweise

- **Keine Zertifikate im Repo:** PFX und Passwort nur über Secrets/Umgebungsvariablen bereitstellen.
- **Timestamp:** Der Workflow nutzt einen Timestamp-Server (`/tr`), damit die Signatur auch nach Ablauf des Zertifikats gültig bleibt.
- **SmartScreen:** Für bessere Akzeptanz bei Windows SmartScreen wird ein anerkannter EV- oder OV-Code-Signing-Vertrag empfohlen.
