# Plan: Eigener CAD-Geometriekern (Option 2) – Abarbeitbare TODO-Liste

Vollständig **eigener CAD-Geometriekern** (B-Rep) von Grund auf und Integration in Hydra CAD – ohne externe CAD-Backends (Open CASCADE optional).

---

## Implementierungsstand (Build abgeschlossen)

**Build:** Eigen-Kern ist **Standard** (`CAD_USE_EIGENER_KERN=ON`). Target `cad_eigen_kernel` (statische Bibliothek unter `src/core/kernel/`).

**Umgesetzt:**
- **Phase 0–3:** Math, 2D-Geometrie, Topologie, 3D-Geometrie (Line3D, Circle3D, PlaneSurface, CylinderSurface, SphereSurface).
- **Phase 4:** WireBuilder (Sketch → Wire2D), FaceBuilder (Wire2D → planare Face).
- **Phase 5:** SolidBuilder: Box vollständig, Extrude (planare Face + Richtung + Länge), **Cylinder** vollständig (B-Rep: Boden, Deckel, Mantel), **Revolve** (aus Face-Bounding-Box → Zylinder-Approximation), sphere weiter Stub.
- **Phase 6–8:** Boolean (fuse/cut/common als Stub – cut gibt aktuell Basis-Solid zurück), Fillet/Chamfer als **Stubs** (Durchreichen), Shell, Draft, Loft, Sweep als Stubs.
- **Phase 9:** MeshGenerator (Triangulierung), StlWriter, StlReader.
- **Phase 10:** KernelBridge: `buildPartFromSketch(sketch)`, **`buildPartFromPart(part, sketches)`** (erster Extrude/Revolve = Basis, danach Hole/Fillet/Chamfer angewendet). AppController: Extrude/Revolve rufen `buildPartFromPart` auf; Status „Eigen-Kern on“. TechDrawBridge (intern, kein externes Backend).

**Hinweis:** Target heißt `cad_eigen_kernel` (nicht `cad_kernel`), da `src/kernel` bereits ein Target `cad_kernel` hat. Doppelte Deklaration `path_sketch_id` in `Part.h` wurde behoben.

---

## So arbeitest du das ab

1. **Reihenfolge:** Phasen 0 → 1 → 2 → … → 10. Innerhalb einer Phase die Tasks in der angegebenen Reihenfolge.
2. **Eine Aufgabe = eine Checkbox:** Task ausführen → **Prüfen** (siehe „Prüfen:“) → Checkbox auf `- [x]` setzen.
3. **Unterpunkte zuerst:** Übergeordnete Checkbox (z. B. „Phase 0 abgeschlossen“) erst setzen, wenn alle Unterpunkte erledigt sind.
4. **Tests:** Pro Schritt wo „Tests“ steht: Testdatei in `tests/kernel/` (oder `tests/core/kernel/`) anlegen, CMake anbinden, `ctest` oder Test-Runner ausführen. „Tests grün“ = alle Tests dieser Phase laufen durch.
5. **Build:** Nach jeder Phase `cmake --build build -DCAD_USE_EIGENER_KERN=ON` (oder eure Build-Konfiguration) soll ohne Fehler durchlaufen.

**Task-ID:** Jeder Task hat eine ID (z. B. `T-0.1.1`). Du kannst im Commit oder in der Doku referenzieren: „T-4.1.3 erledigt“.

**Definition of Done pro Phase:** Siehe am Ende jeder Phase den Kasten **„Phase X erledigt wenn:“**.

---

## Fortschritts-Übersicht (Master-Checkliste)

| Phase | Inhalt | Status |
|-------|--------|--------|
| **0** | Grundlagen (Math & Toleranzen) | [ ] |
| **1** | 2D-Geometrie (Kurven, Wire2D) | [ ] |
| **2** | Topologie (Vertex … Solid, Shape) | [ ] |
| **3** | 3D-Geometrie (Kurven, Surfaces) | [ ] |
| **4** | Sketch → Wire2D → planare Face | [ ] |
| **5** | Solide (Primitiven, Extrude, Revolve) | [ ] |
| **6** | Boolean-Operationen | [ ] |
| **7** | Fillet & Chamfer | [ ] |
| **8** | Erweiterte Operationen (optional) | [ ] |
| **9** | Import/Export (STL, STEP) | [ ] |
| **10** | Integration in Hydra CAD | [ ] |

**Abhängigkeiten:** 1←0, 2←0, 3←0+2, 4←1+2+3, 5←2+3+4, 6←2+3+5, 7←5+6, 8←5+6+7, 9←5+6, 10←4+5+6+7+9.

---

## 1. Voraussetzungen vor Start

- [ ] **T-VOR-1** Mathematik: Lineare Algebra, analytische Geometrie, Differentialgeometrie (Kurven/Flächen) verfügbar oder eingeplant.
- [ ] **T-VOR-2** Literatur: Hoffmann (Geometric and Solid Modeling) bzw. Abschnitt Literatur bereitgestellt.
- [ ] **T-VOR-3** Build: C++17, CMake; Repo lässt sich bauen (z. B. `cmake -B build`, `cmake --build build`).
- [ ] **T-VOR-4** Optional: Eigen oder eigene Vektor/Matrix-Klassen entschieden (ja/nein).

---

## 2. Projektstruktur anlegen

- [ ] **T-2.1** Ordner `src/core/kernel/` anlegen.  
  **Prüfen:** Verzeichnis existiert.

- [ ] **T-2.2** Datei `src/core/kernel/CMakeLists.txt` anlegen mit:
  - `add_library(cad_kernel STATIC)` (oder SHARED)
  - `target_include_directories(cad_kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})`
  - Noch keine `target_sources` (kommen in Phase 0).  
  **Prüfen:** CMake konfiguriert ohne Fehler, wenn `add_subdirectory(kernel)` in `src/core/CMakeLists.txt` nur bedingt ausgeführt wird (siehe T-2.4).

