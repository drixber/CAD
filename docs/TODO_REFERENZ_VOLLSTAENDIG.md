# TODO: Referenz-Vollständigkeit (Inventor / Fusion 360 / SolidWorks / Creo / NX / CATIA)

Abgeleitet aus **docs/REFERENZ_AUTODESK_INVENTOR.md**. Jeder Punkt aus der Referenz wird hier mit **Status** (vorhanden / teilweise / offen) und **konkretem Umsetzungsbedarf** ergänzt. Ziel: Lücken schließen, damit Hydra CAD die referenzierten CAD-Systeme funktional abdeckt.

**Legende:**  
- **vorhanden** = Kernfunktion existiert, ggf. verfeinerbar  
- **teilweise** = Ansatz da, Referenzumfang nicht erreicht  
- **offen** = bei uns nicht vorhanden, muss umgesetzt werden  

---

# Teil 1: Inventor-Referenz (Abschnitte 1–17)

## 1. Allgemeines / Arbeitsprinzip

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 1 | Parametrisches 3D-CAD, featurebasiert, Historie | teilweise | Modellierungskern (Part, Sketch, Feature-Liste) vorhanden; Historie/Undo konsistent mit Rollback-Leiste und Reihenfolgeänderung verknüpfen; „Skizze → Feature → Verlauf → Ableitung → Zeichnung“ als durchgängiger Workflow in UI und Doku abbilden. |
| 1 | Zielbereiche (Maschinenbau, Metallbau, Produktdesign, Baugruppen) | — | Kein Implementierungs-TODO; als Zielgruppen-Dokumentation/Positionierung nutzen. |

---

## 2. Dateitypen & Formate

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 2.1 | Native Formate .ipt, .iam, .idw, .dwg, .ipn, .ipj | vorhanden | Eigenes Format .hcad/.hcasm/.hcdrw/.hcadproject in docs/NATIVE_FORMATS.md; ProjectFileService Speichern/Laden inkl. Konfigurationen, Skeleton, Thin/PathPattern, 3D-Skizze; loadProjectConfig/saveProjectConfig. |
| 2.2 | Import: STEP, IGES, SAT, STL, OBJ, DXF, DWG, Parasolid, JT, SolidWorks | teilweise | STEP, IGES, STL, DXF, DWG, OBJ, 3MF, GLTF bereits in ImportExportService; fehlen: SAT, Parasolid (.x_t/.x_b), JT, native SolidWorks (.sldprt/.sldasm). Leser pro Format ergänzen, in IoPipeline/UI einbinden. |
| 2.3 | Export: PDF, DXF/DWG, STL, STEP, OBJ, FBX, SAT, JT | teilweise | STEP, IGES, STL, DXF, DWG, OBJ, 3MF, GLTF vorhanden; fehlen: PDF (Zeichnung), FBX, SAT, JT. Export-Methoden und Datei-Dialog-Optionen ergänzen. |

---

## 3. Startfenster (Inventor-Startseite)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 3 | Neue Datei (Standard.ipt, .iam, .idw), Benutzerdefinierte Vorlagen | teilweise | „Neu“ öffnet Vorlagen-Dialog: Vorlagenverzeichnis (ProjectConfig.template_directory, QSettings „templates/directory“), Auswahl „Leeres Dokument“ oder .cad/.hcad-Vorlagen, Kopie als neues Dokument (newProjectFromTemplate); Serialisierung TEMPLATE_DIR in .hcadproject. Optional: Vorlagen pro Dokumenttyp (Part/Assembly/Drawing). |
| 3 | Letzte Dateien, Projekt wechseln, Tutorials, Beispielprojekte, Cloud-Optionen | teilweise | Letzte Dateien (Recent Projects) vorhanden; Help-Menü mit Get Started, Tutorials, Sample Projects, About ergänzt; Projekt wechseln/Cloud optional. |

---

