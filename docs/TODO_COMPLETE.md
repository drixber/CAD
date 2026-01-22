# Vollständige TODO-Liste: Was noch zu tun ist

## 🔴 KRITISCH - Für funktionierende Anwendung

### 1. 3D Viewport Rendering (Coin3D/OCCT Integration)
- [ ] **Coin3D Integration**: SoQtExaminerViewer oder ähnliches integrieren
  - `Viewport3D::initializeViewport()` - Coin3D Viewer initialisieren
  - `Viewport3D::renderGeometry()` - Echte 3D-Geometrie rendern (nicht nur Text)
  - `Viewport3D::renderAssembly()` - Assembly-Komponenten als 3D-Modelle rendern
  - `Viewport3D::renderMbdAnnotations()` - PMI-Annotationen in 3D-Szene rendern
  - `Viewport3D::renderScene()` - Echte 3D-Projektion statt Text-Overlay
  - Ray-Casting für Object-Picking implementieren
  - OpenGL/Coin3D Scene Graph Management

- [ ] **OCCT (OpenCASCADE) Integration** (optional, aber empfohlen)
  - OCCT Shape-Erstellung für Geometrie
  - OCCT Viewer Integration
  - OCCT <-> Coin3D Konvertierung

### 2. FreeCAD Integration - Vollständige Implementierung
- [ ] **Sketch Synchronisation**:
  - `FreeCADAdapter::syncGeometry()` - Echte Geometrie-Entitäten zu FreeCAD Sketcher syncen
  - `FreeCADAdapter::syncConstraints()` - Constraint-Mapping zu FreeCAD Constraints
  - Sketch-Geometrie (Line, Circle, Arc, Rectangle) zu FreeCAD Sketcher-Geometrie konvertieren

- [ ] **Part Feature Synchronisation**:
  - Extrude zu FreeCAD PartDesign::Pad
  - Revolve zu FreeCAD PartDesign::Revolution
  - Hole zu FreeCAD PartDesign::Hole
  - Fillet zu FreeCAD PartDesign::Fillet
  - Loft zu FreeCAD PartDesign::Loft

- [ ] **Drawing Synchronisation**:
  - `TechDrawBridge::syncDrawing()` - Echte TechDraw-Seiten erstellen
  - `TechDrawBridge::syncDimensions()` - Dimensionen zu TechDraw Views hinzufügen
  - `TechDrawBridge::syncAssociativeLinks()` - Assoziative Links zwischen 3D und 2D

### 3. Import/Export - Echte Datei-Operationen
- [ ] **STEP Import/Export**:
  - `ImportExportService::importStep()` - STEP-Dateien lesen und in Assembly/Part konvertieren
  - `ImportExportService::exportStep()` - Assembly/Part zu STEP-Dateien exportieren
  - OCCT STEP Reader/Writer Integration

- [ ] **IGES Import/Export**:
  - `ImportExportService::importIges()` - IGES-Dateien lesen
  - `ImportExportService::exportIges()` - IGES-Dateien schreiben

- [ ] **STL Import/Export**:
  - `ImportExportService::importStl()` - STL-Mesh-Dateien lesen
  - `ImportExportService::exportStl()` - STL-Mesh-Dateien schreiben (ASCII/Binary)

- [ ] **DWG/DXF Import/Export**:
  - `ImportExportService::importDwg()` - AutoCAD DWG-Dateien lesen
  - `ImportExportService::importDxf()` - AutoCAD DXF-Dateien lesen
  - `ImportExportService::exportDwg()` / `exportDxf()` - AutoCAD-Format exportieren

- [ ] **Assembly File Loading**:
  - `AssemblyManager::loadAssembly()` - Echte Datei-Ladung statt Simulation
  - Dateiformat-Erkennung (STEP, IGES, native Format)
  - Progress-Tracking für große Dateien

### 4. Constraint Solver - Professionelle Implementierung
- [ ] **Sketch Constraint Solver**:
  - `Modeler::solveConstraints()` - PlanarGCS oder ähnlichen Solver integrieren
  - Vollständige Constraint-Typen: Parallel, Perpendicular, Tangent, Equal, Angle
  - Constraint-Validierung und Fehlerbehandlung
  - Over-constrained/Under-constrained Detection

- [ ] **Assembly Mate Solver**:
  - `Assembly::solveMates()` - Professioneller Constraint-Solver (z.B. SolveSpace-ähnlich)
  - Mate-Validierung
  - Degrees of Freedom (DOF) Berechnung

