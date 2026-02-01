# Phase 5: Tests & Deployment (User-Backend & Lizenzen)

Konkrete Checklisten und Schritte für Tests sowie Produktions-Deployment.  
Basis: [TODO_USER_BACKEND_LICENSES.md](TODO_USER_BACKEND_LICENSES.md), [backend/README.md](../backend/README.md).

---

## 1. Tests: Auth (Registrierung, Login, Refresh, Logout)

**Voraussetzung:** Backend läuft (`cd backend && uvicorn main:app --reload --port 8000`), App mit API-URL (z. B. `CAD_API_BASE_URL=http://localhost:8000` oder in Einstellungen).

| # | Schritt | Erwartung | ☐ |
|---|---------|-----------|---|
| 1 | Backend starten: `cd backend && pip install -r requirements.txt && uvicorn main:app --reload --port 8000` | Server läuft, http://localhost:8000/docs erreichbar | ☐ |
| 2 | In der App: Einstellungen → API-Basis-URL auf `http://localhost:8000` setzen (falls nicht per Umgebungsvariable) | URL gespeichert | ☐ |
| 3 | In der App: Registrierung (neuer Benutzer: Username, E-Mail, Passwort) | Erfolg, ggf. „Angemeldet“ oder Login-Screen | ☐ |
| 4 | Einloggen mit neuem Account | Erfolg, Benutzer ist eingeloggt (z. B. Username in UI sichtbar) | ☐ |
| 5 | Token in QSettings prüfen (optional: Einstellungen/Diagnostics oder Registry/plist: Schlüssel für `auth/access_token` o. ä.) | Token vorhanden (oder nur in Memory bei „Angemeldet bleiben“ aus) | ☐ |
| 6 | Refresh testen: Access-Token ablaufen lassen (z. B. 60 Min warten) oder Backend kurz neu starten und alten Token nutzen; App erneut nutzen (z. B. Lizenz prüfen oder Community-Feed) | App holt neues Access-Token via Refresh; keine Abmeldung | ☐ |
| 7 | Logout in der App ausführen | Token lokal weg; bei erneutem geschützten Aufruf Login nötig | ☐ |
| 8 | Mit falschem Passwort einloggen | Fehlermeldung, kein Token | ☐ |

**Optional (automatisierte Tests):**  
Für API-Tests ohne GUI: z. B. `pytest` + `httpx`; `POST /api/auth/register` → `POST /api/auth/login` → Response enthält `access_token`; `GET /api/auth/me` mit Bearer → 200; `POST /api/auth/refresh` mit `refresh_token` → neues Access-Token.

---

## 2. Tests: Lizenz (Aktivieren, Prüfen, Offline)

**Voraussetzung:** Backend läuft, User eingeloggt (oder Lizenz-Endpoints mit gültigem Bearer-Token aufrufbar).

| # | Schritt | Erwartung | ☐ |
|---|---------|-----------|---|
| 1 | Lizenz anlegen: `cd backend && python admin_create_license.py pro` (oder `trial`, `enterprise`; optional `2026-12-31T23:59:59Z` für Ablauf) | Ausgabe mit Lizenzschlüssel (Key) | ☐ |
| 2 | In der App: Einstellungen → „Lizenz aktivieren“ (oder vergleichbar) öffnen | Dialog mit Feldern Lizenzschlüssel und ggf. Machine-ID | ☐ |
| 3 | Lizenzschlüssel (aus Schritt 1) eintragen; Machine-ID ggf. automatisch befüllt → Aktivieren | Erfolgsmeldung, Lizenzstatus „aktiv“ o. ä. | ☐ |
| 4 | Lizenz prüfen: App neu starten oder „Lizenz prüfen“ manuell auslösen | Status bleibt gültig (z. B. Pro/Trial), kein Fehler | ☐ |
| 5 | Zweite Maschine (andere Machine-ID): gleichen Schlüssel aktivieren (falls Lizenz mehrere Maschinen erlaubt) | Entweder Erfolg (bei Mehrplatz) oder Fehler „max. Maschinen erreicht“ | ☐ |
| 6 | Offline: Backend stoppen; in App Aktion auslösen, die Lizenz prüft (z. B. Start oder manuell „Prüfen“) | App nutzt Cache (z. B. 24 h); letzte gültige Response, ggf. Hinweis „Offline“; keine sofortige Sperre | ☐ |
| 7 | Abgelaufene Lizenz (falls `expires_at` gesetzt): Nach Ablaufdatum prüfen | Status „abgelaufen“ oder Herabstufung; Feature-Einschränkung laut Plan | ☐ |

---

## 3. Tests: Community (Feed, Download, Upload)

| # | Schritt | Erwartung | ☐ |
|---|---------|-----------|---|
| 1 | Backend starten; in App API-URL gesetzt; Community-Tab/Dock öffnen | Feed wird geladen (Karten mit Thumbnail, Titel, Autor, Stats oder Platzhalter) | ☐ |
| 2 | Auf „View“/Detail einer Karte klicken | Detail-Ansicht mit Beschreibung, ggf. Kommentare, „Download & Open“ | ☐ |
| 3 | „Download & Open“ klicken | Datei wird heruntergeladen und in den Viewport importiert (oder Fehlermeldung bei fehlendem Backend/Datei) | ☐ |
| 4 | Upload („Share design“ o. ä.): Datei + Titel/Beschreibung hochladen (eingeloggt oder optional) | Erfolg, neuer Eintrag im Feed sichtbar (oder nach Reload) | ☐ |

