# TODO: User-Backend & Lizenzen

Konkrete Aufgaben für die Umsetzung (eigenes Backend).  
Basis: `docs/PLAN_USER_AND_LICENSING.md`.

---

## Phase 1 – Backend Auth (ca. 3–5 Tage)

- [ ] **1.1** Server: User-Tabelle anlegen (z. B. `id`, `username`, `email`, `password_hash`, `created_at`, `last_login_at`).
- [ ] **1.2** Server: Passwort-Hashing (bcrypt oder Argon2) bei Registrierung; Passwörter nie im Klartext speichern.
- [ ] **1.3** Server: Endpoint `POST /api/auth/register` (username, email, password) → User anlegen, 201 + user_id oder Fehler.
- [ ] **1.4** Server: Endpoint `POST /api/auth/login` (username/email, password) → bei Erfolg JWT (Access + optional Refresh) zurückgeben.
- [ ] **1.5** Server: JWT-Logik (Signatur, Ablauf Access z. B. 15–60 Min, Refresh z. B. 30 Tage).
- [ ] **1.6** Server: Endpoint `POST /api/auth/refresh` (Refresh-Token) → neues Access-Token ausstellen.
- [ ] **1.7** Server: Endpoint `GET /api/auth/me` (Header: `Authorization: Bearer <token>`) → aktueller User (id, username, email) + ggf. Profil.
- [ ] **1.8** Server: HTTPS erzwingen; keine Passwörter in Logs.

---

## Phase 2 – App an API (ca. 3–4 Tage)

- [ ] **2.1** App: Konfiguration für API-Basis-URL (z. B. Umgebungsvariable oder Config-Datei).
- [ ] **2.2** App: `UserAuthService`: Registrierung → `POST /api/auth/register` aufrufen statt lokale Datei; Fehlerbehandlung (z. B. „Email bereits vergeben“).
- [ ] **2.3** App: `UserAuthService`: Login → `POST /api/auth/login` aufrufen; Access- und Refresh-Token speichern (sicher, z. B. QSettings oder OS Keychain).
- [ ] **2.4** App: Beim Start: wenn Token vorhanden → `GET /api/auth/me` (mit Bearer-Token); bei Erfolg User als eingeloggt setzen.
- [ ] **2.5** App: Wenn Access-Token abgelaufen: Refresh-Token → `POST /api/auth/refresh` → neues Access-Token speichern; bei Fehler Logout / Login-Dialog.
- [ ] **2.6** App: „Angemeldet bleiben“ = Refresh-Token persistieren; ohne „Angemeldet bleiben“ nur Access-Token im Speicher (Session Ende beim Schließen).
- [ ] **2.7** App: Logout: Token verwerfen (lokal löschen); optional Server-Endpoint `POST /api/auth/logout` (Token invalidation) falls gewünscht.
- [ ] **2.8** App: Fallback/Fehlerbehandlung wenn Server nicht erreichbar (Meldung, ggf. Offline-Modus oder lokaler Fallback definieren).

---

## Phase 3 – Backend Lizenzen (ca. 2–3 Tage)

- [ ] **3.1** Server: Lizenz-Tabelle (z. B. `id`, `user_id`, `license_key`, `type` [free/trial/pro/enterprise], `starts_at`, `expires_at`, `machine_ids` [JSON oder eigene Tabelle], `created_at`).
- [ ] **3.2** Server: Endpoint `POST /api/license/activate` (Header: Bearer; Body: license_key, machine_id) → Lizenz prüfen, User zuordnen, Machine-ID speichern (max. N pro User); Antwort gültig/ungültig + Lizenztyp + Ablauf.
- [ ] **3.3** Server: Endpoint `POST /api/license/check` (Header: Bearer; Body: machine_id) → Lizenzstatus für User + Machine zurückgeben (gültig, typ, expires_at).
- [ ] **3.4** Server: Lizenzschlüssel-Erzeugung (Admin-Tool oder Script): eindeutige Keys, Typ, Laufzeit; in DB speichern.
- [ ] **3.5** Server: Regeln: Free ohne Schlüssel; Trial mit Ablauf; Pro/Enterprise mit Schlüssel + optional Machine-Bindung (max. X Geräte).

---

## Phase 4 – App Lizenzen (ca. 2–3 Tage)

- [ ] **4.1** App: `LicenseService` (oder Erweiterung bestehender Logik): Aufruf `POST /api/license/check` (Token + Machine-ID); Ergebnis cachen (z. B. 24 h oder bis Ablauf).
- [ ] **4.2** App: Beim Start (nach Auth): Lizenz prüfen; wenn ungültig/abgelaufen: Hinweis anzeigen, ggf. Feature-Einschränkung.
- [ ] **4.3** App: UI „Lizenz aktivieren“: Eingabefeld Lizenzschlüssel + Button → `POST /api/license/activate` (Token + Schlüssel + Machine-ID); bei Erfolg Lizenz cachen und Meldung anzeigen.
- [ ] **4.4** App: Geschützte Features: vor Ausführung `LicenseService::isAllowed(feature)` prüfen (z. B. „export“, „cloud“, „pro_module“); bei nicht erlaubt Dialog oder deaktivierter Button.
- [ ] **4.5** App: Offline-Toleranz: wenn Server nicht erreichbar, letzte gültige Lizenz-Response + Toleranz (z. B. 7 Tage) akzeptieren; danach eingeschränkter Modus oder Hinweis.

---

## Phase 5 – Abschluss & Puffer (ca. 2–4 Tage)

- [ ] **5.1** Tests: Registrierung, Login, Refresh, Logout (manuell oder automatisiert).
- [ ] **5.2** Tests: Lizenz aktivieren, Lizenz prüfen, Offline-Verhalten.
- [ ] **5.3** Dokumentation: API-Endpoints (z. B. in README oder OpenAPI); Konfiguration der App (API-URL, Lizenz-Features).
- [ ] **5.4** Deployment: Server deployen (HTTPS); App-Config für Produktion (API-URL setzen).

---

## Kurzreferenz API (Zielbild)

| Endpoint | Methode | Zweck |
|----------|---------|--------|
| `/api/auth/register` | POST | User anlegen |
| `/api/auth/login` | POST | Login → JWT |
| `/api/auth/refresh` | POST | Refresh → neues Access-Token |
| `/api/auth/me` | GET | Aktueller User (mit Token) |
| `/api/license/activate` | POST | Lizenzschlüssel + Machine-ID aktivieren |
| `/api/license/check` | POST | Lizenzstatus (Token + Machine-ID) |

---

**Geschätzte Gesamtdauer:** ca. 2,5–4 Wochen (ein Entwickler Vollzeit).  
Nach Phase 1+2: **User + Backend nutzbar.** Nach Phase 3+4: **Lizenzen nutzbar.**