## 🟡 WICHTIG - Für vollständige Funktionalität

### 5. Simulation Service - Echte Berechnungen
- [ ] **FEA (Finite Element Analysis)**:
  - `SimulationService::runFeaAnalysis()` - Echte FEA-Berechnung (z.B. CalculiX, OpenFOAM)
  - Mesh-Generierung mit OCCT oder externem Tool
  - Material-Eigenschaften-Datenbank
  - Boundary Conditions (Fixed, Force, Pressure, Temperature)
  - Stress/Strain/Displacement Berechnung
  - Ergebnis-Visualisierung im Viewport

- [ ] **Motion Analysis**:
  - `SimulationService::runMotionAnalysis()` - Kinematik-Simulation
  - Joint-Definitionen
  - Trajectory-Berechnung

- [ ] **Deflection Analysis**:
  - `SimulationService::runDeflectionAnalysis()` - Durchbiegungs-Berechnung

- [ ] **Optimization**:
  - `SimulationService::runOptimization()` - Parameter-Optimierung

### 6. Drawing Generation - Echte Zeichnungserstellung
- [ ] **TechDraw Integration**:
  - `DrawingService::createDrawing()` - Echte TechDraw-Seiten erstellen
  - View-Erstellung (BaseView, SectionView, DetailView)
  - Dimension-Erstellung mit korrekten Werten
  - BOM-Integration in TechDraw
  - Sheet-Templates (ISO, ANSI, JIS)

- [ ] **Drawing Updates**:
  - `AssociativeLinkService::updateFromModel()` - Echte assoziative Updates
  - Automatische Dimension-Updates bei Modelländerungen
  - View-Updates bei Feature-Änderungen

### 7. Sheet Metal Service - Echte Operationen
- [ ] **Sheet Metal Features**:
  - `SheetMetalService::applyOperation()` - Echte Flange, Face, Cut, Bend Operationen
  - Unfold/Refold Berechnung
  - K-Factor Berechnung
  - Flat Pattern Generation

### 8. Routing Service - Echte Pfadfindung
- [ ] **Route Generation**:
  - `RoutingService::createRoute()` - Echte Pfadfindung-Algorithmen
  - Obstacle-Detection
  - Route-Optimierung (kürzester Weg, Biegeradius)
  - Rigid Pipe, Flexible Hose, Bent Tube Berechnung

### 9. Direct Edit Service - Echte Geometrie-Manipulation
- [ ] **Direct Editing**:
  - `DirectEditService::applyEdit()` - Echte Face-Modifikation
  - MoveFace, OffsetFace, DeleteFace Operationen
  - Freeform Deformation
  - Feature-History-Update

### 10. Pattern Service - Echte Pattern-Generierung
- [ ] **Pattern Operations**:
  - `PatternService::createPattern()` - Echte Pattern-Erstellung
  - Rectangular, Circular, Curve-Driven Patterns
  - Pattern-Instanzen korrekt positionieren

### 11. Simplify Service - Echte Vereinfachung
- [ ] **Simplification**:
  - `SimplifyService::simplify()` - Echte Geometrie-Vereinfachung
  - Feature-Removal
  - Detail-Level-Reduktion
  - Geometry-Comparison für Komplexitäts-Messung

### 12. Visualization Service - Echte Visualisierung
- [ ] **Visualization Modes**:
  - `VisualizationService::runVisualization()` - Echte Rendering-Pipeline
  - Illustration Mode (technische Zeichnung)
  - Rendering Mode (photorealistisch)
  - Animation (Keyframe-basiert)

### 13. MBD (Model-Based Definition) - Vollständige Implementierung
- [ ] **PMI Rendering**:
  - `MbdService::prepareForRendering()` - Echte 3D-PMI-Darstellung
  - Datum-Symbole rendern
  - Tolerance-Callouts rendern
  - Annotation-Leader-Lines rendern
  - Visibility-Modes (Always, OnSelection, OnHover, Hidden)

## 🟢 ERWEITERT - Für professionelle Nutzung

### 14. Performance & Optimierung
- [ ] **Assembly Loading**:
  - Echte asynchrone Background-Loading
  - Progress-Tracking für große Assemblies
  - Memory-Management für große Modelle
  - LOD-System vollständig implementieren (echte Geometrie-Reduktion)