- [ ] **T-2.3** Unterordner anlegen: `math/`, `geometry2d/`, `topology/`, `geometry3d/`, `builder/`, `boolean/`, `fillet/`, `advanced/`, `io/` unter `src/core/kernel/`.  
  **Prüfen:** Alle Ordner existieren.

- [ ] **T-2.4** In `src/core/CMakeLists.txt`:
  - `option(CAD_USE_EIGENER_KERN "Build eigenes CAD-Kern-Modul" OFF)`
  - `if(CAD_USE_EIGENER_KERN) add_subdirectory(kernel) endif()`
  - Optional: `target_link_libraries(cad_core PRIVATE cad_kernel)` wenn `CAD_USE_EIGENER_KERN` ON.  
  **Prüfen:** Mit `-DCAD_USE_EIGENER_KERN=OFF` baut Projekt wie bisher; mit ON baut `cad_kernel` (noch leer).

---

## 3. Phase 0: Grundlagen (Math & Toleranzen)

**Ziel:** Vektoren, Transformationen, Toleranzen – einheitlich im ganzen Kern.

### 3.1 Vektoren und Punkte

- [ ] **T-0.1.1** Datei `src/core/kernel/math/Vector2.h` anlegen: Struct/Class `Vector2` mit `double x, y`; Methoden `length()`, `normalized()`, `dot(Vector2)`, `angleTo(Vector2)` (Radiant). Namespace z. B. `cad::kernel::math`.  
  **Prüfen:** Include in einer .cpp kompiliert fehlerfrei.

- [ ] **T-0.1.2** Datei `src/core/kernel/math/Vector3.h` anlegen: Struct/Class `Vector3` mit `double x, y, z`; Methoden `length()`, `normalized()`, `dot(Vector3)`, `cross(Vector3)`.  
  **Prüfen:** Wie T-0.1.1.

- [ ] **T-0.1.3** Punkt-Typen: Entweder `using Point2 = Vector2; using Point3 = Vector3;` in einer gemeinsamen Header-Datei (z. B. `Types.h` unter `math/`) oder eigene Structs `Point2`/`Point3` mit gleichem Layout.  
  **Prüfen:** Punkt-Werte können als Vektor verwendet werden (Komponentenzugriff).

- [ ] **T-0.1.4** Testdatei `tests/kernel/math/Vector2Test.cpp` (oder unter `tests/core/kernel/`) anlegen: Test für `length()`, `normalized()`, `dot`, `angleTo` (z. B. 90° zwischen (1,0) und (0,1)).  
  **Prüfen:** Test ausführbar und grün.

- [ ] **T-0.1.5** Testdatei für Vector3: Kreuzprodukt (Rechtssystem), z. B. (1,0,0) × (0,1,0) = (0,0,1).  
  **Prüfen:** Test grün.

- [ ] **T-0.1.6** In `kernel/CMakeLists.txt` Sources eintragen: `math/Vector2.h`, `math/Vector3.h` (und ggf. `math/Types.h`); Test-Target anlegen und mit `cad_kernel` linken.  
  **Prüfen:** Build + Tests laufen.

### 3.2 Matrizen und Transformationen

- [ ] **T-0.2.1** Datei `src/core/kernel/math/Matrix3.h` anlegen: 3×3-Matrix; Rotation um X/Y/Z (Radiant), Skalierung; keine Translation. Methoden: `apply(Vector3)` für Richtungen.  
  **Prüfen:** Kompiliert.

- [ ] **T-0.2.2** Datei `src/core/kernel/math/Transform3.h` anlegen: Translation + Rotation (4×4 oder 3×3 + Vector3); Methode `apply(Point3) -> Point3`; Factory z. B. `rotateZ(angle)`, `translate(dx,dy,dz)`.  
  **Prüfen:** Kompiliert; `apply` verschiebt Punkt wie erwartet.

- [ ] **T-0.2.3** Unit-Tests: Rotation 90° um Z-Achse auf (1,0,0) → (0,1,0); Inverse * Original = Identität; Kombination translate + rotate.  
  **Prüfen:** Tests grün.

- [ ] **T-0.2.4** Matrix3/Transform3 in `kernel/CMakeLists.txt` aufnehmen; Tests in Test-Target.  
  **Prüfen:** Build + Tests grün.

### 3.3 Toleranzen

- [ ] **T-0.3.1** Datei `src/core/kernel/math/Tolerance.h` anlegen: Konstanten z. B. `kDistanceTolerance = 1e-7`, `kAngularTolerance` (Radiant), `kParamTolerance` (für Kurvenparameter).  
  **Prüfen:** Include in anderem Modul möglich.

- [ ] **T-0.3.2** Hilfsfunktionen: `bool pointsEqual(Point3 a, Point3 b)`, `bool vectorsParallel(Vector3 a, Vector3 b)` unter Verwendung von `Tolerance.h`.  
  **Prüfen:** Unit-Test: zwei Punkte mit Abstand < 1e-7 gelten als gleich.

- [ ] **T-0.3.3** Kurzdokumentation im Header oder in README: „Im Kern keine rohen `==` für double; Vergleiche über Tolerance.“  
  **Prüfen:** Doku vorhanden.

### 3.4 CMake Phase 0 abschließen

- [ ] **T-0.4.1** Alle Phase-0-Dateien in `cad_kernel` eintragen (Vector2, Vector3, Matrix3, Transform3, Tolerance; ggf. Types.h).  
  **Prüfen:** `cmake --build build` mit `CAD_USE_EIGENER_KERN=ON` erfolgreich.

