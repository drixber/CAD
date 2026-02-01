# TODO: User-Backend & Lizenzen

Konkrete Aufgaben für die Umsetzung (eigenes Backend).  
Basis: `docs/PLAN_USER_AND_LICENSING.md`.  
**Stand:** Phasen 1–4 umgesetzt (ohne Cloud). Phase 5: Doku, automatisierte API-Tests und Deployment-Artefakte umgesetzt (Checklisten in PHASE5_TESTS_DEPLOYMENT.md zum Abhaken; Durchführung vor Ort).

---

## Phase 1 – Backend Auth ✅

- [x] **1.1** Server: User-Tabelle (SQLite: `id`, `username`, `email`, `password_hash`, `created_at`, `last_login_at`). `backend/database.py`, `backend/auth.py`.
- [x] **1.2** Server: Passwort-Hashing (passlib/bcrypt) bei Registrierung; Passwörter nie im Klartext.
- [x] **1.3** Server: Endpoint `POST /api/auth/register` → User anlegen, JWT zurück.
- [x] **1.4** Server: Endpoint `POST /api/auth/login` → JWT (Access + Refresh).
- [x] **1.5** Server: JWT (python-jose), Access 60 Min, Refresh 30 Tage.
- [x] **1.6** Server: Endpoint `POST /api/auth/refresh` → neues Access-Token.
- [x] **1.7** Server: Endpoint `GET /api/auth/me` (Bearer) → User (id, username, email).
- [x] **1.8** Server: Keine Passwörter in Logs; HTTPS in Produktion (Doku/Deployment).

---

## Phase 2 – App an API ✅

- [x] **2.1** App: API-Basis-URL (Umgebungsvariable, QSettings). Bereits vorhanden.
- [x] **2.2** App: `UserAuthService::registerViaApi` → `POST /api/auth/register`; Fehlerbehandlung.
- [x] **2.3** App: `UserAuthService::loginViaApi` → `POST /api/auth/login`; Token in QSettings (optional Keychain später).
- [x] **2.4** App: Beim Start: Token → `GET /api/auth/me` bzw. Refresh → Me; User eingeloggt setzen.
- [x] **2.5** App: Access abgelaufen → `POST /api/auth/refresh`; bei Fehler Logout/Session Ende.
- [x] **2.6** App: „Angemeldet bleiben“ = Refresh-Token in QSettings; sonst nur Session.
- [x] **2.7** App: Logout: Token lokal löschen (`clearStoredTokens`).
- [x] **2.8** App: Fallback: Meldung bei Server-Fehler („Server error or network failure“).

---

## Phase 3 – Backend Lizenzen ✅

- [x] **3.1** Server: Lizenz-Tabelle (SQLite); `backend/database.py`, `backend/licenses.py`.
- [x] **3.2** Server: Endpoint `POST /api/license/activate` (Bearer, license_key, machine_id).
- [x] **3.3** Server: Endpoint `POST /api/license/check` (Bearer, machine_id).
- [x] **3.4** Server: Admin-Script `backend/admin_create_license.py` (free/trial/pro/enterprise, optional expires_at).
- [x] **3.5** Server: Regeln Free/Trial/Pro/Enterprise; max. N Maschinen pro Lizenz.

---

## Phase 4 – App Lizenzen ✅

- [x] **4.1** App: `LicenseService` → `POST /api/license/check`; Cache 24 h.
- [x] **4.2** App: Lizenz prüfen (optional beim Start; `license_service_.check()`).
- [x] **4.3** App: UI „Lizenz aktivieren“ (Settings → Activate License); `QtLicenseDialog`; `POST /api/license/activate`.
- [x] **4.4** App: `LicenseService::isAllowed(feature)` für spätere Feature-Freischaltung.
- [x] **4.5** App: Ohne API-URL: Lizenz = „free“, gültig; bei API-Fehler last_error_ setzen.

---

## Phase 5 – Abschluss & Puffer

**Dokumentation:** Ausführliche Checklisten und Deployment-Anleitung: [docs/PHASE5_TESTS_DEPLOYMENT.md](PHASE5_TESTS_DEPLOYMENT.md).

- [x] **5.1** Tests: Registrierung, Login, Refresh, Logout (manuell oder automatisiert).
  - **Automatisiert:** `backend/tests/test_auth.py` (pytest); manuelle Checkliste: [PHASE5_TESTS_DEPLOYMENT.md § 1](PHASE5_TESTS_DEPLOYMENT.md#1-tests-auth-registrierung-login-refresh-logout). Ausführen: `cd backend && pip install -r requirements.txt && pytest tests -v`.
- [x] **5.2** Tests: Lizenz aktivieren, Lizenz prüfen, Offline-Verhalten.
  - **Automatisiert:** `backend/tests/test_license.py` (pytest); manuelle Checkliste: [PHASE5_TESTS_DEPLOYMENT.md § 2](PHASE5_TESTS_DEPLOYMENT.md#2-tests-lizenz-aktivieren-prüfen-offline). Offline-Verhalten: App-seitig (Cache 24 h).
- [x] **5.3** Dokumentation: API in `backend/README.md`; App-Config (API-URL) in INSTALLATION/Docs.
- [x] **5.4** Deployment: Server deployen (HTTPS); App-Config für Produktion (API-URL setzen).
  - **Artefakte:** [docs/deployment/](deployment/) (systemd: `hydracad-api.service.example`, nginx: `nginx-api.example.conf`), [backend/Dockerfile](../backend/Dockerfile). Schritte: [PHASE5_TESTS_DEPLOYMENT.md § 4–5](PHASE5_TESTS_DEPLOYMENT.md#4-deployment-server-produktion).

---

## Kurzreferenz API

| Endpoint | Methode | Zweck |
|----------|---------|--------|
| `/api/auth/register` | POST | User anlegen |
| `/api/auth/login` | POST | Login → JWT |
| `/api/auth/refresh` | POST | Refresh → neues Access-Token |
| `/api/auth/me` | GET | Aktueller User (mit Token) |
| `/api/license/activate` | POST | Lizenzschlüssel + Machine-ID aktivieren |
| `/api/license/check` | POST | Lizenzstatus (Token + Machine-ID) |

---

**Backend starten:** `cd backend && pip install -r requirements.txt && uvicorn main:app --reload --port 8000`  
**App:** API-URL setzen (z. B. `CAD_API_BASE_URL=http://localhost:8000`), dann Registrierung/Login und „Lizenz aktivieren“ nutzbar.  
**Phase-5-Tests & Deployment:** [docs/PHASE5_TESTS_DEPLOYMENT.md](PHASE5_TESTS_DEPLOYMENT.md).
