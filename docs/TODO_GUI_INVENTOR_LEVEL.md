# GUI auf Inventor-Niveau – TODO

Ziel: Hydra CAD optisch und funktional auf das Niveau von Autodesk Inventor bringen (Ribbon, Model Browser, Property-/Feature-Panels, 3D-Viewport), **AI-Chat bleibt rechts**.

Abgeleitet aus den Referenz-Screenshots (Inventor 2020, Extrusion/Shell/Pattern-Panels, Assembly-Browser).

---

## 1. Ribbon (oberer Bereich)

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 1.1 | Ribbon-Tabs horizontal wie Inventor: File, 3D Model/Sketch, Annotate, Inspect, Tools, Manage, View, Get Started | erledigt | hoch |
| 1.2 | Aktiver Tab mit blauem Unterstrich (nicht nur Hintergrund) | erledigt (ThemeManager) | - |
| 1.3 | Command Groups pro Tab horizontal anordnen (nicht vertikal gestapelt) | erledigt | - |
| 1.4 | Große Icons für Hauptbefehle (z. B. Extrude, Revolve), Text darunter | erledigt (TextUnderIcon, 32×32) | - |
| 1.5 | Kleine Icons mit Dropdown für Varianten (z. B. Extrude → Direction) | erledigt (Extrude: Normal/Reverse/Both) | mittel |
| 1.6 | Suchleiste rechts: „Search Help & Commands…“ (Placeholder) | erledigt | niedrig |
| 1.7 | Quick Access Toolbar über dem Ribbon: Save, Undo, Redo, Open (bereits vorhanden, optisch an Inventor anpassen) | optional | niedrig |
| 1.8 | Dokument-Tabs unter dem Ribbon für geöffnete Dateien | erledigt (Placeholder „Document 1") | mittel |

---

## 2. Model Browser (linkes Panel)

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 2.1 | Hierarchische Baumansicht: Solid Bodies, View, Origin, Komponenten, End of Part | erledigt | hoch |
| 2.2 | Pro Eintrag Icon (Assembly, Part, Sketch, Feature) | erledigt | mittel |
| 2.3 | Kontextmenü (Rechtsklick): Sichtbarkeit, Unterdrücken, Neue Komponente, Aus Datei platzieren, Bearbeiten, iProperties, Umbenennen, Löschen, Kopieren, Einfügen, Suchen, Abhängigkeiten anzeigen | erledigt (Struktur, Aktionen noch Stubs) | - |
| 2.4 | Such-/Filter-Icon und „Hinzufügen“ (+) in der Titelleiste des Panels | erledigt (Filter + „+") | niedrig |
| 2.5 | Sichtbarkeit/Unterdrückung als kleines Symbol neben Einträgen (Auge, durchgestrichen) | erledigt (Auge-Spalte) | mittel |

---

## 3. Property-/Feature-Panels (rechts, kontextabhängig)

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 3.1 | Titelleiste pro Panel: Titel (z. B. „Eigenschaften Extrusion32“), X (Schließen), + (Neue Konfiguration), Auge (Sichtbarkeit), Suche, Hamburger-Menü | erledigt (Titel, X, +, Auge) | hoch |
| 3.2 | Kontext/Breadcrumb unter Titel: z. B. „Extrusion32 > Skizze44“ | erledigt (setBreadcrumb) | mittel |
| 3.3 | Preset-Dropdown („No Preset“) mit + und Zahnrad | offen | niedrig |
| 3.4 | **Klappbare Sektionen**: z. B. „Input Geometry“, „Behavior“, „Output“, „Advanced Properties“ mit Dreieck (▼/▶) | teilweise (QGroupBox „Selection“/„Status“ im Property-Panel, ThemeManager QGroupBox-Styling) | hoch |
| 3.5 | Input-Felder mit Einheiten (mm, in, deg), Dropdown-Pfeil, optional Icon zum Picken aus der 3D-Ansicht | offen | mittel |
| 3.6 | Richtung/Boolean als Icon-Reihe (z. B. Join, Cut, Intersect, New Body) mit Hervorhebung | offen | mittel |
| 3.7 | Checkboxen modern (z. B. „Match Shape“, „Allow Approximation“) | erledigt (ThemeManager QCheckBox) | - |
| 3.8 | OK, Abbrechen unten; großer grüner „+“ (Apply & New) | erledigt | mittel |
| 3.9 | Grüner „+“ zum Hinzufügen von Überschreibungssätzen / unregelmäßigen Abständen | offen | niedrig |

---

## 4. 3D-Viewport

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 4.1 | **ViewCube** oben rechts (TOP, FRONT, LEFT etc.) für schnelle Ansichtwechsel | erledigt | hoch |
| 4.2 | **Navigation Bar** unter ViewCube: Pan, Zoom, Orbit, Home View, ggf. Section/Fullscreen | erledigt | hoch |
| 4.3 | **Koordinaten-Widget** (XYZ-Tripod) unten links | erledigt | mittel |
| 4.4 | Heller/weißer Hintergrund, saubere Schattierung und Kanten (Rendering-Qualität) | teilweise (Coin3D/SoQt) | mittel |
| 4.5 | Live-Vorschau bei Feature-Bearbeitung (z. B. transparente Extrusion) | offen | mittel |

---

## 5. Dialoge & allgemeines UI-Niveau

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 5.1 | Dialoge: abgerundete Ecken (6–8 px), dezente Schatten, flache Moderne | erledigt (ThemeManager QDialog) | - |
| 5.2 | Einheitliche Schrift: Segoe UI / SF Pro / system-ui | erledigt (ThemeManager) | - |
| 5.3 | Eingabefelder: abgerundet, klarer Fokus-Rand (blau) | erledigt (ThemeManager) | - |
| 5.4 | Buttons: abgerundet, Primary (blau) für OK/Apply | erledigt (ThemeManager) | - |
| 5.5 | QGroupBox für Sektionen: flach, klappbar, einheitliche Palette | erledigt (ThemeManager QGroupBox + Property-Panel Sektionen) | - |

---

## 6. Layout & AI-Chat

| # | Aufgabe | Status | Priorität |
|---|---------|--------|-----------|
| 6.1 | **AI-Chat rechts** beibehalten (als Dock, tabbed mit Properties/Agent) | erledigt (QtMainWindow) | - |
| 6.2 | Viewport zentral, skaliert mit Fenstergröße; Browser links, Properties/Chat rechts | erledigt | - |
| 6.3 | Docks verschiebbar/speicherbar (Layout Manager) | erledigt (DockLayoutManager) | - |

---

## 7. Funktionsparität (aus den Panels ableitbar)

| # | Bereich | Mögliche Features | Priorität |
|---|---------|-------------------|-----------|
| 7.1 | Extrusion | Richtung (Normal/Reverse/Both), Distance A/B, Boolean (Join/Cut/Intersect/New Body), Taper, Match Shape | mittel |
| 7.2 | Shell | Wandlungstyp (Innen/Außen/Symmetrisch), Stärke, Flächen entfernen, Überschreibungssätze für Stärke | niedrig |
| 7.3 | Rechteckige Anordnung | Richtung A/B, Anzahl, Verteilung, Abstand, unregelmäßiger Abstand | niedrig |
| 7.4 | Stanzwerkzeug (Sheet Metal) | Pfad, Name, Positionen, Drehwinkel, Größenparameter, Über Biegung, Abwicklung | niedrig |

---

## 8. Umsetzungsreihenfolge (Empfehlung)

1. **Ribbon**: Groups horizontal (1.3), ggf. größere Icons (1.4).  
2. **Property-Panel**: Klappbare Sektionen (3.4, 5.5), Titelleiste mit X/+ (3.1).  
3. **Model Browser**: Kontextmenü (2.3), Icons (2.2).  
4. **Viewport**: ViewCube (4.1), Navigation Bar (4.2), Koordinaten (4.3).  
5. **Dialoge**: QGroupBox-Styling (5.5), OK/Cancel/+ (3.8).  
6. **Suchleiste** (1.6), **Dokument-Tabs** (1.8), **Feature-Live-Vorschau** (4.5) nach Bedarf.

---

## Referenzen

- **ThemeManager:** `src/ui/theme/ThemeManager.cpp`
- **Ribbon:** `src/ui/qt/QtRibbon.cpp`
- **Property Panel:** `src/ui/qt/QtPropertyPanel.cpp`
- **Browser:** `src/ui/qt/QtBrowserTree.cpp`
- **Main Window / Docks:** `src/ui/qt/QtMainWindow.cpp`
- **Viewport:** `src/ui/viewport/Viewport3D.cpp`, `SoQtViewerIntegration.cpp`, **ViewCube:** `src/ui/qt/QtViewCubeWidget.cpp`
- **Building/Features/Texturen/Viewport (vollständig):** [TODO_INVENTOR_LEVEL_FULL.md](TODO_INVENTOR_LEVEL_FULL.md)