- [ ] **Viewport Performance**:
  - Frustum Culling
  - Level-of-Detail (LOD) Rendering
  - Occlusion Culling
  - Multi-threaded Rendering

- [ ] **Caching**:
  - Geometry-Cache für wiederholte Berechnungen
  - Render-Cache für Viewport
  - File-Cache für Import/Export

### 15. Undo/Redo System
- [ ] **Command History**:
  - `UndoStack` vollständig implementieren
  - Command-Pattern für alle Operationen
  - State-Snapshots für komplexe Operationen
  - Memory-Efficient Undo/Redo

### 16. Crash Reporter
- [ ] **Error Handling**:
  - `CrashReporter` vollständig implementieren
  - Stack-Trace-Erfassung
  - Crash-Dumps generieren
  - Error-Reporting an Server

### 17. Update Service
- [ ] **Update Mechanism**:
  - `UpdateService::checkForUpdates()` - Echte Server-Abfrage (JSON-API)
  - `UpdateService::downloadUpdate()` - HTTP-Download implementieren
  - `UpdateService::installUpdate()` - Installer-Ausführung
  - Update-Verification (Checksums)
  - Rollback-Mechanismus

### 18. Python Bindings
- [ ] **pybind11 Integration**:
  - Python-Module vollständig binden
  - API-Dokumentation für Python
  - Beispiel-Scripts
  - Workflow-Engine Python-Integration

### 19. Dokumentation
- [ ] **API-Dokumentation**:
  - Doxygen/Similar für alle Module
  - Code-Beispiele
  - Tutorials
  - Architecture-Diagramme

- [ ] **User-Dokumentation**:
  - Benutzerhandbuch
  - Quick-Start Guide
  - Video-Tutorials
  - FAQ

### 20. Testing
- [ ] **Unit Tests**:
  - Tests für alle Services
  - Tests für Core-Module (erweitern)
  - Tests für UI-Komponenten
  - Code-Coverage > 80%

- [ ] **Integration Tests**:
  - End-to-End Tests
  - FreeCAD-Integration Tests
  - Import/Export Tests
  - Performance Tests

- [ ] **Regression Tests**:
  - Test-Suite für bekannte Bugs
  - Automated Testing Pipeline

### 21. Build & Deployment
- [ ] **Build-System**:
  - CI/CD Pipeline (GitHub Actions, etc.)
  - Automated Testing in CI
  - Multi-Platform Builds (Windows, Linux, macOS)
  - Dependency-Management (vcpkg Integration)

- [ ] **Installer**:
  - NSIS-Installer vollständig konfigurieren
  - Update-Mechanismus im Installer
  - Uninstaller
  - Desktop-Shortcuts
  - File-Associations

- [ ] **Packaging**:
  - Release-Packages erstellen
  - Versioning-System
  - Changelog-Generierung

## 📊 Priorisierung

### Phase 8 (Nächste kritische Schritte):
1. **3D Viewport Rendering** (Coin3D Integration) - 🔴 KRITISCH
2. **FreeCAD Geometry Sync** - 🔴 KRITISCH  
3. **Import/Export (STEP)** - 🔴 KRITISCH
4. **Constraint Solver** - 🔴 KRITISCH

### Phase 9 (Wichtige Features):
5. **Simulation Service** (FEA)
6. **Drawing Generation** (TechDraw)
7. **Assembly File Loading**

### Phase 10 (Erweiterte Features):
8. **Sheet Metal, Routing, Direct Edit** vollständig
9. **Performance-Optimierung**
10. **Undo/Redo System**

### Phase 11 (Polish & Deployment):
11. **Testing & Dokumentation**
12. **Build-Automatisierung**
13. **Installer-Finalisierung**

## Geschätzter Aufwand

- **Phase 8**: 20-30 Wochen
- **Phase 9**: 15-20 Wochen  
- **Phase 10**: 15-20 Wochen
- **Phase 11**: 10-15 Wochen

**Gesamt**: 60-85 Wochen (ca. 1.5-2 Jahre bei Vollzeit-Entwicklung)

## Notizen

- Viele Services sind aktuell "Stubs" - sie simulieren Operationen, führen aber keine echten Berechnungen durch
- Viewport verwendet aktuell QPainter (2D) statt 3D-Rendering
- FreeCAD-Integration ist teilweise vorhanden, aber Geometry-Sync fehlt
- Import/Export gibt nur "queued" zurück, keine echten Datei-Operationen
- Constraint Solver ist sehr einfach - braucht professionelle Bibliothek