- [ ] **T-0.4.2** Alle Phase-0-Tests in einem Test-Target (z. B. `cad_kernel_test`) und in CTest registrieren.  
  **Prüfen:** `ctest` oder Test-Runner zeigt alle Phase-0-Tests grün.

- [ ] **T-0** **Phase 0 abgeschlossen:** Math + Toleranzen im Repo; Build mit `CAD_USE_EIGENER_KERN=ON` und alle Phase-0-Tests grün.

**Phase 0 erledigt wenn:** Build läuft, alle genannten Tests existieren und sind grün, Toleranzen und Hilfsfunktionen nutzbar.

---

## 4. Phase 1: 2D-Geometrie (Kurven und Wire)

**Ziel:** Curve2D, Line2D, Circle2D, Arc2D, Spline2D (Polyline), Wire2D – geschlossener Wire aus mehreren Kurven.

### 4.1 Basis-Kurve und BoundingBox

- [ ] **T-1.1.1** Struct `BoundingBox2` in `math/` oder `geometry2d/` definieren: `minX, maxX, minY, maxY` (oder zwei Point2).  
  **Prüfen:** Kompiliert.

- [ ] **T-1.1.2** Datei `src/core/kernel/geometry2d/Curve2D.h`: Abstrakte Basisklasse mit virtuellen Methoden `Point2 pointAt(double t)`, `Vector2 tangentAt(double t)`; optional `double length()`, `BoundingBox2 bounds()`, `clone()`, `reverse()`.  
  **Prüfen:** Abgeleitete Klasse kann implementieren.

- [ ] **T-1.1.3** Curve2D (und BoundingBox2) in `kernel/CMakeLists.txt` eintragen.  
  **Prüfen:** Build.

### 4.2 Line2D

- [ ] **T-1.2.1** `geometry2d/Line2D.h` und `.cpp`: Klasse erbt von Curve2D; Speicherung Startpunkt, Endpunkt; `pointAt(t) = start + t*(end-start)`, t∈[0,1]; `tangentAt` konstant.  
  **Prüfen:** Kompiliert.

- [ ] **T-1.2.2** Tests: Punkt auf Linie (z. B. t=0.5 = Mittelpunkt); Tangentenrichtung; Länge = Abstand start–end.  
  **Prüfen:** Tests grün.

- [ ] **T-1.2.3** Line2D in CMake.  
  **Prüfen:** Build + Tests.

### 4.3 Circle2D

- [ ] **T-1.3.1** `geometry2d/Circle2D.h` und `.cpp`: Mittelpunkt, Radius; `pointAt(t) = center + radius*(cos(2πt), sin(2πt))`, t∈[0,1]; `tangentAt` ableiten.  
  **Prüfen:** Kompiliert.

- [ ] **T-1.3.2** Tests: Punkt auf Kreis (Abstand = Radius); Umfang 2πr.  
  **Prüfen:** Tests grün.

- [ ] **T-1.3.3** Circle2D in CMake.  
  **Prüfen:** Build + Tests.

### 4.4 Arc2D

- [ ] **T-1.4.1** `geometry2d/Arc2D.h` und `.cpp`: Mittelpunkt, Radius, startAngle, endAngle (Radiant); `pointAt(t)` interpoliert zwischen startAngle und endAngle.  
  **Prüfen:** Kompiliert.

- [ ] **T-1.4.2** Tests: Endpunkte bei t=0 und t=1; Tangenten an den Enden.  
  **Prüfen:** Tests grün.

- [ ] **T-1.4.3** Arc2D in CMake.  
  **Prüfen:** Build + Tests.

### 4.5 Spline2D (Polyline)

- [ ] **T-1.5.1** `geometry2d/Spline2D.h` und `.cpp`: Polyline – Folge von Point2; `pointAt(t)` lineare Interpolation zwischen Segmenten, t∈[0,1] über gesamte Polyline.  
  **Prüfen:** Kompiliert.

- [ ] **T-1.5.2** Tests: Geschlossenheit (erster Punkt = letzter); Gesamtlänge.  
  **Prüfen:** Tests grün.

- [ ] **T-1.5.3** Spline2D in CMake.  
  **Prüfen:** Build + Tests.

### 4.6 Wire2D

- [ ] **T-1.6.1** `geometry2d/Wire2D.h` und `.cpp`: `std::vector<std::shared_ptr<Curve2D>>` (oder eigene Handle-Klasse); `add(std::shared_ptr<Curve2D>)`; `isClosed()` (Endpunkt letzte Kurve ≈ Startpunkt erste, mit Tolerance); `bounds()`, `length()`.  
  **Prüfen:** Kompiliert.

- [ ] **T-1.6.2** Tests: Rechteck aus 4 Line2D → `isClosed() == true`, length = Umfang; ein Circle2D → ein geschlossener Wire, length = 2πr.  
  **Prüfen:** Tests grün.

- [ ] **T-1.6.3** Wire2D in CMake.  
  **Prüfen:** Build + Tests.

- [ ] **T-1** **Phase 1 abgeschlossen:** Wire2D aus Linie, Kreis, Bogen (und optional Spline); geschlossener Wire; alle Phase-1-Tests grün.

**Phase 1 erledigt wenn:** Alle Curve2D-Typen und Wire2D gebaut, getestet und in CMake eingetragen.

---

## 5. Phase 2: Topologie (B-Rep-Datenstrukturen)

**Ziel:** Vertex, Edge, Wire, Loop, Face, Shell, Solid, Shape + Explorer. Edge/Face können vorerst Platzhalter-Geometrie (nullptr oder Stub) referenzieren; Phase 3 füllt echte Geometrie.

### 5.1 Typen und IDs

- [ ] **T-2.1.1** `kernel/topology/Types.h`: Enum `ShapeType { Vertex, Edge, Wire, Face, Shell, Solid, Compound }`; ID-Typ festlegen (z. B. `using ShapeId = size_t` oder `std::string`).  
  **Prüfen:** Kompiliert; von anderen Topologie-Headern inkludierbar.