## 4. Projektverwaltung (.ipj)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 4 | Arbeitsbereich, Bibliothekspfade, Inhaltscenter, Suchpfade, absolute/relative Pfade, Vault | teilweise | ProjectConfig (working_directory, search_paths, library_paths); .hcadproject (load/save); projectConfigPathForProject(); beim Laden/Speichern wird .hcadproject geladen/gespeichert; Vault optional später. |

---

## 5. Benutzeroberfläche (UI)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 5.1 | Menüband, Grafikfenster, Modellbrowser, Eigenschaftenfenster, Statusleiste, Navigationswürfel, ViewCube, Mausnavigation | vorhanden / teilweise | Ribbon, Viewport, Browser (QtBrowserTree), Property-Panel, ViewCube, Navigation Bar vorhanden; Statusleiste mit Einheiten/Modus/Koordinaten konsistent füllen; Maus-Orbit/Pan/Zoom dokumentieren und ggf. an Inventor-Verhalten angleichen. |
| 5.2 | Ribbon-Tabs: Datei, 3D-Modell, Skizze, Zusammenbauen, Prüfen, Extras, Verwalten, Ansicht, Umgebung | teilweise | Tabs laut TODO_GUI_INVENTOR_LEVEL vorhanden; „Zusammenbauen“ und „Prüfen“ mit konkreten Befehlen (Mates, Kollision, Maßprüfung) füllen; „Umgebung“ für Blech/Schweiß/Rohr etc. als Wechsel der Workspace-Art. |

---

## 6. Modellbrowser

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 6 | Ursprung (XY/YZ/XZ, Mittelpunkt), Skizzen, Features, Arbeitselemente, Parameter, iLogic, Abhängigkeiten, Unterdrückung, Rollback-Leiste | teilweise | Baum mit Origin, Reference Geometry (Work Planes, Work Axes, Work Points), Bodies, Skizzen, Features; Part.workPlanes/Axes/Points werden in Browser angezeigt (setWorkPlanes/setWorkAxes/setWorkPoints); Parameter-Tabelle, iLogic, Rollback-Leiste später. |

---

## 7. Navigationssystem

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 7 | Orbit, Pan, Zoom, Zoom alles, Zoom Auswahl, ViewCube (Standardansichten), Isometrisch | vorhanden | ViewCube + Navigation Bar; Zoom Auswahl (fitToSelection) implementiert: Button „Zoom Selection“ in Viewport-Toolbar. |

---

## 8. Skizzenumgebung

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 8.1 | Skizze starten, Ebene wählen, Skizze beenden | teilweise | createSketch(name, plane) im Modeler; in UI: „Skizze starten“ mit Ebenenauswahl (XY/YZ/XZ oder Fläche), explizites „Skizze beenden“ (Rückkehr in 3D-Modell-Modus) anbieten. |
| 8.2 | Linie, Rechteck (2-Punkt, Mittelpunkt), Kreis (Mittelpunkt, Tangential), Bogen, Ellipse, Polygon, Spline, Punkt, Text | teilweise | Ellipse, Polygon, Spline, Text + addEllipse/addPolygon/addSpline/addText in Sketch; Serialisierung in ProjectFileService; Eigen-Kern. |
| 8.3 | Constraints: Horizontal, Vertikal, Parallel, Senkrecht, Tangential, Koinzident, Gleich, Symmetrisch, Fixiert | teilweise | Symmetric und Fixed in ConstraintType; Solver (Fixed = keine Delta, Symmetric = Achsen-Residual); Modeler/Serialisierung berücksichtigt. |
| 8.4 | Bemaßung: Längenmaß, Durchmesser, Radius, Winkel, Abstand, parametrisch verknüpft | teilweise | Distance/Angle als Constraints; Längen-/Radius-/Durchmesser-/Winkelmaß als beschriftete Bemaßung in Skizze (Anzeige + Parametername verknüpfen); Formeln/Parameter in Parameter-Tabelle. |
| 8.5 | Skizzenanalyse: Voll/Unter/Überbestimmt, Konfliktanzeige | teilweise | getDegreesOfFreedom, isOverConstrained, isUnderConstrained im Modeler; UI: Statusleiste zeigt „Sketch: Voll bestimmt (0 DOF)“ / „Unterbestimmt (N DOF)“ / „Überbestimmt“ (updateSketchConstraintStatus bei setActiveSketch, nach Skizzen-Befehlen, beim Laden). Konflikte (welche Constraints kollidieren) markieren optional später. |

