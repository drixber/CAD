# TODO: Inventor-Niveau – Building, Features, Texturen, Viewport

Ziel: Hydra CAD in **Modellierung**, **Features**, **Texturen/Materialien** und **Viewport** auf das Niveau von Autodesk Inventor / Fusion 360 bringen. Keine Abstriche gegenüber Inventor & Co.

Abgeleitet aus Referenz-Screenshots (Inventor, Fusion 360): Baugruppen, Materialien, ViewCube, Visual Style (Shadows, Reflections), Feature-Baum, Ribbon, Property-Panels.

---

## 1. Building / Modellierung (Feature-Parität)

| # | Thema | Status | Priorität | Details |
|---|--------|--------|-----------|---------|
| 1.1 | **Extrusion** | teilweise | hoch | Richtung (Normal/Reverse/Both), Distance A/B, Boolean (Join/Cut/Intersect/New Body), Taper, Match Shape. Modeler/Part hat createExtrude; UI/Backend-Anbindung vollständig machen. |
| 1.2 | **Revolve** | teilweise | hoch | Achse wählbar, Winkel, Boolean. createRevolve vorhanden; Panel + Picken der Achse. |
| 1.3 | **Sweep** | teilweise | hoch | Profil + Pfad, Twist, Scale. createSweep; UI und Geometrie-Anbindung. |
| 1.4 | **Loft** | teilweise | hoch | Mehrere Skizzen, Optionen. createLoft; UI. |
| 1.5 | **Bohrung (Hole)** | teilweise | hoch | Durch, Tiefe, Gewinde-Optionen, Bohrungsmuster. createHole; Panel mit Normen. |
| 1.6 | **Verrundung (Fillet)** | teilweise | hoch | Radius, Kantenauswahl, variable Radien. createFillet; Edge-Pick. |
| 1.7 | **Fase (Chamfer)** | teilweise | hoch | Distanz/Winkel, Kanten. createChamfer. |
| 1.8 | **Schale (Shell)** | teilweise | mittel | Innen/Außen/Symmetrisch, Stärke, zu entfernende Flächen. createShell; Panel wie Inventor. |
| 1.9 | **Muster (Pattern)** | teilweise | mittel | Rechteckig/kreisförmig, Richtung A/B, Anzahl, Abstand, unregelmäßige Abstände. createPattern. |
| 1.10 | **Spiegeln (Mirror)** | teilweise | mittel | Ebene, Merge. createMirror. |
| 1.11 | **Rippe (Rib)** | teilweise | niedrig | createRib. |
| 1.12 | **Entformung (Draft)** | teilweise | niedrig | Winkel, Ebene, Flächen. createDraft. |
| 1.13 | **Gewinde (Thread)** | teilweise | niedrig | Norm, Steigung, innen/außen. createThread. |
| 1.14 | **Helix** | teilweise | niedrig | Radius, Steigung, Windungen. createHelix. |
| 1.15 | **Baugruppen** | teilweise | hoch | Komponenten platzieren, Gelenke (Mate), Unterdrückung, Sichtbarkeit. AssemblyManager/Browser. |
| 1.16 | **Design-Historie** | offen | mittel | Zeitleiste/Feature-Baum bearbeitbar (Schritt zurück, Parameter ändern). |

---

## 2. Texturen / Materialien / Visual Style

| # | Thema | Status | Priorität | Details |
|---|--------|--------|-----------|---------|
| 2.1 | **Material pro Körper/Fläche** | teilweise | hoch | RenderEngine3D/Coin3D setMaterial(r,g,b,a); UI: Material-Bibliothek (Stahl, Alu, Kunststoff, Glas) zuweisbar. |
| 2.2 | **Visual Style im Viewport** | erledigt | hoch | Shaded/Wireframe/Hidden Line im ViewCube-Widget + Ribbon (AppController). |
| 2.3 | **Schatten (Shadows)** | Service vorhanden | hoch | VisualizationService enable_shadows; im interaktiven Viewport aktivierbar (Option wie Inventor). |
| 2.4 | **Reflexionen (Reflections)** | Service vorhanden | hoch | enable_reflections; im Viewport optional. |
| 2.5 | **Texture Mapping** | offen | mittel | Textur auf Flächen (Bild, UV); optional. |
| 2.6 | **Ground Plane / Reflexion** | offen | niedrig | Bodenebene mit Spiegelung wie in Referenz. |
| 2.7 | **Orthografisch / Perspektive** | erledigt | mittel | Persp/Ortho-Toggle im ViewCube; SoQtViewerIntegration::setProjectionType. |

---

## 3. Viewport: ViewCube & Navigation