- [ ] **T-2.1.2** Types.h in CMake.  
  **Prüfen:** Build.

### 5.2 Vertex

- [ ] **T-2.2.1** `topology/Vertex.h` und `.cpp`: `Point3 position`; `ShapeId id`; Methoden `point()`, `id()`.  
  **Prüfen:** Kompiliert.

- [ ] **T-2.2.2** Vertex in CMake.  
  **Prüfen:** Build.

### 5.3 Edge (mit Kurven-Stub)

- [ ] **T-2.3.1** `topology/Edge.h` und `.cpp`: Referenz auf `Curve3D*` (forward-decl oder Interface aus Phase 3); Start-Vertex, End-Vertex; Parameterbereich `t0, t1`. Methoden: `curve()`, `startVertex()`, `endVertex()`, `pointAt(t)`. Für Phase 2: Curve3D darf nullptr oder Stub sein, `pointAt` kann linear zwischen start/end interpoliert werden.  
  **Prüfen:** Kompiliert; Edge mit zwei Vertices erstellbar.

- [ ] **T-2.3.2** Edge in CMake.  
  **Prüfen:** Build.

### 5.4 Wire

- [ ] **T-2.4.1** `topology/Wire.h` und `.cpp`: Geordnete Liste von Edges; `isClosed()` (Ende letzte Edge ≈ Start erste Edge, Toleranz); `edges()`, `vertices()`.  
  **Prüfen:** Kompiliert.

- [ ] **T-2.4.2** Wire in CMake.  
  **Prüfen:** Build.

### 5.5 Loop

- [ ] **T-2.5.1** `topology/Loop.h` und `.cpp`: Wrapper um einen geschlossenen Wire als Rand einer Face (außen oder Loch).  
  **Prüfen:** Kompiliert.

- [ ] **T-2.5.2** Loop in CMake.  
  **Prüfen:** Build.

### 5.6 Face

- [ ] **T-2.6.1** `topology/Face.h` und `.cpp`: Referenz auf `Surface*` (forward-decl); Liste von Loops (erster = äußerer Rand, Rest = Löcher); `surface()`, `outerLoop()`, `innerLoops()`, `normalAt(u,v)` (über Surface, kann Stub sein).  
  **Prüfen:** Kompiliert.

- [ ] **T-2.6.2** Face in CMake.  
  **Prüfen:** Build.

### 5.7 Shell

- [ ] **T-2.7.1** `topology/Shell.h` und `.cpp`: Liste von Faces; Invariante: jede Kante max. zwei Faces.  
  **Prüfen:** Kompiliert.

- [ ] **T-2.7.2** Shell in CMake.  
  **Prüfen:** Build.

### 5.8 Solid

- [ ] **T-2.8.1** `topology/Solid.h` und `.cpp`: `outerShell()`; optional innere Shells; Methoden `volume()` und `bounds()` können vorerst 0 bzw. leeren Bereich zurückgeben.  
  **Prüfen:** Kompiliert.

- [ ] **T-2.8.2** Solid in CMake.  
  **Prüfen:** Build.

### 5.9 Shape und Explorer

- [ ] **T-2.9.1** `topology/Shape.h` und `.cpp`: Gemeinsamer Typ (z. B. `std::variant<Vertex, Edge, Wire, Face, Shell, Solid>` oder Basisklasse + Downcast); `type()`, `isNull()`, `vertex()`, `edge()`, `face()`, `solid()` etc.  
  **Prüfen:** Kompiliert; Shape kann Solid halten, type() == Solid.

- [ ] **T-2.9.2** Explorer: z. B. `std::vector<Edge> getEdges(const Solid&)`; `std::vector<Face> getFaces(const Shell&)`.  
  **Prüfen:** Kompiliert; Test: Box (später) liefert 12 Edges, 6 Faces.

- [ ] **T-2.9.3** Shape + Explorer in CMake.  
  **Prüfen:** Build.

- [ ] **T-2** **Phase 2 abgeschlossen:** Vertex, Edge, Wire, Loop, Face, Shell, Solid, Shape + Explorer; Build grün.

**Phase 2 erledigt wenn:** Alle Topologie-Klassen vorhanden, Edge/Face können Stub-Geometrie haben; Explorer liefert Sub-Shapes.

---

## 6. Phase 3: 3D-Geometrie (Kurven und Flächen)

**Ziel:** Curve3D, Line3D, Circle3D, Surface, PlaneSurface, CylinderSurface, SphereSurface (optional TorusSurface); Anbindung an Edge/Face.

### 6.1 Curve3D und Line3D

- [ ] **T-3.1.1** `geometry3d/Curve3D.h`: Interface `pointAt(double t)`, `tangentAt(double t)`, Parameterbereich (tMin, tMax).  
  **Prüfen:** Kompiliert.

- [ ] **T-3.2.1** `geometry3d/Line3D.h` und `.cpp`: Startpunkt, Endpunkt; implementiert Curve3D.  
  **Prüfen:** Kompiliert; Edge kann Line3D nutzen.

- [ ] **T-3.1.2 + T-3.2.2** Curve3D + Line3D in CMake.  
  **Prüfen:** Build.

### 6.2 Circle3D

- [ ] **T-3.3.1** `geometry3d/Circle3D.h` und `.cpp`: Mittelpunkt, Radius, Achse (Vector3); `pointAt(t)` mit t∈[0,2π] oder [0,1] skaliert.  
  **Prüfen:** Kompiliert.

- [ ] **T-3.3.2** Circle3D in CMake.  
  **Prüfen:** Build.

### 6.3 Surface und PlaneSurface

