# Hydra CAD – Auth & License API (Backend)

Eigenes Backend für User-Auth und Lizenzen (kein Cloud-Sync).  
Basis: `docs/PLAN_USER_AND_LICENSING.md`, `docs/TODO_USER_BACKEND_LICENSES.md`.

## Voraussetzungen

- Python 3.10+
- `pip install -r requirements.txt`

## Start (Entwicklung)

```bash
cd backend
pip install -r requirements.txt
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

API: http://localhost:8000  
Dokumentation: http://localhost:8000/docs

## Umgebungsvariablen

| Variable | Beschreibung |
|----------|--------------|
| `HYDRACAD_DB_PATH` | Pfad zur SQLite-Datei (Standard: `./hydracad.db`) |
| `HYDRACAD_JWT_SECRET` | Geheimer Schlüssel für JWT (min. 32 Zeichen; Produktion setzen!) |

## Endpoints

- **POST /api/auth/register** – Registrierung (username, email, password) → JWT
- **POST /api/auth/login** – Login (username, password) → JWT
- **POST /api/auth/refresh** – Refresh-Token → neues Access-Token
- **GET /api/auth/me** – Aktueller User (Header: `Authorization: Bearer <access_token>`)
- **POST /api/license/activate** – Lizenzschlüssel aktivieren (Bearer + license_key, machine_id)
- **POST /api/license/check** – Lizenzstatus prüfen (Bearer + machine_id)

## Lizenzschlüssel anlegen

```bash
python admin_create_license.py [free|trial|pro|enterprise] [expires_at]
# Beispiel: python admin_create_license.py pro 2026-12-31T23:59:59Z
```

## Community-API (Makerworld-Style)

- **GET /api/community/feed?sort=new|popular|trending&q=** – Feed (items mit id, title, author, likes, downloads, description)
- **GET /api/community/items/{id}** – Detail inkl. file_url
- **GET /api/community/items/{id}/download** – Datei herunterladen (erhöht downloads)
- **POST /api/community/items** – Upload (multipart: title, description, file; Auth optional)

Upload-Verzeichnis: `HYDRACAD_UPLOAD_DIR` (Standard: `./uploads`).

---

## Phase 5: Tests & Deployment

**Ausführliche Checklisten und Deployment-Anleitung:** [docs/PHASE5_TESTS_DEPLOYMENT.md](../docs/PHASE5_TESTS_DEPLOYMENT.md).

### Tests (automatisiert) – Phase 5.1 & 5.2

Registrierung, Login, Refresh, Logout sowie Lizenz aktivieren/prüfen werden per API-Tests abgedeckt:

```bash
cd backend
pip install -r requirements.txt   # enthält pytest, httpx
python -m pytest tests/ -v
```

- **tests/test_auth.py:** Register, Login, GET /me, Refresh, falsches Passwort, Me ohne Token.
- **tests/test_license.py:** Activate, Check, ungültiger Schlüssel, abgelaufene Lizenz, Auth-Pflicht.

Die Tests nutzen eine temporäre SQLite-Datenbank; keine laufende Server-Instanz nötig.

### Tests (manuell)

1. **Auth:** Backend starten → in App registrieren → einloggen → Token in QSettings prüfen; Refresh (Token ablaufen lassen); Logout → Token weg.
2. **Lizenz:** `python admin_create_license.py pro` → in App „Lizenz aktivieren“ → Schlüssel + Machine-ID → Lizenz prüfen; Offline: API aus → letzte gültige Response.
3. **Community:** Backend starten → in App API-URL setzen → Community-Tab: Feed laden; Upload (Share design) testen; Download → Import in Viewport.

### Deployment (Produktion)

1. **Docker:** Aus Repo-Root: `docker build -t hydracad-api -f backend/Dockerfile .`; dann `docker run -d -p 8000:8000 -v hydracad-data:/data -e HYDRACAD_JWT_SECRET="<secret>" hydracad-api`. Siehe [PHASE5 § 4.3](../docs/PHASE5_TESTS_DEPLOYMENT.md). **systemd:** Beispiel [hydracad-api.service.example](../docs/deployment/hydracad-api.service.example) nach `/etc/systemd/system/` kopieren; User/Verzeichnisse anlegen; `HYDRACAD_JWT_SECRET` setzen; `systemctl enable --now hydracad-api`. **HTTPS:** Reverse-Proxy (nginx/Caddy) mit Let's Encrypt, Beispiel [nginx-api.example.conf](../docs/deployment/nginx-api.example.conf), siehe PHASE5 § 4.4.
2. `HYDRACAD_JWT_SECRET` und ggf. `HYDRACAD_DB_PATH`, `HYDRACAD_UPLOAD_DIR` setzen.
3. In der App: API-URL auf Produktion setzen (Umgebungsvariable `CAD_API_BASE_URL` oder QSettings `auth/api_base_url`). Siehe [INSTALLATION.md](../docs/INSTALLATION.md).

---

## App anbinden

In der App API-Basis-URL setzen (z. B. Umgebungsvariable `CAD_API_BASE_URL` oder `HYDRACAD_API_URL`, oder in den Einstellungen). Dann nutzen Registrierung/Login, „Lizenz aktivieren“ und Community-Feed das Backend.