| # | Thema | Status | Priorität | Details |
|---|--------|--------|-----------|---------|
| 3.1 | **ViewCube** | erledigt | hoch | Oben rechts: Buttons Top/Front/Right/Left/Back/Bottom/Iso/Home (QtViewCubeWidget); Klick setzt Kamera. |
| 3.2 | **Standardansichten** | erledigt | hoch | setStandardView("Top"|"Front"|"Right"|"Left"|"Back"|"Bottom"|"Isometric"); SoQtViewerIntegration + Viewport3D. |
| 3.3 | **Navigation Bar** | erledigt | hoch | Unter ViewCube: O, P, Z, Home (zoom_fit-Icon); QtViewCubeWidget. |
| 3.4 | **Koordinaten-Widget (XYZ-Tripod)** | erledigt | mittel | QtViewportAxesWidget unten links (X/Y/Z farbig). |
| 3.5 | **Home View** | teilweise | mittel | resetCamera/fitToView; als „Home“ speichern und abrufbar. |
| 3.6 | **Heller/weißer Hintergrund** | konfigurierbar | mittel | Bereits Hintergrundfarbe; Preset „Inventor Light“. |

---

## 4. UI: Ribbon, Browser, Property-Panels

| # | Thema | Status | Priorität | Details |
|---|--------|--------|-----------|---------|
| 4.1 | Ribbon-Tabs wie Inventor | teilweise | hoch | File, 3D-Modell/Skizze, Analyse, Extras, Ansicht, Verwalten. |
| 4.2 | Model Browser (Feature-Baum) | teilweise | hoch | Ursprung, Ebenen, Achsen, Skizzen, Körper, End of Part; Icons, Sichtbarkeit (Auge). |
| 4.3 | Property-Panel kontextabhängig | teilweise | hoch | Titelleiste (Titel, X, +, Auge) erledigt; Klappbare Sektionen, OK/Apply & New. |
| 4.4 | Suchleiste „Search Help & Commands“ | offen | niedrig | Platzhalter rechts im Ribbon. |
| 4.5 | Dokument-Tabs | offen | mittel | Unter dem Ribbon für geöffnete Dateien. |

*(Detaillierte GUI-Punkte siehe [TODO_GUI_INVENTOR_LEVEL.md](TODO_GUI_INVENTOR_LEVEL.md).)*

---

## 5. Analyse & Überlagerungen

| # | Thema | Status | Priorität | Details |
|---|--------|--------|-----------|---------|
| 5.1 | **Entformungsschrägen-Analyse** | offen | niedrig | Überlagerung (z. B. farbige/flache Darstellung) wie in Inventor-Screenshot. |
| 5.2 | **Wandstärken-Analyse** | offen | niedrig | Dickenverteilung visualisieren. |
| 5.3 | **Interferenzprüfung** | vorhanden | - | InterferenceChecker; ggf. UI anbinden. |

---

## 6. Live-Vorschau & UX

| # | Thema | Status | Priorität | Details |
|---|--------|--------|-----------|---------|
| 6.1 | **Live-Vorschau bei Feature-Bearbeitung** | offen | mittel | z. B. transparente Extrusion vor Apply. |
| 6.2 | **Selection Highlighting** | teilweise | hoch | Ausgewählte Objekte klar hervorheben (Farbe/Rand); konsistent. |
| 6.3 | **Transparenz pro Komponente** | offen | mittel | Baugruppen: Teile halbtransparent für Einblick. |

---

## 7. Umsetzungsreihenfolge (Empfehlung)

1. **Viewport:** ViewCube + Standardansichten (3.1, 3.2) → Navigation Bar (3.3).  
2. **Visual Style:** Schatten/Reflexionen im Viewport anbindbar (2.2, 2.3, 2.4).  
3. **Building:** Extrusion/Revolve/Hole/Fillet UI und Boolean-Optionen (1.1–1.7).  
4. **Material:** Material-Bibliothek und Zuweisung im Viewport (2.1).  
5. **Browser & Panels:** Siehe TODO_GUI_INVENTOR_LEVEL.md.  
6. **Analyse & Live-Vorschau** nach Bedarf.

---

## Referenzen (Code)

| Bereich | Dateien |
|--------|---------|
| Viewport | `src/ui/viewport/Viewport3D.cpp`, `SoQtViewerIntegration.cpp`, `QtViewport.cpp` |
| Modeler/Features | `src/core/Modeler/Modeler.cpp`, `Part.h` |
| Visualisierung | `src/modules/visualization/VisualizationService.cpp`, `RenderEngine3D.cpp`, `Coin3DIntegration.cpp` |
| Ribbon/Browser/Panel | `src/ui/qt/QtRibbon.cpp`, `QtBrowserTree.cpp`, `QtPropertyPanel.cpp` |
| GUI-TODO | [TODO_GUI_INVENTOR_LEVEL.md](TODO_GUI_INVENTOR_LEVEL.md) |