- [ ] **T-3.4.1** `geometry3d/Surface.h`: Interface `pointAt(u,v)`, `normalAt(u,v)`, Parameterbereich.  
  **Prüfen:** Kompiliert.

- [ ] **T-3.5.1** `geometry3d/PlaneSurface.h` und `.cpp`: Ursprung + zwei Vektoren u_axis, v_axis (oder Origin + Normal); u,v → Punkt.  
  **Prüfen:** Kompiliert; Face kann PlaneSurface nutzen.

- [ ] **T-3.4.2 + T-3.5.2** Surface + PlaneSurface in CMake.  
  **Prüfen:** Build.

### 6.4 CylinderSurface und SphereSurface

- [ ] **T-3.6.1** `geometry3d/CylinderSurface.h` und `.cpp`: Achse (Point3 + Vector3), Radius; u = Winkel, v = Höhe.  
  **Prüfen:** Kompiliert.

- [ ] **T-3.7.1** `geometry3d/SphereSurface.h` und `.cpp`: Mittelpunkt, Radius; u = Längengrad, v = Breitengrad.  
  **Prüfen:** Kompiliert.

- [ ] **T-3.6.2 + T-3.7.2** CylinderSurface + SphereSurface in CMake.  
  **Prüfen:** Build.

### 6.5 TorusSurface (optional)

- [ ] **T-3.8.1** `geometry3d/TorusSurface.h` und `.cpp`: großer Radius R, kleiner Radius r; Standard-Parameterisierung. In CMake.  
  **Prüfen:** Build.

### 6.6 Anbindung Topologie ↔ Geometrie

- [ ] **T-3.9.1** Edge: Curve3D + t0/t1 verbindlich nutzen; Vertex-Position aus curve->pointAt(t0) bzw. pointAt(t1).  
  **Prüfen:** Edge mit Line3D/Circle3D befüllt, Vertices an Enden korrekt.

- [ ] **T-3.9.2** Face: Surface + Loops; Loops sind Wires, deren Edges auf der Fläche liegen. Plane-Face: 2D (u,v) ↔ 3D-Punkte in Ebene.  
  **Prüfen:** Eine planare Face mit einem Loop aus 4 Edges (Rechteck) darstellbar.

- [ ] **T-3** **Phase 3 abgeschlossen:** Line3D, Circle3D, PlaneSurface, CylinderSurface, SphereSurface (+ optional Torus); Edges/Faces mit echter Geometrie; Build grün.

**Phase 3 erledigt wenn:** Alle 3D-Kurven und -Flächen implementiert, in Topologie eingebunden, Build erfolgreich.

---

## 7. Phase 4: Sketch → Wire2D → planare Face

**Ziel:** Aus `cad::core::Sketch` (Modeler/Sketch.h) Wire2D bauen; aus Wire2D eine planare Face.

### 7.1 WireBuilder

- [ ] **T-4.1.1** `kernel/builder/WireBuilder.h` und `.cpp` anlegen. Include `core/Modeler/Sketch.h` (Include-Pfad in CMake: kernel darf auf `src/core` zugreifen).  
  **Prüfen:** Kompiliert.

- [ ] **T-4.1.2** WireBuilder::build(sketch): Über `sketch.geometry()` iterieren; je GeometryType: Line→Line2D, Circle→Circle2D, Arc→Arc2D, Rectangle→4×Line2D, Ellipse→Polygon-Approximation (oder Spline2D), Polygon/Spline→Spline2D. Reihenfolge so, dass geschlossener Wire entsteht (heuristisch: Endpunkt = Startpunkt nächste). Rückgabe: `Wire2D` oder `std::vector<Wire2D> buildWires(sketch)`.  
  **Prüfen:** Kompiliert; Test mit Sketch mit einem Rechteck liefert einen geschlossenen Wire2D mit 4 Kurven.

- [ ] **T-4.1.3** Test: Sketch mit einem Kreis → ein Wire2D aus einem Circle2D.  
  **Prüfen:** Test grün.

- [ ] **T-4.1.4** WireBuilder in CMake; Include-Pfad für Modeler.  
  **Prüfen:** Build + Tests.

### 7.2 FaceBuilder

- [ ] **T-4.2.1** `kernel/builder/FaceBuilder.h` und `.cpp`: `Face buildPlanarFace(Wire2D wire, PlaneSurface plane)` (oder Ebene Z=0). Wire2D (x,y) → 3D (x,y,0); jede 2D-Kurve in 3D-Kurve in Ebene (Line2D→Line3D, Circle2D→Circle3D in Ebene); aus 3D-Kurven Edges + Vertices + Wire + Loop + Face bauen.  
  **Prüfen:** Kompiliert.

- [ ] **T-4.2.2** Test: Rechteck-Wire2D → eine Face mit 4 Edges, 4 Vertices, 1 Loop.  
  **Prüfen:** Test grün.

- [ ] **T-4.2.3** FaceBuilder in CMake.  
  **Prüfen:** Build + Tests.

- [ ] **T-4** **Phase 4 abgeschlossen:** WireBuilder(Sketch)→Wire2D; FaceBuilder(Wire2D)→planare Face; Tests grün.

**Phase 4 erledigt wenn:** Sketch (Rechteck/Kreis) → Wire2D → planare Face durchgängig getestet.

---

## 8. Phase 5: Solide (Primitiven, Extrude, Revolve)

**Ziel:** SolidBuilder mit box, cylinder, sphere, extrude, revolve; Volumen/Bounds wo sinnvoll.

### 8.1 SolidBuilder-Rahmen

- [ ] **T-5.1.1** `kernel/builder/SolidBuilder.h` und `.cpp` anlegen. Signatur: `Solid box(double wx, double wy, double hz)`, `Solid cylinder(double r, double h)`, `Solid sphere(double r)`, `Solid extrude(Face base, Vector3 direction, double length)`, `Solid revolve(Face base, Axis axis, double angle)` (Axis = Point3 + Vector3).  
  **Prüfen:** Kompiliert (Implementierungen können Stub sein).

