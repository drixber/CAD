# Plan: Benutzer (Registrierung, Speichern, Wiedernutzung) & Lizenzen

**Nur Planung – keine Umsetzung.**  
Stand: 2026-01-27

---

## 1. Ist-Zustand (kurz)

### Benutzer & Auth
- **Registrierung:** Lokal, `UserAuthService::registerUser()` → Benutzer werden in einer **lokalen Datei** gespeichert (Textformat, z. B. `users.dat` im App-Datenverzeichnis).
- **Login:** Lokal, Passwort wird gegen gespeicherten Hash geprüft.
- **Speichern / Wiedernutzung:**
  - **Session:** Nur „Username merken“ über QSettings (HydraCAD) – kein Token, kein Auto-Login.
  - **Benutzerdaten:** Alle Nutzer in einer Datei pro Rechner; **kein** zentraler Server, **kein** Cloud-Account.
- **Konsequenz:** Jeder Rechner hat seine eigene Nutzerliste; kein Account über Geräte hinweg, keine Passwort-Zurücksetzung, keine zentrale Verwaltung.

### Lizenzen
- **Aktuell:** Keine Lizenzlogik – die App läuft nach Login ohne zeitliche oder funktionale Einschränkung.

---

## 2. Ziele (ohne konkrete Umsetzung)

- **Benutzer:** Registrierung, Speichern, sichere Wiedernutzung (Login, Session, ggf. mehrere Geräte).
- **Lizenzen:** Faire, nachvollziehbare Nutzungsmodelle (z. B. Free/Trial/Pro/Enterprise) mit klarer Prüfung (online/offline) und optionaler Bindung (Rechner/User).

---

## 3. Optionen: Wo und wie Benutzer speichern?

| Option | Beschreibung | Vorteile | Nachteile |
|--------|--------------|----------|-----------|
| **A) Lokal bleiben** | Wie jetzt: Nutzer nur in lokaler Datei, Session nur „Username merken“. | Kein Backend, Datenschutz einfach, offline. | Kein Gerätewechsel, kein Passwort vergessen, keine zentrale Nutzerverwaltung. |
| **B) Eigenes Backend** | Eigener Server (API): Registrierung, Login, Token, ggf. Profil/Sync. | Volle Kontrolle, eigene Lizenzlogik, Nutzerdaten bei dir. | Hosting, Wartung, Sicherheit (HTTPS, Passwort-Hashing, Token, etc.). |
| **C) Drittanbieter-Auth** | z. B. Firebase Auth, Auth0, Supabase Auth, Keycloak. | Schnell integrierbar, Passwort vergessen, OAuth, oft kostenlose Tiers. | Abhängigkeit, Datenschutz/DSGVO prüfen, ggf. Kosten bei Skalierung. |

**Empfehlung für Planung:**  
- Kurzfristig: **A** beibehalten, aber Session/„Wiedernutzung“ klarer machen (z. B. optionales „Angemeldet bleiben“ mit lokalem Token/Ablauf).  
- Mittelfristig: **B** oder **C**, wenn du Accounts geräteübergreifend und für Lizenzen brauchst.

---

## 4. Benutzer: Registrierung speichern & wieder nutzen (konzeptionell)

### 4.1 Registrierung
- **Lokal (Option A):** Unverändert: Registrierung → Eintrag in lokale Nutzerdatei (Hash, Email, Username, etc.).
- **Mit Backend (B/C):** Registrierung → API-Aufruf → Account auf Server/bei Provider; optional E-Mail-Verifizierung, dann „Account aktiv“.

### 4.2 Speichern
- **Was speichern?**
  - Bereits jetzt: Username, Email, Passwort-Hash, Erstell-/Letzt-Login-Datum.
  - Erweiterbar: Profil (Anzeigename, Sprache, bevorzugte Einstellungen), zugeordnete Lizenz(en), Geräteliste (wenn mehrgerätefähig).
- **Wo speichern?**
  - Lokal: weiter Datei + QSettings (oder kleine lokale DB).
  - Mit Backend: Server/Provider speichert Profil; Client speichert nur Session/Token und ggf. Cache.

### 4.3 Wiedernutzung (Login, Session, „Angemeldet bleiben“)
- **Einfach (lokal):**
  - „Username merken“ (wie jetzt).
  - Optional: „Angemeldet bleiben“ = lokales Session-Token (z. B. Zufalls-ID + Ablaufdatum), in QSettings oder sicherer Speicher; beim Start prüfen → wenn gültig, direkt als eingeloggt betrachten.
- **Mit Backend:**
  - Login → Server gibt Token (JWT o. ä.) zurück.
  - Client speichert Token (sicher, z. B. nur im Speicher oder OS Keychain); bei jedem Start oder bei geschützten Aktionen: Token mitschicken, Server prüft Gültigkeit.
  - „Angemeldet bleiben“ = längerer Token oder Refresh-Token; Ablauf und Widerruf auf Server-Seite möglich.

### 4.4 Mehrere Geräte (optional)
- Nur sinnvoll mit Backend/Provider: Ein Account, mehrere Geräte; Session pro Gerät oder gemeinsame Nutzer-ID; Lizenz kann „pro User“ statt „pro Rechner“ sein.

---

## 5. Lizenzen: Konzept (ohne Implementierungsdetails)