---

## 9. 3D-Features

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 9.1 | Extrusion (hinzufügen, entfernen, vereinen), Rotation, Sweep, Loft | vorhanden | ExtrudeMode Join/Cut/Intersect/NewBody; createExtrude/createThinExtrude/applyThinExtrude; Serialisierung FEXTRUDE_MODE, FTHIN_WALL, FTHIN_THICKNESS; Rotation, Sweep, Loft. |
| 9.2 | Abrundung, Fase, Bohrung (Gewinde, Durchgang, Sackloch), Gewinde | teilweise | Fillet, Chamfer, Hole vorhanden; Hole: through_all im Feature + createHole/applyHole(diameter, depth, through_all); Serialisierung FTHROUGH_ALL; UI „Hole (Through All)“ im 3D-Modell-Ribbon. Gewindebohrung (Norm, Durchmesser, Tiefe) und Thread-Feature (Darstellung/Export) optional später. |
| 9.3 | Rechteckmuster, Kreismuster, Pfadmuster, Flächenmuster | vorhanden | Pattern, CircularPattern, PathPattern; Serialisierung FPATH_COUNT, FPATH_EQUAL; applyPathPattern. Flächenmuster optional. |
| 9.4 | Arbeitsebene, Arbeitsachse, Arbeitspunkt, Koordinatensystem | teilweise | ReferenceGeometry.h: WorkPlane, WorkAxis, WorkPoint, CoordinateSystem; Part: addWorkPlane/addWorkPlaneOffset, addWorkAxis/addWorkAxisBase, addWorkPoint, addCoordinateSystem; Serialisierung in ProjectFileService; im Modellbrowser anzeigen (UI) optional. |
| 9.5 | Direktbearbeitung: Fläche verschieben, löschen, ersetzen, Offset | teilweise | DirectEditService vorhanden; „Fläche verschieben“, „Fläche ersetzen“, „Offset-Fläche“ als konkrete Befehle mit Auswahl und Parametern (Vektor, Ersatzfläche, Offset-Wert) implementieren und in Ribbon/PropertyPanel. |

---

## 10. Parameter

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 10 | Benutzerparameter, Modellparameter, Formeln, Verknüpfungen, Externe Tabellen | teilweise | Part: userParameters(), addUserParameter, setParameterValue, removeParameter; Modeler::evaluatePartParameters(Part&); Formeln (+, -, *, /, Referenz auf andere Parameter); Serialisierung PART_PARAMETERS/UP: in ProjectFileService; UI: Parameter-Tabelle (Name, Wert, Formel) im Property-Panel, Befehl „Parameters“ zeigt Part-Benutzerparameter + Skizzen-Parameter. Verknüpfung Teil/Zeichnung, externe Tabellen (CSV/Excel) optional später. |

---

## 11. iLogic

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 11 | Regel erstellen, Wenn-Dann-Logik, Automatisierung, Variantenteile, Konfigurationssteuerung | teilweise | Rule (name, trigger, condition_expression, then_parameter, then_value_expression); Part: addRule, rules(); Modeler: evaluateCondition(expr, symbols), evaluatePartRules(Part&); Serialisierung PART_RULES/R: in ProjectFileService; UI „iLogic“ im Manage-Ribbon, Ausführung bei „Parameters“ und „iLogic“. Editor/Variantenteile optional später. |

---

## 12. Komponenten (Baugruppe)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 12 | Bauteil platzieren, Baugruppe platzieren, Ableiten, Ersetzen, Unterdrücken, Sichtbarkeit | teilweise | Assembly/AssemblyManager; „Bauteil/Baugruppe platzieren“ (Insert Part/Assembly), Ableiten (abgeleitetes Teil), Ersetzen (Komponente austauschen), Unterdrücken/Sichtbarkeit pro Komponente in Baum und Viewport. |