- [ ] **T-5.1.2** SolidBuilder in CMake.  
  **Prüfen:** Build.

### 8.2 Box

- [ ] **T-5.2.1** box(wx,wy,hz): 8 Vertices, 12 Edges (Line3D), 6 Faces (PlaneSurface), je 1 Loop mit 4 Edges; 1 Shell, 1 Solid.  
  **Prüfen:** Kompiliert.

- [ ] **T-5.2.2** Volume für Quader: L*W*H. Solid::volume() für Box implementieren (oder in SolidBuilder Hilfsfunktion).  
  **Prüfen:** box(10,20,5).volume() == 1000 (oder getVolume(solid)==1000).

- [ ] **T-5.2.3** Test: Box(10,20,5) → Solid, 6 Faces, 12 Edges, Volumen 1000; bounds prüfen.  
  **Prüfen:** Test grün.

### 8.3 Zylinder

- [ ] **T-5.3.1** cylinder(r,h): 2 Kreisfaces (Deckel), 1 Mantel (CylinderSurface); Kanten: 2×Circle3D, 4×Line3D; Vertices passend.  
  **Prüfen:** Kompiliert.

- [ ] **T-5.3.2** Test: cylinder(5,10) → Solid, Volumen π*25*10.  
  **Prüfen:** Test grün.

### 8.4 Kugel

- [ ] **T-5.4.1** sphere(r): Einfache Variante (z. B. eine Face mit SphereSurface + parametrischem Rand, oder UV-Grid aus planaren/zyklischen Faces).  
  **Prüfen:** Kompiliert.

- [ ] **T-5.4.2** Test: sphere(1) → Solid, Volumen (4/3)π.  
  **Prüfen:** Test grün.

### 8.5 Extrusion

- [ ] **T-5.5.1** extrude(base, direction, length): Obere Face = Base um direction*length verschoben; pro Base-Edge eine Mantel-Face (4 Kanten: untere Kante, zwei vertikale, obere Kante); PlaneSurface oder bei Bogen CylinderSurface; Shell aus allen Faces → Solid.  
  **Prüfen:** Kompiliert.

- [ ] **T-5.5.2** Optional: Parameter symmetric (± halbe Länge).  
  **Prüfen:** Optional getestet.

- [ ] **T-5.5.3** Test: Rechteck-Face extrudiert → Quader; Kreis-Face extrudiert → Zylinder.  
  **Prüfen:** Test grün.

### 8.6 Revolution

- [ ] **T-5.6.1** revolve(base, axis, angle): 2D-Kontur um Achse rotieren → 3D-Surfaces/Kanten; geschlossene Kontur → geschlossener Solid.  
  **Prüfen:** Kompiliert.

- [ ] **T-5.6.2** Test: Rechteck um Z → Zylinder; Halbkreis um Z → Kugel.  
  **Prüfen:** Test grün.

- [ ] **T-5** **Phase 5 abgeschlossen:** Box, Zylinder, Kugel, Extrude, Revolve; alle Tests grün.

**Phase 5 erledigt wenn:** Primitiven und Extrude/Revolve implementiert, Volumen/Bounds getestet.

---

## 9. Phase 6: Boolean-Operationen

**Ziel:** Fuse, Cut, Common für zwei Solide; mindestens zwei Quader getestet.

### 9.1 Vorbereitung

- [ ] **T-6.1.1** Kurzdokumentation: Konzept (Schnittkurven → Faces zerschneiden → Innen/Außen → neue Hülle).  
  **Prüfen:** Doku vorhanden.

### 9.2 Face–Face-Schnitt

- [ ] **T-6.2.1** `kernel/boolean/FaceFaceIntersection.h` und `.cpp`: Zwei Faces → Schnittkurve(n) in 3D. Eben–Eben: Linie oder leer; Eben–Zylinder: Linie/Ellipse; Zylinder–Zylinder: nach Bedarf.  
  **Prüfen:** Kompiliert.

- [ ] **T-6.2.2** Test: Zwei Ebenen → Schnittlinie; Ebene + Zylinder → Ellipse/Linien.  
  **Prüfen:** Test grün.

- [ ] **T-6.2.3** FaceFaceIntersection in CMake.  
  **Prüfen:** Build + Tests.

### 9.3 Trimmen

- [ ] **T-6.3.1** `kernel/boolean/TrimEdgesOnFace.h` und `.cpp`: Face F, Schnittkurve K → F getrimmt (neue Ränder = Teile alter Loops + Teile von K); (u,v)-Parameterraum, 2D-Clipping.  
  **Prüfen:** Kompiliert.

- [ ] **T-6.3.2** Trimmen in CMake.  
  **Prüfen:** Build.

### 9.4 BooleanOps (Fuse, Cut, Common)

- [ ] **T-6.4.1** `kernel/boolean/BooleanOps.h` und `.cpp`: (1) Face–Face-Schnitte A vs. B; (2) Faces trimmen; (3) Innen/Außen (Punkt-in-Solid, Ray-Casting); (4) Fuse/Cut/Common: passende Face-Stücke auswählen, neue Topologie aufbauen. API: `Solid fuse(A,B)`, `Solid cut(A,B)`, `Solid common(A,B)`.  
  **Prüfen:** Kompiliert.

- [ ] **T-6.4.2** Test: Zwei Quader Fuse → ein zusammengesetzter Solid; Cut → Ausschnitt; Common → Überlappung.  
  **Prüfen:** Tests grün.

- [ ] **T-6.4.3** BooleanOps in CMake.  
  **Prüfen:** Build + Tests.

