# Offene Punkte – gebündelt

Alle noch offenen Aufgaben aus **TODO_GUI_INVENTOR_LEVEL**, **TODO_COMMUNITY_MAKERWORLD**, **TODO_USER_BACKEND_LICENSES** und TODO_INVENTOR_LEVEL_FULL, priorisiert.

---

## Kurz: Was ist noch offen?

| Bereich | Offen (Priorität) | Referenz |
|---------|-------------------|----------|
| **Community (Makerworld)** | Feed-Karten mit Daten, Detail-Ansicht, Backend-API (Feed/Detail/Download/Upload), CommunityService, Download → Viewport | [TODO_COMMUNITY_MAKERWORLD.md](TODO_COMMUNITY_MAKERWORLD.md) |
| **GUI Inventor** | Ribbon-Tabs (File, 3D Model, …), ViewCube/Nav (teils erledigt), Property Titelleiste OK/+ (teils), Browser-Icons, Dokument-Tabs, Suchleiste | [TODO_GUI_INVENTOR_LEVEL.md](TODO_GUI_INVENTOR_LEVEL.md) |
| **User & Lizenzen** | Phase 5: Tests (Registrierung, Login, Lizenz), Deployment (HTTPS, API-URL in Produktion) | [TODO_USER_BACKEND_LICENSES.md](TODO_USER_BACKEND_LICENSES.md) |

**Erledigt:** Community-Dock + Panel (Grundgerüst), Backend Auth + Lizenzen (Phase 1–4), App an API, Lizenz-UI, ViewCube + Nav + XYZ-Tripod, Property-Panel Sektionen, Ribbon horizontal + große Icons, ThemeManager.

---

## Priorität HOCH (als nächstes umsetzen)

| # | Quelle | Aufgabe | Status |
|---|--------|---------|--------|
| H1 | Viewport | **Navigation Bar** unter ViewCube: Pan, Zoom, Orbit, Home mit Icons | erledigt |
| H2 | Viewport | **Visual Style** im Viewport: Schattiert / Drahtgitter / Versteckte Linien umschaltbar (Toolbar oder Buttons) | erledigt |
| H3 | GUI | **Property-Panel Titelleiste**: Titel (z. B. „Eigenschaften Extrusion32“), X (Schließen), + (Neue Konfiguration), Auge (Sichtbarkeit), Suche, Hamburger | erledigt |
| H4 | Ribbon | **Ribbon-Tabs** wie Inventor: File, 3D Model/Sketch, Annotate, Inspect, Tools, Manage, View, Get Started | offen |
| H5 | Browser | **Model Browser**: Hierarchie Solid Bodies, View, Origin, Komponenten, End of Part; Icons pro Eintrag | teilweise |

---

## Priorität MITTEL

| # | Quelle | Aufgabe | Status |
|---|--------|---------|--------|
| M1 | Viewport | **Koordinaten-Widget (XYZ-Tripod)** unten links im Viewport | erledigt |
| M2 | Viewport | Orthografisch / Perspektive umschaltbar | erledigt |
| M3 | GUI | Ribbon: Kleine Icons mit **Dropdown** für Varianten (z. B. Extrude → Direction) | offen |
| M4 | Property | **Input-Felder** mit Einheiten (mm, in, °), Pick-Icon aus 3D-Ansicht | offen |
| M5 | Property | **Richtung/Boolean** als Icon-Reihe (Join, Cut, Intersect, New Body) | offen |
| M6 | Property | **OK, Abbrechen** unten; großer grüner **+** (Apply & New) | erledigt |
| M7 | Browser | **Sichtbarkeit/Unterdrückung** (Auge, durchgestrichen) neben Einträgen | offen |
| M8 | Viewport | **Live-Vorschau** bei Feature-Bearbeitung (z. B. transparente Extrusion) | offen |
| M9 | Building | Extrusion/Revolve/Hole/Fillet: **Boolean** (Join/Cut/Intersect), Taper, UI vollständig | teilweise |

---

## Priorität NIEDRIG

| # | Quelle | Aufgabe | Status |
|---|--------|---------|--------|
| N1 | Ribbon | **Suchleiste** rechts: „Search Help & Commands…“ (Placeholder) | offen |
| N2 | GUI | **Dokument-Tabs** unter dem Ribbon für geöffnete Dateien | offen |
| N3 | Browser | Such-/Filter-Icon und „Hinzufügen“ (+) in der Titelleiste des Panels | offen |
| N4 | Property | Preset-Dropdown („No Preset“) mit + und Zahnrad | offen |
| N5 | Analyse | Entformungsschrägen-Analyse, Wandstärken-Analyse (Overlay) | offen |
| N6 | Backend | **Phase 5**: Tests (Registrierung, Login, Lizenz), Deployment (HTTPS, App-Config); Doku: [PHASE5_TESTS_DEPLOYMENT.md](PHASE5_TESTS_DEPLOYMENT.md) | Doku erstellt, Durchführung offen |

---

## Community (Makerworld-Style) – offen

| # | Aufgabe | Priorität |
|---|---------|-----------|
| C1 | **Feed-Liste:** Karten mit Thumbnail, Titel, Autor, Likes, Downloads (z. B. Mock, dann API) | hoch |
| C2 | **Detail-Ansicht:** Klick auf Karte → Vorschau, Beschreibung, Kommentare, Download / In Viewport öffnen | hoch |
| C3 | **Backend API:** Feed, Detail, Download, Upload, Like, Kommentare | hoch/mittel |
| C4 | **App:** CommunityService, Feed laden, Download → Import in Viewport; Upload-UI mit Auth | hoch |
| C5 | Suchleiste + Filter an echte API anbinden (derzeit Platzhalter) | mittel |

Siehe [TODO_COMMUNITY_MAKERWORLD.md](TODO_COMMUNITY_MAKERWORLD.md).

---

## Bereits erledigt (Referenz)

- ViewCube + Standardansichten (Top, Front, Right, …), Navigation Bar, XYZ-Tripod
- Backend Auth + Lizenzen (Phase 1–4), App an API, Lizenz-UI
- ThemeManager (Dialoge, Buttons, QGroupBox), Docks, AI-Chat rechts, Community-Dock (Grundgerüst)
- Ribbon Command Groups horizontal, große Icons, Undo/Redo
- Property-Panel: Sektionen (Selection/Status), Titelleiste (Titel, X, +, Auge)

---

## Umsetzungsreihenfolge (diese Session)

1. ~~Visual Style (H2)~~ – Buttons im ViewCube-Widget + AppController executeCommand (Shaded/Wireframe/HiddenLine)  
2. ~~Navigation Bar mit Icons (H1)~~ – unter ViewCube (O, P, Z, Home mit zoom_fit-Icon)  
3. ~~XYZ-Tripod (M1)~~ – QtViewportAxesWidget unten links  
4. ~~Property-Panel Titelleiste (H3)~~ – Titel „Properties“, X, +, Auge (Sichtbarkeit)  
5. ~~Phase 5 (N6) – Doku/Checkliste für Tests & Deployment~~ – [PHASE5_TESTS_DEPLOYMENT.md](PHASE5_TESTS_DEPLOYMENT.md) erstellt; Durchführung (Checklisten abhaken) offen.  