---

## 13. Abhängigkeiten (Constraints) & Gelenke

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 13 | Abhängigkeiten: Bündig, Fluchtend, Winkel, Tangential, Einfügen, Gear, Cam | vorhanden | MateType: Mate, Flush, Concentric, Tangent, Angle, Insert, Gear, Cam; createGear/createCam; solveMates/getDegreesOfFreedom; Serialisierung MATE (type int). |
| 13 | Gelenke: Drehgelenk, Schiebegelenk, Zylindergelenk, Planargelenk | vorhanden | JointType: Rigid, Revolute, Slider, Cylindrical, Planar; addJoint, joints(), getJointDegreesOfFreedom; createRevolute/createSlider/createCylindrical/createPlanar; Serialisierung JOINTS/JOINT. |

---

## 14. Baugruppenfunktionen

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 14 | Schrauben, Normteile (Content Center), Spiegeln, Muster, Kollision prüfen, Kontaktanalyse, Freiheitsgrade anzeigen | teilweise | Pattern/Mirror für Komponenten; Schrauben-Assistent (Lochauswahl, Norm/Nutzlänge); Content-Center-Äquivalent (Bibliothek + Einfügen); InterferenceChecker ausbauen (Kollision/Kontakt). Freiheitsgrade: updateAssemblyConstraintStatus() zeigt in Statusleiste „Assembly: N Komponenten, M Mates, DOF“ bzw. „Voll bestimmt (0 DOF)“ / „Überbestimmt“; Aufruf bei Init, nach Mate/Flush/Angle, beim Laden. Pro-Komponente-DOF optional später. |

---

## 15. Explosionsdarstellung

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 15 | Präsentation (.ipn), Explosionsschritte, Animieren, Zeitachsen, Rückwärtsanimation | teilweise | Explosion: Assembly setExplosionOffset/getExplosionOffset, setExplosionFactor, getDisplayTransform; Viewport setComponentTransform/updateNodeTransform; UI „Explosion View“ im Assembly-Ribbon; Serialisierung EXPLOSION_FACTOR/EXPLOSION_OFFSET; Sync Baugruppen-Transforms in Viewport. Offen: Timeline, Animieren Vor/Rück, Export Animation. |

---

## 16. Zeichnungen (.idw / .dwg)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 16.1 | Ansichten: Grundansicht, Projektion, Schnitt, Detail, Ausbruch, Hilfsansicht | teilweise | DrawingService/TechDrawBridge: Grundansicht, Schnitt (createSectionView), Detail (createDetailView); UI „Base View“, „Section“, „Detail View“; Ausbruch, Hilfsansicht optional später. |
| 16.2 | Bemaßung & Annotation: Maßlinien, Toleranzen, Passungen, Oberflächenzeichen, Schweißsymbole, Mittellinien, Stückliste | teilweise | AnnotationService, BomService; Maßlinien an Ansichten; Toleranzen/Passungen (Text/Symbol); Oberflächen-/Schweißsymbole; Mittellinien; Stückliste aus Baugruppe mit Positionierung auf Zeichnung. |
| 16.3 | Zeichnungsverwaltung: Blattformate, Maßstab, Rahmen, Schriftfelder, Normen (ISO/DIN/ANSI) | teilweise | SheetFormat + getSheetFormats() (A4–A0, ANSI A–D); DrawingSheet.format_id/width_mm/height_mm; TechDrawBridge Page aus Sheet; StylePreset DIN + dinStyles(); Maßstab pro Ansicht (DrawingView.scale). |

---