### 9.5 Robustheit

- [ ] **T-6.5.1** Toleranzen in Boolean nutzen; Sonderfälle (parallel, tangential, degeneriert) dokumentieren und wo möglich behandeln.  
  **Prüfen:** Doku/Code sichtbar.

- [ ] **T-6** **Phase 6 abgeschlossen:** Boolean Fuse/Cut/Common für zwei Solide (mind. Quader); Tests grün.

**Phase 6 erledigt wenn:** Zwei Quader Fuse/Cut/Common liefern erwartete Ergebnisse.

---

## 10. Phase 7: Fillet und Chamfer

**Ziel:** Fillet (konstanter Radius), Chamfer (Abstand) auf Kanten; mindestens Quader getestet.

### 10.1 Fillet

- [ ] **T-7.1.1** `kernel/fillet/FilletOps.h` und `.cpp`: Eingabe Solid, Edge-IDs, Radius. Vereinfachung: Kanten mit zwei planaren Faces; Schnitt der Ebenen, Versatz um Radius, neue Face (Torus-Segment/Zylinder-Segment), benachbarte Faces trimmen. API: `Solid fillet(Solid, const std::vector<EdgeId>&, double radius)`.  
  **Prüfen:** Kompiliert.

- [ ] **T-7.1.2** Test: Quader, eine Kante Fillet R → Volumenänderung, neue Face-Anzahl.  
  **Prüfen:** Test grün.

- [ ] **T-7.1.3** FilletOps in CMake.  
  **Prüfen:** Build + Tests.

### 10.2 Chamfer

- [ ] **T-7.2.1** `kernel/fillet/ChamferOps.h` und `.cpp`: Solid, Edge-IDs, Abstand(e); abgeschrägte Ebene, benachbarte Faces trimmen. API: `Solid chamfer(Solid, const std::vector<EdgeId>&, double distance)`.  
  **Prüfen:** Kompiliert.

- [ ] **T-7.2.2** Test: Quader, eine Kante Chamfer 1×1 → neue Face.  
  **Prüfen:** Test grün.

- [ ] **T-7.2.3** ChamferOps in CMake.  
  **Prüfen:** Build + Tests.

- [ ] **T-7** **Phase 7 abgeschlossen:** Fillet und Chamfer auf Quader getestet; Tests grün.

**Phase 7 erledigt wenn:** Fillet und Chamfer API vorhanden, mindestens ein Test pro Operation grün.

---

## 11. Phase 8: Erweiterte Operationen (optional)

**Ziel:** Shell, Draft, Loft, Sweep – je nach Priorität.

- [ ] **T-8.1.1** `kernel/advanced/ShellOps.h` und `.cpp`: Solid aushöhlen (Wandstärke); API `Solid shell(Solid, double thickness, optional face_ids)`. Test: Quader → Hohlkörper. In CMake.  
  **Prüfen:** Build + Test.

- [ ] **T-8.2.1** `kernel/advanced/DraftOps.h` und `.cpp`: Flächen um Winkel kippen; API `Solid draft(Solid, ...)`. In CMake.  
  **Prüfen:** Build.

- [ ] **T-8.3.1** `kernel/advanced/LoftOps.h` und `.cpp`: Mehrere Wires zu Solid verbinden; API `Solid loft(std::vector<Wire>, ...)`. In CMake.  
  **Prüfen:** Build.

- [ ] **T-8.4.1** `kernel/advanced/SweepOps.h` und `.cpp`: Profil entlang 3D-Kurve; API `Solid sweep(Wire, Curve3D path, ...)`. In CMake.  
  **Prüfen:** Build.

- [ ] **T-8** **Phase 8 abgeschlossen:** (Optional) Shell/Draft/Loft/Sweep je nach Priorität umgesetzt.

---

## 12. Phase 9: Import und Export

**Ziel:** STL Export/Import; optional STEP Export/Import.

### 12.1 Triangulierung (MeshGenerator)

- [ ] **T-9.0.1** `kernel/io/MeshGenerator.h` und `.cpp` (oder unter builder): Solid → Dreiecksmesh (Vertices + Indices). Pro Face: planare Face in Dreiecke zerlegen; Zylinder/Kugel diskretisieren.  
  **Prüfen:** Kompiliert; box() → Mesh mit 12 Dreiecken (2 pro Face).

- [ ] **T-9.0.2** MeshGenerator in CMake.  
  **Prüfen:** Build.

### 12.2 STL

- [ ] **T-9.1.1** `kernel/io/StlWriter.h` und `.cpp`: Solid → MeshGenerator → STL (binär oder ASCII) schreiben.  
  **Prüfen:** Kompiliert.

- [ ] **T-9.1.2** `kernel/io/StlReader.h` und `.cpp`: STL lesen → Dreiecksnetz → Shell aus Dreiecks-Faces; wenn geschlossen → Solid.  
  **Prüfen:** Kompiliert.

- [ ] **T-9.1.3** Test: Box → STL export → STL import → Bounds/Volumen plausibel.  
  **Prüfen:** Test grün.

- [ ] **T-9.1.4** StlWriter/StlReader in CMake.  
  **Prüfen:** Build + Tests.

### 12.3 STEP (optional)

- [ ] **T-9.2.1** `kernel/io/StepWriter.h` und `.cpp`: B-Rep in STEP AP203/AP214 abbilden.  
  **Prüfen:** Kompiliert.

- [ ] **T-9.2.2** `kernel/io/StepReader.h` und `.cpp`: STEP parsen → Topologie/Geometrie.  
  **Prüfen:** Kompiliert.

- [ ] **T-9.2.3** Test: Box → STEP export → import → Solid plausibel. In CMake.  
  **Prüfen:** Build + Test.

- [ ] **T-9** **Phase 9 abgeschlossen:** STL Export/Import; optional STEP; Tests grün.