### 5.1 Mögliche Lizenzmodelle
- **Free:** Volle Nutzung ohne Zeitlimit (evtl. mit Einschränkungen, z. B. Wasserzeichen oder Limit bei Cloud-Features).
- **Trial:** Zeitlich begrenzt (z. B. 14/30 Tage), danach auf Free wechseln oder Lizenz kaufen.
- **Pro / Einzellizenz:** Ein Nutzer oder ein Rechner; Lizenzschlüssel oder Account-Bindung.
- **Enterprise / Mehrplatz:** Mehrere Nutzer/Rechner; zentrale Verwaltung, ggf. eigener Lizenzserver.

### 5.2 Lizenz-„Objekt“ (logisch)
- Typ (Free / Trial / Pro / Enterprise).
- Gültigkeit: Start- und Enddatum (bei Trial/Abo).
- Bindung: **Rechner** (Machine-ID), **Benutzer** (Account), oder beides.
- Optional: Feature-Flags (welche Module freigeschaltet sind).

### 5.3 Prüfung: Wo und wie?
- **Prüfpunkte in der App:**  
  Beim Start, nach Login, vor geschützten Aktionen (z. B. Export, Cloud, erweiterte Module). Zentrale Stelle (z. B. `LicenseService::isAllowed(feature)`) die alle Prüfungen bündelt.
- **Online-Prüfung:**  
  App sendet Lizenzschlüssel oder User-Token + Machine-ID an deinen Server; Server antwortet mit gültig/ungültig + Restlaufzeit. Bei Pro: regelmäßig (z. B. täglich/wöchentlich) oder bei Start.
- **Offline / Toleranz:**  
  Wenn kein Netz: letzte erfolgreiche Prüfung + Grauzone (z. B. 7–30 Tage) akzeptieren; danach nur eingeschränkte Nutzung bis wieder online.

### 5.4 Lizenzschlüssel (optional)
- Format: z. B. `XXXX-XXXX-XXXX-XXXX` (lesbar) oder längerer Opaque-Token.
- Erzeugung: Nur auf deiner Seite (Backend/Admin-Tool); Schlüssel wird mit Typ, Laufzeit, Bindung signiert oder in DB gespeichert.
- Aktivierung: User gibt Schlüssel ein → App sendet Schlüssel + Machine-ID/User an Server → Server speichert Zuordnung und gibt OK zurück; App speichert lokal „aktiviert“ + Ablauf.

### 5.5 Bindung (Rechner vs. User)
- **Rechner:** Machine-ID (z. B. Hash aus HW-Infos); Lizenz „auf diesem PC“; gut für Einzelplatz ohne Account.
- **User:** Account (eigenes Backend oder Provider); Lizenz „diesem User“; gut für Mehrgeräte.
- **Kombination:** z. B. „Pro User, max. 2 Geräte“ – Server verwaltet Geräteliste pro User.

---

## 6. Grober Phasenplan (nur Planung)

| Phase | Inhalt (konzeptionell) |
|-------|------------------------|
| **1 – Klarheit** | Entscheidung: Nur lokal (A) oder Backend/Provider (B/C). Festlegen: Welche Lizenzstufen (Free/Trial/Pro/…) brauchst du wirklich? |
| **2 – Auth/Session** | Session und „Wiedernutzung“ verbessern: optional „Angemeldet bleiben“ mit Ablauf; klare Trennung: wer ist eingeloggt, wo wird das gespeichert. Keine Lizenzlogik nötig. |
| **3 – Lizenzmodell** | Lizenz-Typen und -Regeln definieren (inkl. Trial-Dauer, Feature-Unterschiede). Kein Code, nur Spezifikation. |
| **4 – Lizenzprüfung (minimal)** | Eine zentrale Prüfung (z. B. „Lizenz gültig ja/nein“) beim Start; zunächst immer „gültig“ oder nur „Free“. Später erweitern. |
| **5 – Backend (falls gewählt)** | API für Registrierung, Login, Token; optional Lizenz-Aktivierung und -Abruf. Dann App an API anbinden. |
| **6 – Schlüssel & Aktivierung** | Falls Lizenzschlüssel gewünscht: Erzeugung, Aktivierungsflow, Bindung (Rechner/User), Offline-Toleranz. |

---

## 7. Offene Punkte (für dich zu entscheiden)

1. **Backend ja/nein?** Wenn ja: eigener Server oder Auth-Provider?
2. **Lizenzmodelle:** Welche Stufen (Free, Trial, Pro, …) und welche Features pro Stufe?
3. **Bindung:** Pro Rechner, pro User oder beides mit Begrenzung?
4. **Offline:** Wie lange darf die App ohne Internet „noch gültig“ sein?
5. **Datenschutz:** Wo werden E-Mail/Passwort/Token gespeichert; welche DSGVO-Maßnahmen (Dokumentation, Einwilligung, Löschung)?

---

## 8. Bezug zum bestehenden Code (nur Orientierung)

- **UserAuthService:** Lokale Nutzerdatei + QSettings für „Username merken“. Erweiterbar um: Session-Token, Ablauf, optional API-Calls.
- **ProjectFileService:** Unabhängig von Lizenzen; Projekte können weiter lokal bleiben; später optional „Projekt zu User/Cloud zuordnen“.
- **AppController:** Ruft Auth und Projektlogik auf; hier würden Lizenzprüfungen (z. B. vor bestimmten Aktionen) aufgerufen werden – konzeptionell eine Stelle wie `LicenseService::isAllowed(...)`.

Wenn du dich für eine Option (lokal vs. Backend) und ein klares Lizenzmodell entschieden hast, kann der nächste Schritt ein detaillierterer Plan (z. B. API-Design oder Lizenz-Datenmodell) sein – weiterhin nur Planung, keine Umsetzung.