## 17. Zusatzumgebungen (Inventor)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 17.1 | Blech: Regeln (Dicke, Biegeradius, K-Faktor, Abwicklung, Eckbehandlung, Biegetabelle); Features (Flansch, Stanzung, Sicke, Abwicklung); DXF-Export, Fertigungshinweise | teilweise | SheetMetalRules (thickness, bend_radius_default, k_factor, corner_relief, bend_table_name); setRules/getRules; Punch (Stanzung), Bead (Sicke); exportFlatPatternToDxf(part_id, path) schreibt Kontur + Biegelinien; UI: SheetMetalRules, ExportFlatDXF, Punch, Bead im Part-Ribbon. Fertigungshinweise (Biegelinien-Texte) optional später. |
| 17.2 | Schweiß: Schweißumgebung, Nahttypen (Kehl-, Stumpf-, V-, U-, Punkt-, Kosmetisch), Nahtvolumen, Zeichnungsintegration (Symbole, Stückliste) | teilweise | WeldingService: WeldType (Fillet, Butt, V, U, Spot, Cosmetic); WeldJoint (name, type, length_mm, size_mm, part_a/b, symbol); addWeld, getWelds, getWeldBom, estimateWeldVolume, getSymbolText; UI „Weld“, „Weld BOM“ im Assembly-Ribbon. Zeichnungsintegration (Symbole in Zeichnung) optional später. |
| 17.3 | Rahmen: Profilbibliothek (DIN/ISO/EN), Rahmen einfügen, Gehrung, Zuschnittliste, Längenoptimierung | teilweise | FrameService: getProfileCatalog/getProfile (Rechteck, Rohr, U, I, L), addMember/addMemberAlongLine, setMiterAngle, getCutList, optimizeLengths (Zuschnittliste + Längenoptimierung). UI-Befehle optional später. |
| 17.4 | Rohr & Schlauch: Rohrsysteme, Routen, Biegeradien, Fittings, Stücklisten | teilweise | RouteSegment.bend_radius; FittingSpec (type, nominal_diameter, part_number); PipeBomItem; RoutingResult.fittings_used; createRigidPipe/createBentTube füllen fittings_used (Elbows pro Biegung); getRouteBom(route_id) → Rohr (Länge, DN) + Fittings (Typ, Menge); UI „Route BOM“ im Assembly-Ribbon; RigidPipe/FlexibleHose/BentTube setzen Typ korrekt. Anschlussdefinitionen optional später. |
| 17.5 | Kabel & Harness: Kabelbaum, Routen, Bündel, Kabellängen, Stecker | teilweise | HarnessService: createHarness, addCable, setCableRoute, getCableLength, addWaypoint, addBundle, getBundles. Stecker/Elektronik-Anbindung optional. |
| 17.6 | Belastungsanalyse: Statik, Randbedingungen, Material, Ergebnisse (Spannung, Verformung, Sicherheit, Bericht) | teilweise | SimulationService: FEA (FeaResult max_stress, safety_factor, stress_map); exportFeaReport(result, path); UI: FEA-Ergebnis (max stress, safety factor) in Statusleiste; „Export FEA Report“ → fea_report.txt. Farbskala optional später. |
| 17.7 | Dynamische Simulation: Bewegungsarten, Antriebe, Ausgabe (Diagramme, Animation) | teilweise | SimulationRequest.joint_drives (Gelenk → Winkelgeschwindigkeit); calculateMotion nutzt joint_drives für Antrieb; exportMotionReport(result, path) für Zeit/Position/Geschwindigkeit; UI „Export Motion Report“. Animation optional später. |
| 17.8 | Freiform: T-Splines, Ziehen, Glätten, Verdicken, Freiformflächen | teilweise | FreeformService-Stub: createFromPrimitive, pull, smooth, thickenToSolid. Vollimplementierung später. |
| 17.9 | Datenverwaltung (Vault): Versionen, Rechte, Freigabe, Revisionen | teilweise | VaultService-Stub: checkOut, checkIn, getVersionHistory. Vollimplementierung oder PDM-Anbindung später. |
| 17.10 | Automatisierung: iLogic, Makros, API (VBA/C#), Add-Ins, Konfiguratoren | teilweise | Siehe 11 (iLogic); AutomationService-Stub: runScript, recordMacro, stopMacro, getRecordedMacro. API/Add-Ins später. |

---

# Teil 2: Fusion-360-Referenz (18.x)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 18.1 | Cloud/Offline, Zielgruppen, Timeline-Version | — | Architektur-Dokumentation; Timeline wie Rollback/Historie (siehe 6, 9). |
| 18.2 | Projektstruktur, Versionierung, Exportformate (F3D, STEP, …) | teilweise | Projekt/Versionierung siehe 3, 4; Exportformate siehe 2; F3D-Äquivalent: eigenes Projektformat. |
| 18.3 | Datenpanel, Arbeitsbereichsauswahl, Toolbar, Canvas, Browser, Timeline, Navigationswürfel | teilweise | Browser, Viewport, ViewCube vorhanden; Datenpanel (Dateien/Projekte), Workspace-Wechsel (Design/Blech/Zeichnung/Simulation), Timeline unten wie Rollback-Leiste. |
| 18.4 | Design: Komponenten & Bodies, Skizzen (Elemente/Constraints/Bemaßung), Solid-Features, Muster/Spiegel, Arbeitselemente, Direktmodellierung, Timeline | siehe 8, 9, 6 | Wie Inventor: Skizzen + Features + Arbeitselemente + Direktbearbeitung + Timeline. |
| 18.5 | Surface Workspace: Patch, Extend, Trim, Stitch, Thicken, Loft, Boundary Surface | teilweise | SurfaceService-Stub: patch, trim, stitch, thicken. Vollimplementierung später. |
| 18.6 | Form (T-Splines): Primitiven, Ziehen, Glätten, Subdivision, Umwandlung Solid | teilweise | Wie 17.8 (FreeformService-Stub). |
| 18.7 | Sheet Metal: Regeln, Flansche, Abwicklung, DXF | siehe 17.1 | |
| 18.8 | Joints (Rigid, Revolute, Slider, …), As-Built Joint, Limits, Bewegungssimulation | siehe 13 | Gelenke + Limits + Motion. |
| 18.9 | Simulation: Static, Modal, Thermal, Event, Buckling, Shape Optimization; Randbedingungen; Ergebnisse/Bericht | siehe 17.6 | Erweiterung um Modal, Thermal, Buckling, Shape Optimization. |
| 18.10 | Generative Design: Designraum, Lastfälle, Material, Fertigung, Cloud, Varianten | teilweise | GenerativeService-Stub: createStudy (DesignSpace), getResultVariants. Solver/Varianten später. |
| 18.11 | Render: Materialien, HDRI, Kamera, Raytracing, Cloud Rendering | teilweise | VisualizationService; Materialien/Texturen, Umgebungslicht; Raytracing/Cloud optional. |
| 18.12 | Animation: Explosion, Zeitachse, Bewegungspfad, Kamera, Videoexport | siehe 15 | |
| 18.13 | Drawing: Ansichten, Schnitte, Maße, Stückliste, Normen | siehe 16 | |
| 18.14 | Manufacture (CAM): 2D/3D/5-Achs, Adaptive, Contour, Pocket, Drilling, Turning; Werkzeuge; Simulation, G-Code | teilweise | CamService-Stub: addTool, generateToolpath, exportGCode. Strategien/Postprozessor später. |
| 18.15 | Electronics: Schaltplan, PCB, Routing, 3D-PCB, ECAD-MCAD | teilweise | ElectronicsService-Stub: createSchematic, createPcb, routePcb, exportTo3D. Vollimplementierung später. |
| 18.16 | Cloud & Zusammenarbeit: Live-Sharing, Kommentare, Markups, Team-Projekte, Rechte | teilweise | CollaborationService-Stub: shareProject, addComment, getComments, setUserRole. Vollimplementierung später. |
| 18.17 | API (Python/JavaScript), Skripte, Add-Ins, Automatisierung | teilweise | AutomationService-Stub; Python-Bindings (cadursor). Skript-API (Python/JS) später. |

---

# Teil 3: SolidWorks-Referenz (19)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 19.1 | .SLDPRT, .SLDASM, .SLDDRW | teilweise | ImportExportService: FileFormat SldPrt/SldAsm/SldDrw, importSldPrt/importSldAsm, exportSldPrt/exportSldAsm (Stub: „use STEP“). |
| 19.2 | FeatureManager, PropertyManager, ConfigurationManager, CommandManager, Heads-Up | teilweise | Browser = FeatureManager; PropertyPanel = PropertyManager; Konfigurationen (ConfigurationManager) und Heads-Up-Toolbar (ViewCube/Ansicht) ausbauen. |
| 19.3 | 2D/3D-Skizzen, Beziehungen, Vollbestimmung, Gleichungen, Referenzgeometrie | vorhanden | Sketch: set3D/is3D, addWaypoint3D, waypoints3D(); Serialisierung SKETCH_3D/WAYPOINTS_3D/WP3D. Siehe 8, 9.4. |
| 19.4 | Extrusion, Rotation, Sweep, Loft, Thin Feature, Shell, Draft, Fillet, Chamfer | vorhanden | Siehe 9; Thin Feature mit Serialisierung FTHIN_WALL/FTHIN_THICKNESS. |
| 19.5 | Mates (Coincident, Parallel, Distance, Angle, Gear, Cam), Flexible Subassemblies, Large Assembly, SpeedPak | vorhanden | Gear/Cam + Solver; setComponentLightweight/isComponentLightweight; Serialisierung LIGHTWEIGHT. Flexible Subassemblies optional. |
| 19.6 | Konfigurationen: Varianten, Maß/Feature-Steuerung, Tabellen, Stücklistenlogik | vorhanden | Part/Assembly Configuration; Serialisierung CONFIG/ASSEMBLY_CONFIG/PART_ACTIVE_CONFIG/ASSEMBLY_ACTIVE_CONFIG; loadPartConfigurationsFromCsv/savePartConfigurationsToCsv. |
| 19.7 | Blech: Flansche, Abwicklung, K-Faktor, DXF, Fertigungszeichnungen | siehe 17.1 | |
| 19.8 | Simulation: Statik, Frequenz, Thermik, Ermüdung, Motion, Flow | siehe 17.6, 17.7 | Frequenz, Thermik, Ermüdung, Flow Simulation als Erweiterungen. |
| 19.9 | Routing, Electrical, Plastics, Visualize, PDM | teilweise | Routing (17.4), Electronics-Stub (18.15), PlasticsService-Stub (Füllanalyse, Schweißlinien), VisualizationService (Render), Vault-Stub (PDM). |

---

# Teil 4: Creo-Referenz (20)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 20.1 | .PRT, .ASM, .DRW | teilweise | ImportExportService: CreoPrt/CreoAsm/CreoDrw, importCreoPrt/importCreoAsm, exportCreoPrt/exportCreoAsm (Stub: „use STEP“). |
| 20.2 | Top-Down, Skeleton-Modelle, Referenzsteuerung, Design Intent | vorhanden | Part: setSkeletonPartId/skeletonPartId; Serialisierung PART_SKELETON_ID. Abgeleitete Teile optional. |
| 20.3 | Relations, Family Tables, Feature Groups, UDFs | vorhanden | Relations (10, 11); loadPartConfigurationsFromCsv/savePartConfigurationsToCsv (CSV round-trip); UdfService saveUdf/insertUdf/listUdfs (Stub API produktiv). |
| 20.4–20.8 | Baugruppen, Flächen, Blech/Struktur, Simulation, Windchill | siehe 12–14, 17.1, 17.3, 17.6, 17.9 | |

---

# Teil 5: Siemens-NX-Referenz (21)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 21.1 | .prt (alles in einer Datei) | — | Eigenes Format kann single-file sein. |
| 21.2 | Parametrisch, Direkt, Synchronous Technology, History-frei | teilweise | Direktmodellierung (DirectEdit); „Synchronous“ = kombinierter parametrisch/direkt Modus; History-frei als Option (kein Feature-Baum, nur Körper). |
| 21.3–21.5 | Skizzen, Solid, Surfacing (Class-A, Studio, Through Curves, Boundary) | siehe 8, 9; offen für Class-A | Class-A-Surfacing = hohe Kontinuität (G2/G3), Studio-Surface-Tools. |
| 21.6 | Massive Assembly, Lightweight, Arrangement Manager, Constraints, Structure Navigator | siehe 12, 14; AssemblyManager LOD | Arrangement Manager = alternative Anordnungen/Varianten einer Baugruppe. |
| 21.7 | FEM, CFD, Multiphysics, Dynamik, Nichtlinear | teilweise | SimulationService: runCfdAnalysis, runMultiphysicsAnalysis (Stub). FEA/Motion vorhanden. |
| 21.8 | CAM: 2–5-Achs, Drehen, Mill-Turn, Robotik, Werkzeugpfade | teilweise | CamService-Stub: addTool, generateToolpath, exportGCode. Siehe 18.14. |
| 21.9 | Teamcenter, Versionierung, Produktstruktur, Änderungsprozesse | teilweise | VaultService-Stub: checkOut, checkIn, getVersionHistory. PDM-Anbindung später. |

---

# Teil 6: CATIA-Referenz (22)

| Ref | Punkt | Status | Was zu tun ist |
|-----|--------|--------|-----------------|
| 22.1 | .CATPart, .CATProduct, .CATDrawing | teilweise | ImportExportService: CatPart/CatProduct/CatDrawing, importCatPart/importCatProduct, exportCatPart/exportCatProduct (Stub: „use STEP“). |
| 22.2 | Workbenches: Part, Assembly, GSD, FreeStyle, Wireframe, Drafting, DMU, Kinematics, Composites | teilweise | WorkspaceType (app): Design, Assembly, Sketch, Drawing, SheetMetal, Weldment, TubePipe, Frame, Harness, Simulation, Surface, Freeform, Cam, Electronics, Generative, Render, Composites, Dmu, Kinematics. Umgebung (5.2) / Ribbon-Tabs. |
| 22.3 | Feature-basiert, Body, Boolean Bodies, Parametrik | siehe 9 | |
| 22.4 | Class-A-Flächen, G0–G3 | teilweise | SurfaceService-Stub (patch, trim, stitch, thicken). Kontinuitätsstufen G0–G3 später. |
| 22.5 | Große Strukturen, DMU Space Analysis, Clash, Mockups | siehe 14 (InterferenceChecker), 21.6 | |
| 22.6 | Kinematik: Mechanisms, Bewegung, Simulation, Animation | siehe 17.7 | |
| 22.7 | Composite Design: Faserverbund, Lagenaufbau, CFK | teilweise | CompositesService-Stub: createCompositePart, addPly, getPlies, exportLayupReport. |
| 22.8 | ENOVIA, 3DEXPERIENCE, Lebenszyklus, Freigaben, Revisionen | teilweise | VaultService-Stub (17.9, 21.9). PLM-Integration später. |

---

# Priorisierung (Vorschlag)

1. **Hoch (Kern-Workflow):** Skizze (8.2 Ellipse/Polygon/Spline/Text, 8.3 Symmetrisch/Fixiert, 8.5 Konfliktanzeige), Features (9.1 Extrusion Join/Cut, 9.3 Pfad-/Flächenmuster, 9.4 Arbeitselemente), Parameter (10), Baugruppen-Mates (13), Zeichnungen (16.1–16.3), Blech (17.1).  
2. **Mittel:** Startfenster/Vorlagen (3), Projektverwaltung (4), iLogic-ähnlich (11), Explosion (15), Simulation/Ergebnisse (17.6, 17.7), Rohr (17.4).  
3. **Niedrig / später:** Schweiß (17.2), Rahmen (17.3), Harness (17.5), Freiform (17.8), Vault/PDM (17.9), CAM (18.14), Electronics (18.15), Generative Design (18.10), Creo/NX/CATIA-spezifische Formate.

---

*Quelle: docs/REFERENZ_AUTODESK_INVENTOR.md. Dieses TODO wird bei Umsetzung der Punkte laufend angepasst (Status „offen“ → „teilweise“ → „vorhanden“).*