**Phase 9 erledigt wenn:** STL Roundtrip (Box) funktioniert; STEP optional.

---

## 13. Phase 10: Integration in Hydra CAD

**Ziel:** KernelBridge; buildPartFromSketch(); Anzeige im Viewport (trianguliert); CMake-Option.

### 13.1 KernelBridge

- [ ] **T-10.1.1** `src/core/kernel/KernelBridge.h` und `.cpp` anlegen (oder `src/core/KernelBridge.h` mit Include kernel). `bool initialize()`; `bool buildPartFromSketch(const Sketch& sketch)` (WireBuilder → FaceBuilder → SolidBuilder.extrude); `Solid getLastSolid()` oder `Shape getPartShape(const Part& part)`.  
  **Prüfen:** Kompiliert.

- [ ] **T-10.1.2** CMake: KernelBridge nutzt cad_kernel; App/cad_core kann KernelBridge nutzen wenn CAD_USE_EIGENER_KERN ON. Include-Pfad für Modeler/Sketch.  
  **Prüfen:** Build.

### 13.2 Feature-Historie (Part → Kern)

- [ ] **T-10.2.1** KernelBridge: Für Part mit Features nacheinander: erste Skizze → Solid (extrude/revolve); Hole → Boolean Cut mit Zylinder; Fillet → FilletOps. Sketch-IDs und Feature-Parameter aus Part/Feature lesen; Ergebnis (Solid) cachen.  
  **Prüfen:** Optional Test oder manuell: Part mit Extrude + Hole liefert Solid.

### 13.3 Viewport / Rendering

- [ ] **T-10.3.1** KernelBridge::getPartShape(part) → Solid → MeshGenerator::triangulate(solid) → Vertices/Indices.  
  **Prüfen:** Aufruf liefert Mesh.

- [ ] **T-10.3.2** Viewport/AppController: Wenn CAD_USE_EIGENER_KERN und Kernel aktiv, Mesh von KernelBridge an RenderEngine/Coin3D übergeben.  
  **Prüfen:** Sichtbarer 3D-Körper im Viewport bei aktivem Eigenem Kern.

### 13.4 Build-Option und Ablösung

- [ ] **T-10.4.1** CMake: Bei CAD_USE_EIGENER_KERN=ON Kernel bauen; AppController nutzt KernelBridge für Part-Erzeugung.  
  **Prüfen:** Build mit ON; App startet.

- [ ] **T-10** **Phase 10 abgeschlossen:** KernelBridge; buildPartFromSketch() mit Extrude; Anzeige im Viewport; mind. ein End-to-End-Test (Sketch → Part → Anzeige).

**Phase 10 erledigt wenn:** Sketch → Part über Eigenen Kern → sichtbar im Viewport; CMake-Option funktioniert.

---

## 14. Schnell-Checkliste „Erster lauffähiger Kern“

Zum Abhaken, sobald die Phase vollständig ist:

- [ ] **P0** Phase 0: Math + Toleranzen, Tests grün.
- [ ] **P1** Phase 1: Wire2D (Linie, Kreis, Bogen, Spline), geschlossen.
- [ ] **P2** Phase 2: Topologie + Explorer.
- [ ] **P3** Phase 3: 3D-Kurven + Surfaces.
- [ ] **P4** Phase 4: Sketch → Wire2D → planare Face.
- [ ] **P5** Phase 5: Box, Zylinder, Kugel, Extrude, Revolve.
- [ ] **P6** Phase 6: Boolean Fuse/Cut/Common (Quader).
- [ ] **P7** Phase 7: Fillet, Chamfer (Quader).
- [ ] **P8** Phase 8: (optional) Shell/Draft/Loft/Sweep.
- [ ] **P9** Phase 9: STL Export/Import.
- [ ] **P10** Phase 10: KernelBridge, Viewport-Anzeige.

Wenn P0–P10 (P8 optional) erfüllt sind, ist der **minimale lauffähige eigene CAD-Kern** in Hydra CAD fertig.

---

## 15. Reihenfolge und Dauer (Orientierung)

| Phase | Inhalt           | Dauer (1–2 Devs, grob) |
|-------|------------------|-------------------------|
| 0     | Math, Toleranzen | 1–2 Wochen              |
| 1     | 2D-Kurven, Wire2D | 2–4 Wochen             |
| 2     | Topologie        | 2–3 Wochen              |
| 3     | 3D-Kurven, Surfaces | 3–6 Wochen           |
| 4     | Sketch → Face    | 2–4 Wochen              |
| 5     | Primitiven, Extrude, Revolve | 4–8 Wochen |
| 6     | Boolean          | 2–6 Monate              |
| 7     | Fillet, Chamfer  | 1–2 Monate              |
| 8     | Shell, Draft, Loft, Sweep | je 2–4 Wochen   |
| 9     | STL/STEP         | 1–2 Monate              |
| 10    | Integration      | 1–2 Monate              |

**Minimum (Extrude + Boolean + Fillet + Anzeige):** ca. 6–12 Monate.

---

## 16. Literatur

- B-Rep: Hoffmann, *Geometric and Solid Modeling*; Mantyla, *An Introduction to Solid Modeling*; OCCT (Referenz).
- Boolean: SHAPES, CGAL 3D Boolean; Laidlaw et al., „A robust approach to the solid modeling“.
- NURBS: Piegl & Tiller, *The NURBS Book*.
- Toleranzen: Hoffmann, *Robustness in Geometric Computations* (JCISE).
- STEP: ISO 10303.

---

*Stand: Januar 2026. Hydra CAD – Plan Option 2 (eigener CAD-Kern). Jede Checkbox = eine abarbeitbare Aufgabe; „Prüfen:“ = Definition of Done für den Task.*