---

## 4. Deployment: Server (Produktion)

### 4.1 Umgebungsvariablen (Produktion)

| Variable | Beschreibung | Beispiel (Produktion) |
|----------|--------------|------------------------|
| `HYDRACAD_DB_PATH` | Pfad zur SQLite-Datei | `/var/lib/hydracad/hydracad.db` |
| `HYDRACAD_JWT_SECRET` | Geheimer Schlüssel für JWT (min. 32 Zeichen) | Starker Zufallswert, geheim halten |
| `HYDRACAD_UPLOAD_DIR` | Verzeichnis für Community-Uploads | `/var/lib/hydracad/uploads` |

### 4.2 systemd (Beispiel)

1. **Service-Datei** anlegen (z. B. `/etc/systemd/system/hydracad-api.service`):

```ini
[Unit]
Description=Hydra CAD API (Auth, License, Community)
After=network.target

[Service]
Type=simple
User=hydracad
Group=hydracad
WorkingDirectory=/opt/hydracad/backend
Environment="HYDRACAD_DB_PATH=/var/lib/hydracad/hydracad.db"
Environment="HYDRACAD_JWT_SECRET=<starkes-geheimes-token>"
Environment="HYDRACAD_UPLOAD_DIR=/var/lib/hydracad/uploads"
ExecStart=/usr/bin/python3 -m uvicorn main:app --host 0.0.0.0 --port 8000
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

2. **Verzeichnisse und Rechte:**

```bash
sudo mkdir -p /var/lib/hydracad/uploads
sudo useradd -r -s /bin/false hydracad
sudo chown -R hydracad:hydracad /var/lib/hydracad /opt/hydracad/backend
sudo systemctl daemon-reload
sudo systemctl enable hydracad-api
sudo systemctl start hydracad-api
sudo systemctl status hydracad-api
```

3. **Hinweis:** Der Service läuft hier ohne HTTPS. HTTPS wird vor dem Dienst (Reverse-Proxy) terminiert (siehe 4.4).

### 4.3 Docker (Beispiel)

**Dockerfile** (im Backend-Verzeichnis oder Projektroot):

```dockerfile
FROM python:3.11-slim
WORKDIR /app
COPY backend/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY backend/ .
ENV HYDRACAD_DB_PATH=/data/hydracad.db
ENV HYDRACAD_UPLOAD_DIR=/data/uploads
VOLUME /data
EXPOSE 8000
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
```

**Start:**

```bash
docker build -t hydracad-api -f - . <<'DOCKERFILE'
FROM python:3.11-slim
WORKDIR /app
COPY backend/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY backend/ .
ENV HYDRACAD_DB_PATH=/data/hydracad.db
ENV HYDRACAD_UPLOAD_DIR=/data/uploads
VOLUME /data
EXPOSE 8000
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
DOCKERFILE

docker run -d --name hydracad-api -p 8000:8000 \
  -v hydracad-data:/data \
  -e HYDRACAD_JWT_SECRET="<starkes-geheimes-token>" \
  hydracad-api
```

### 4.4 HTTPS (Reverse-Proxy, z. B. Let's Encrypt)

**Empfehlung:** API nicht direkt im Internet exponieren; dahinter einen Reverse-Proxy (nginx, Caddy, Traefik) mit HTTPS betreiben.

**Beispiel nginx** (API auf Port 8000, HTTPS für `api.example.com`):

```nginx
server {
    listen 443 ssl http2;
    server_name api.example.com;
    ssl_certificate     /etc/letsencrypt/live/api.example.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/api.example.com/privkey.pem;

    location / {
        proxy_pass http://127.0.0.1:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

**Let's Encrypt (certbot):**

```bash
sudo certbot certonly --standalone -d api.example.com
# oder mit nginx: sudo certbot --nginx -d api.example.com
```

**Doku:** Keine Passwörter in Logs; in Produktion nur HTTPS verwenden. Siehe auch [TODO_USER_BACKEND_LICENSES.md](TODO_USER_BACKEND_LICENSES.md) Phase 1.8.

---

## 5. App-Config für Produktion (API-URL)

Damit die App in Produktion das Backend nutzt:

1. **Umgebungsvariable (empfohlen für Installationen):**  
   `CAD_API_BASE_URL=https://api.example.com` (oder `HYDRACAD_API_URL`, je nach App-Implementierung) setzen – z. B. in systemd, Desktop-Eintrag oder Installer-Skript.

2. **QSettings (Einstellungen in der App):**  
   Unter Einstellungen die API-Basis-URL auf die Produktions-URL setzen (z. B. `https://api.example.com`). Wird persistent gespeichert.

3. **Installation/Verteilung:**  
   In [INSTALLATION.md](INSTALLATION.md) bzw. Installer-Doku hinweisen: „Für Login/Lizenz/Community die API-URL in den Einstellungen eintragen oder `CAD_API_BASE_URL` setzen.“

---

## 6. Kurzreferenz

| Aufgabe | Referenz |
|--------|----------|
| API-Endpoints, lokaler Start | [backend/README.md](../backend/README.md) |
| Phase 5 Aufgaben (5.1–5.4) | [TODO_USER_BACKEND_LICENSES.md](TODO_USER_BACKEND_LICENSES.md) |
| App bauen, API-URL erwähnt | [INSTALLATION.md](INSTALLATION.md) |

Nach Abschluss der Checklisten (1–3) und Deployment (4–5) gilt Phase 5 als umgesetzt.
