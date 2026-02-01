# Community-Ansicht (Makerworld-Style)

Ziel: In Hydra CAD eine **Community-Ansicht** wie bei [Makerworld](https://makerworld.com) – Nutzer können geteilte Modelle/Designs entdecken, liken, kommentieren, herunterladen und selbst hochladen.

---

## 1. Was Makerworld bietet (Orientierung)

- **Feed:** Geteilte 3D-Modelle/Designs mit Vorschaubild, Titel, Autor, Likes, Downloads
- **Kategorien/Filter:** Trending, Neu, Beliebt, Suche, Tags
- **Detailseite:** Großes Vorschaubild, Beschreibung, Kommentare, Download-Button, „Remix“ / In App öffnen
- **Upload:** Nutzer können eigene Designs teilen (Datei + Titel + Beschreibung + Tags)
- **Profil:** Eigene Uploads, Likes, Follower (optional)
- **Sozial:** Likes, Kommentare, ggf. Follows

---

## 2. UI in Hydra CAD

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 2.1 | **Dock „Community“** (rechts, tabbed mit AI Chat / Properties) mit Community-Panel | erledigt (QtCommunityPanel) | - |
| 2.2 | Suchleiste + Filter (Trending, Neu, Beliebt, Meine Uploads) | Platzhalter | hoch |
| 2.3 | **Feed-Liste:** Karten mit Thumbnail, Titel, Autor, Likes, Downloads | offen | hoch |
| 2.4 | Klick auf Karte → Detail-Ansicht (Vorschau, Beschreibung, Kommentare, Download / In Viewport öffnen) | offen | hoch |
| 2.5 | **Upload-Button:** Eigenes Projekt teilen (Titel, Beschreibung, Tags, Datei) | offen | mittel |
| 2.6 | Optional: Vollbild-Community-Ansicht (zentraler Bereich umschaltbar Design ↔ Community) | offen | niedrig |

---

## 3. Backend / API (für echte Community)

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 3.1 | **API Feed:** `GET /api/community/feed?sort=trending|new|popular&q=...` → Liste Items (id, title, author, thumbnail_url, likes, downloads, created_at) | offen | hoch |
| 3.2 | **API Detail:** `GET /api/community/items/:id` → Item inkl. Beschreibung, Kommentare, download_url | offen | hoch |
| 3.3 | **API Upload:** `POST /api/community/items` (Auth, multipart: file + title, description, tags) | offen | mittel |
| 3.4 | **API Like:** `POST /api/community/items/:id/like`, `DELETE .../like` | offen | mittel |
| 3.5 | **API Kommentare:** `GET/POST /api/community/items/:id/comments` | offen | mittel |
| 3.6 | **API Download:** `GET /api/community/items/:id/download` → Datei (z. B. .step, .cad) oder redirect | offen | hoch |

Backend kann eigenes (Node/Python/Go) oder später an Makerworld-ähnliche Dienste angebunden werden.

---

## 4. App-Anbindung

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 4.1 | **CommunityService** (oder in AppController): Feed laden, Detail laden, Upload, Like, Download | offen | hoch |
| 4.2 | Download → Datei speichern und **in Viewport öffnen** (Import-Pfad nutzen) | offen | hoch |
| 4.3 | Upload: Aktuelles Projekt exportieren (z. B. STEP/CAD) und an API senden; Auth (Bearer Token) | offen | mittel |
| 4.4 | Offline/Fehler: Hinweis „Community derzeit nicht erreichbar“, ggf. Cache letzter Feed | offen | mittel |

---

## 5. Umsetzungsreihenfolge (Empfehlung)

1. **UI:** Community-Dock mit Panel (Suchleiste, Filter, Feed-Liste Platzhalter) – **erledigt (Grundgerüst).**
2. **Feed-Liste:** Echte Karten mit Mock-Daten (später API).
3. **Backend:** Feed- und Detail-Endpoints; Download-URL.
4. **App:** CommunityService, Feed/Detail anzeigen, Download → Import in Viewport.
5. **Upload:** API + UI „Teilen“.
6. **Likes/Kommentare:** API + UI.

---

## 6. Referenzen

- **Panel:** `src/ui/qt/QtCommunityPanel.cpp/h`
- **Main Window:** `src/ui/qt/QtMainWindow.cpp` (Dock Community)
- **Import (für Download):** `AppController` / `ImportExportService` / `IoPipeline`
- **Auth (für Upload/Like):** `UserAuthService` / Bearer Token
