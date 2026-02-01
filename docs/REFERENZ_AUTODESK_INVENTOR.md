# Referenz: Autodesk Inventor

> **Umsetzungs-TODO:** Aus dieser Referenz abgeleitete, vollständige TODO-Liste mit Status und konkreten Umsetzungsschritten: [TODO_REFERENZ_VOLLSTAENDIG.md](TODO_REFERENZ_VOLLSTAENDIG.md).

## 1. Allgemeines zu Autodesk Inventor

**Hersteller:** Autodesk

**CAD-Typ:**
- parametrisches 3D-CAD
- featurebasiert
- verlaufsbasiert (Historie)

**Zielbereiche:**
- Maschinenbau
- Metallbau
- Produktdesign
- Konstruktion technischer Baugruppen

**Arbeitsprinzip:**
Skizze → Feature → Verlauf → Ableitung → Zeichnung

---

## 2. Dateitypen (alle Öffnungs- & Speicherformate)

### 2.1 Native Inventor-Dateien

| Erweiterung | Bedeutung |
|-------------|-----------|
| .ipt | Bauteil (Part) |
| .iam | Baugruppe (Assembly) |
| .idw | Inventor-Zeichnung |
| .dwg | AutoCAD-kompatible Zeichnung |
| .ipn | Präsentation (Explosionsdarstellung) |
| .ipj | Projektdatei |
| .ide | iLogic-Regeln (alt) |
| .xls / .xlsx | Parameter-Tabellen |

### 2.2 Importformate

- STEP (.stp, .step)
- IGES (.igs)
- SAT (ACIS)
- STL
- OBJ
- DXF
- DWG
- Parasolid (.x_t, .x_b)
- JT
- SolidWorks-Dateien

### 2.3 Exportformate

- PDF
- DXF / DWG
- STL (3D-Druck)
- STEP
- OBJ
- FBX
- SAT
- JT

---

## 3. Startfenster (Inventor-Startseite)

- **Neue Datei erstellen**
  - Standard.ipt
  - Standard.iam
  - Standard.idw
  - Benutzerdefinierte Vorlagen
- **Letzte Dateien**
- **Projekt wechseln**
- **Tutorials**
- **Beispielprojekte**
- **Cloud-Optionen**

---

## 4. Projektverwaltung (.ipj)

- Arbeitsbereich
- Bibliothekspfade
- Inhaltscenter
- Suchpfade
- Absolute / relative Pfade
- Versionssteuerung (Vault)

---

## 5. Benutzeroberfläche (UI)

### 5.1 Hauptfenster-Aufbau

- Menüband (Ribbon)
- Grafikfenster
- Modellbrowser
- Eigenschaftenfenster
- Statusleiste
- Navigationswürfel
- ViewCube
- Mausnavigation

### 5.2 Ribbon (Menüband)

**Registerkarten (abhängig vom Modus):**
- Datei
- 3D-Modell
- Skizze
- Zusammenbauen
- Prüfen
- Extras
- Verwalten
- Ansicht
- Umgebung

---

## 6. Modellbrowser (linke Seite)

- **Ursprung**
  - XY-Ebene
  - YZ-Ebene
  - XZ-Ebene
  - Mittelpunkt
- **Skizzen**
- **Features**
- **Arbeitselemente**
- **Parameter**
- **iLogic**
- **Abhängigkeiten**
- **Unterdrückung**
- **Rollback-Leiste**

---

## 7. Navigationssystem

- **Drehen (Orbit)**
- **Verschieben (Pan)**
- **Zoomen**
- **Zoom alles**
- **Zoom Auswahl**
- **Freies Drehen**
- **ViewCube:**
  - Vorderansicht
  - Rückansicht
  - Links / Rechts
  - Oben / Unten
  - Isometrisch

---

## 8. Skizzenumgebung

### 8.1 Skizzenerstellung

- Skizze starten
- Ebene wählen
- Skizze beenden

### 8.2 Skizzenelemente

| Element | Varianten |
|---------|-----------|
| **Linie** | — |
| **Rechteck** | 2-Punkt, Mittelpunkt |
| **Kreis** | Mittelpunkt, Tangential |
| **Bogen** | — |
| **Ellipse** | — |
| **Polygon** | — |
| **Spline** | — |
| **Punkt** | — |
| **Text** | — |

### 8.3 Skizzenbedingungen (Constraints)

- Horizontal
- Vertikal
- Parallel
- Senkrecht
- Tangential
- Koinzident
- Gleich
- Symmetrisch
- Fixiert

### 8.4 Bemaßungen

- Längenmaß
- Durchmesser
- Radius
- Winkel
- Abstand
- Parametrisch verknüpft

### 8.5 Skizzenanalyse

- Voll bestimmt
- Unterbestimmt
- Überbestimmt
- Konfliktanzeige

---

## 9. 3D-Features

### 9.1 Grundkörper

- **Extrusion**
  - hinzufügen
  - entfernen
  - vereinen
- **Rotation**
- **Sweep**
- **Loft**

### 9.2 Formbearbeitung

- Abrundung
- Fase
- Verrundung
- **Bohrung**
  - Gewindebohrung
  - Durchgang
  - Sackloch
- Gewinde

### 9.3 Muster

- Rechteckmuster
- Kreismuster
- Pfadmuster
- Flächenmuster

### 9.4 Arbeitselemente

- Arbeitsebene
- Arbeitsachse
- Arbeitspunkt
- Koordinatensystem

### 9.5 Direktbearbeitung

- Fläche verschieben
- Fläche löschen
- Fläche ersetzen
- Offset

---

## 10. Parameter

- Benutzerparameter
- Modellparameter
- Formeln
- Verknüpfungen
- Externe Tabellen

---

## 11. iLogic

- Regel erstellen
- Wenn-Dann-Logik
- Automatisierung
- Variantenteile
- Konfigurationssteuerung

---

## 12. Komponenten

- Bauteil platzieren
- Baugruppe platzieren
- Ableiten
- Ersetzen
- Unterdrücken
- Sichtbarkeit

---

## 13. Abhängigkeiten (Constraints)

**Abhängigkeit:**
- Bündig
- Fluchtend
- Winkel
- Tangential
- Einfügen

**Gelenke:**
- Drehgelenk
- Schiebegelenk
- Zylindergelenk
- Planargelenk

---

## 14. Baugruppenfunktionen

- Schrauben
- Normteile (Content Center)
- Spiegeln
- Muster
- Kollision prüfen
- Kontaktanalyse
- Freiheitsgrade anzeigen

---

## 15. Explosionsdarstellung

- **Präsentation (.ipn)**
- Explosionsschritte
- Animieren
- Zeitachsen
- Rückwärtsanimation

---

## 16. Zeichnungen (.idw / .dwg)

### 16.1 Ansichten

- Grundansicht
- Projektionsansicht
- Schnittansicht
- Detailansicht
- Ausbruch
- Hilfsansicht

### 16.2 Bemaßung & Annotation

- Maßlinien
- Toleranzen
- Passungen
- Oberflächenzeichen
- Schweißsymbole
- Mittellinien
- Stückliste

### 16.3 Zeichnungsverwaltung

- Blattformate
- Maßstab
- Rahmen
- Schriftfelder
- **Normen:** ISO, DIN, ANSI

---

## 17. Zusatzumgebungen

### 17.1 Blechumgebung (Sheet Metal)

**Zweck:** Konstruktion von abgekanteten Blechteilen, automatische Abwicklung, Fertigungsvorbereitung

#### 17.1.1 Blechregeln

- Material
- Blechdicke
- Biegeradius
- K-Faktor
- Abwicklungsregel
- Eckbehandlung
- Relief-Typ
- Biegetabelle
- DIN / ISO / Benutzerdefiniert

#### 17.1.2 Blechfeatures

- Konturflansch
- Kantenflansch
- Eckflansch
- Übergangsflansch
- Stanzung
- Ausklinkung
- Loch
- Sicke
- Biegung lösen
- Biegung wiederherstellen

#### 17.1.3 Abwicklung

- Abwicklung erzeugen
- Flachansicht
- Biegelinien anzeigen
- Biegewinkel
- Laserexport
- DXF-Abwicklung
- Fertigungshinweise

#### 17.1.4 Blechparameter

- Neutralfaser
- Rückfederung
- Kantenzugabe
- Abzugswert
- Tabellenverknüpfung

---

### 17.2 Schweißkonstruktion (Weldment)

#### 17.2.1 Schweißumgebung

- Schweißnaht vorbereiten
- Schweißgruppe
- Schweißprozessverwaltung

#### 17.2.2 Schweißnahttypen

- Kehlnaht
- Stumpfnaht
- V-Naht
- U-Naht
- Punktnaht
- Naht entlang Kante
- Kosmetische Naht

#### 17.2.3 Schweißbearbeitung

- Nahtvolumen
- Nachbearbeitung
- Schweißzug simulieren
- Nahtlisten

#### 17.2.4 Zeichnungsintegration

- Schweißsymbole
- ISO-Schweißzeichen
- Nahtnummern
- Schweißstückliste

---

### 17.3 Rahmenkonstruktion (Frame Generator)

#### 17.3.1 Profilbibliothek

- Rechteckprofile
- Rundrohre
- Quadratrohre
- U-Profile
- I-Träger
- L-Profile
- DIN / ISO / EN

#### 17.3.2 Rahmenfunktionen

- Rahmen einfügen
- Automatische Gehrung
- Profilverlängerung
- Profilkürzung
- Eckverbindungen
- Mehrfachrahmen

#### 17.3.3 Strukturverwaltung

- Strukturgruppe
- Nummerierung
- Zuschnittliste
- Längenoptimierung

---

### 17.4 Rohr- & Schlauchkonstruktion (Tube & Pipe)

#### 17.4.1 Rohrsysteme

- Starre Rohre
- Flexible Schläuche
- Hydraulik
- Pneumatik

#### 17.4.2 Rohrfunktionen

- Startpunkt definieren
- Routen skizzieren
- Biegeradien
- Anschlussdefinitionen
- Fittings
- Normverbinder

#### 17.4.3 Stücklisten

- Rohrlängen
- Fittings
- Normteile
- Anschlusslisten

---

### 17.5 Kabel & Harness

#### 17.5.1 Elektrische Systeme

- Kabelbaum
- Einzeladern
- Stecker
- Kontakte
- Pins

#### 17.5.2 Harness-Funktionen

- Kabel routen
- Bündel definieren
- Kabellängenberechnung
- Biegeradien
- Schutzschläuche

---

### 17.6 Belastungsanalyse (Stress Analysis)

#### 17.6.1 Analysearten

- Statisch
- Linear-elastisch
- Vereinfachte FEM

#### 17.6.2 Randbedingungen

- Fixierungen
- Kräfte
- Druck
- Momente
- Gewichtskraft

#### 17.6.3 Materialdefinition

- Elastizitätsmodul
- Poissonzahl
- Streckgrenze
- Sicherheitsfaktor

#### 17.6.4 Ergebnisse

- Spannungen (von Mises)
- Verformung
- Sicherheitsfaktor
- Farbskalen
- Ergebnisberichte

---

### 17.7 Dynamische Simulation

#### 17.7.1 Bewegungsarten

- Rotation
- Translation
- Schub
- Kurvenbewegung

#### 17.7.2 Antriebe

- Motor
- Drehzahl
- Zeitsteuerung
- Lastprofile

#### 17.7.3 Ausgabe

- Bewegungsdiagramme
- Geschwindigkeiten
- Kräfte
- Animationen

---

### 17.8 Freiform / Formgebung

- T-Splines
- Ziehen
- Glätten
- Verdicken
- Freiformflächen
- Übergangsflächen

---

### 17.9 Datenverwaltung

- Autodesk Vault
- Versionsverwaltung
- Benutzerrechte
- Freigabestatus
- Revisionen

---

### 17.10 Automatisierung & Erweiterungen

- iLogic
- Makros
- API (VBA / C#)
- Add-Ins
- Parametersteuerung
- Konfiguratoren

---

# Referenz: Fusion 360

## 18.1 Allgemeines zu Fusion 360

**Hersteller:** Autodesk

**CAD-Typ:**
- parametrisches 3D-CAD
- direktes Modellieren
- hybrides CAD (parametrisch + direkt)

**Systemarchitektur:**
- Cloud-basiert
- lokale Cache-Dateien

**Zielgruppen:**
- Produktdesign
- Maschinenbau (leicht–mittel)
- Maker
- CNC-Fertigung
- 3D-Druck
- Elektronikentwicklung

**Arbeitsprinzip:**
Skizze → Feature → Timeline → Version

---

## 18.2 Dateisystem & Datenstruktur

### 18.2.1 Projektstruktur

- Projekte
- Ordner
- Unterordner
- Cloud-Speicherung
- Offline-Cache

### 18.2.2 Versionierung

- Automatische Versionsnummern
- Versionsvergleich
- Rollback
- Kommentierung
- Änderungsverlauf

### 18.2.3 Exportformate

- F3D (Fusion intern)
- STEP
- IGES
- STL
- OBJ
- DXF
- SAT
- FBX
- 3MF

---

## 18.3 Benutzeroberfläche

### 18.3.1 Hauptbereiche

- Datenpanel (links)
- Arbeitsbereichsauswahl (oben links)
- Werkzeugleiste (Toolbar)
- Canvas (Grafikbereich)
- Browser (Baumstruktur)
- Timeline (unten)
- Navigationswürfel
- Ansichtssteuerung

### 18.3.2 Arbeitsbereiche (Workspaces)

- Design
- Render
- Animation
- Simulation
- Manufacture (CAM)
- Drawing
- Electronics
- Generative Design

---

## 18.4 Design Workspace – Konstruktion

### 18.4.1 Komponenten & Körper

- Bodies
- Components
- Root Component
- Aktivieren / Deaktivieren
- Sichtbarkeit
- Referenzierung

### 18.4.2 Skizzenumgebung

**Skizzenelemente:** Linie, Rechteck (2-Punkt, Mittelpunkt), Kreis, Bogen, Ellipse, Polygon, Spline, Punkt, Text

**Skizzenbedingungen:** Horizontal / Vertikal, Parallel, Senkrecht, Tangential, Koinzident, Gleich, Fixieren, Symmetrisch

**Skizzenbemaßung:** Länge, Winkel, Radius, Durchmesser, Abstände, Parametrisch verknüpft

### 18.4.3 Solid-Features

- Extrude
- Revolve
- Sweep
- Loft
- Hole
- Thread
- Shell
- Fillet
- Chamfer
- Draft

### 18.4.4 Muster & Spiegeln

- Rectangular Pattern
- Circular Pattern
- Pattern on Path
- Mirror (Feature / Body / Component)

### 18.4.5 Arbeitselemente

- Construction Plane
- Offset Plane
- Tangent Plane
- Construction Axis
- Construction Point

### 18.4.6 Direktmodellierung

- Press Pull
- Move / Copy
- Combine
- Delete Face
- Replace Face
- Offset Face

### 18.4.7 Timeline

- Feature-Historie
- Rollback
- Unterdrücken
- Reihenfolge ändern
- Parametrische Steuerung

---

## 18.5 Surface Workspace

- Patch
- Extend
- Trim
- Stitch
- Thicken
- Replace Face
- Surface Loft
- Boundary Surface

---

## 18.6 Form Workspace (T-Splines)

- Erstellen primitiver Formen
- Ziehen
- Skalieren
- Glätten
- Subdivision
- Umwandlung in Solid
- Freiformmodellierung

---

## 18.7 Sheet Metal

- Blechregeln
- Blechdicke
- Biegeradius
- K-Faktor
- Flansche
- Konturflansch
- Abwicklung
- DXF-Export

---

## 18.8 Baugruppen & Gelenke

- **Joints:** Rigid, Revolute, Slider, Cylindrical, Pin-Slot, Planar
- As-Built Joint
- Joint Limits
- Bewegungssimulation
- Kontakt

---

## 18.9 Simulation Workspace

**Analysearten:** Static Stress, Modal Frequenzen, Thermal, Event Simulation, Buckling, Shape Optimization

**Randbedingungen:** Fixierung, Kraft, Druck, Gewicht, Temperatur, Kontakt

**Ergebnisse:** Spannungen, Verformung, Sicherheitsfaktor, Diagramme, Berichtsexport

---

## 18.10 Generative Design

- Designräume
- Erhaltungsbereiche
- Lastfälle
- Materialien
- Fertigungsverfahren
- Cloud-Berechnung
- Ergebnisvarianten

---

## 18.11 Render Workspace

- Materialien
- Erscheinungsbilder
- Texturen
- Umgebungslicht
- HDRI
- Kamera
- Tiefenschärfe
- Raytracing
- Cloud Rendering

---

## 18.12 Animation Workspace

- Explosionsdarstellung
- Zeitachse
- Bewegungspfad
- Kameraanimation
- Videoexport

---

## 18.13 Drawing Workspace

- 2D-Zeichnungen
- Ansichten
- Schnitte
- Details
- Maße
- Toleranzen
- Stücklisten
- Normen (ISO / ANSI)

---

## 18.14 Manufacture (CAM)

**CAM-Strategien:** 2D Milling, 3D Milling, Adaptive Clearing, Contour, Pocket, Drilling, Turning, Multi-Axis (5-Achs)

**Werkzeuge:** Werkzeugbibliothek, Fräser, Bohrer, Drehwerkzeuge

**Parameter / Ausgabe:** Simulation, Kollisionserkennung, Postprozessor, G-Code Export

---

## 18.15 Electronics Workspace

- Schaltplan
- PCB-Layout
- Bauteilbibliothek
- Routing
- 3D-PCB-Integration
- ECAD ↔ MCAD

---

## 18.16 Cloud & Zusammenarbeit

- Live-Sharing
- Kommentare
- Markups
- Versionsverlauf
- Team-Projekte
- Rechteverwaltung

---

## 18.17 API & Erweiterungen

- Fusion API (Python / JavaScript)
- Skripte
- Add-ins
- Automatisierung
- Parametersteuerung

---

# Referenz: SolidWorks (Dassault Systèmes)

## 19. SolidWorks – Grundcharakter

- parametrisches Feature-CAD
- stark Windows-orientiert
- sehr verbreitet im Mittelstand
- Fokus: Konstruktion & Maschinenbau

### 19.1 Dateitypen

| Erweiterung | Bedeutung |
|-------------|-----------|
| .SLDPRT | Bauteil |
| .SLDASM | Baugruppe |
| .SLDDRW | Zeichnung |

### 19.2 Benutzeroberfläche

- FeatureManager (Modellbaum)
- PropertyManager
- ConfigurationManager
- CommandManager (Ribbon-ähnlich)
- Heads-Up View Toolbar

### 19.3 Skizzen

- 2D-Skizzen
- 3D-Skizzen
- Beziehungen
- Vollbestimmung
- Gleichungen
- Referenzgeometrie

### 19.4 3D-Features

- Extrusion
- Rotation
- Sweep
- Loft
- Thin Feature
- Shell
- Draft
- Fillet
- Chamfer

### 19.5 Baugruppen

**Mates:** Coincident, Parallel, Distance, Angle, Gear, Cam

- Flexible Subassemblies
- Large Assembly Mode
- SpeedPak

### 19.6 Konfigurationen

- Varianten in einer Datei
- Maß- und Featuresteuerung
- Tabellensteuerung
- Stücklistenlogik

### 19.7 Blech

- Flansche
- Abwicklung
- K-Faktor
- DXF-Export
- Fertigungszeichnungen

### 19.8 Simulation (SolidWorks Simulation)

- Statik
- Frequenz
- Thermik
- Ermüdung
- Motion
- Flow Simulation

### 19.9 Zusatzmodule

- Routing
- Electrical
- Plastics
- Visualize (Rendering)
- PDM

---

# Referenz: Creo (PTC)

## 20. Creo – Grundcharakter

- sehr stark parametrisch
- extrem stabil bei großen Baugruppen
- stark regelbasiert
- Industrie- & Serienentwicklung

### 20.1 Dateitypen

- .PRT
- .ASM
- .DRW

### 20.2 Modellierungsphilosophie

- Top-Down-Design
- Skeleton-Modelle
- Referenzsteuerung
- Design Intent

### 20.3 Skizzen & Features

- voll parametergesteuert
- Relations
- Family Tables
- Feature Groups
- UDFs (User Defined Features)

### 20.4 Baugruppen

- Component Interfaces
- Mechanisms
- Kinematik
- Kollisionsanalyse
- Bewegungsstudien

### 20.5 Flächenmodellierung

- Advanced Surfacing
- Class-A-Flächen
- Übergangsflächen
- Style Surfaces

### 20.6 Blech & Struktur

- Sheetmetal
- Weldments
- Frameworks
- Parametersteuerung

### 20.7 Simulation

- Creo Simulate
- Mechanica
- Struktur
- Thermisch
- Dynamisch

### 20.8 PLM-Integration

- Windchill
- Versionskontrolle
- Freigabeprozesse
- Änderungsmanagement

---

# Referenz: Siemens NX

## 21. Siemens NX – Grundcharakter

- High-End-Industrie-CAD
- CAD + CAM + CAE in einem System
- Automotive, Luftfahrt, Maschinenbau
- extrem leistungsfähig

### 21.1 Dateiformat

- .prt (alles in einer Datei)

### 21.2 Modellierung

- Parametrisch
- Direkt
- Synchronous Technology
- Feature-basiert
- History-frei möglich

### 21.3 Skizzen

- hochpräzise Skizzentools
- mathematische Beziehungen
- Expressions

### 21.4 Solid Modeling

- Boolean Operations
- Feature Groups
- Part Families
- Reuse Library

### 21.5 Surfacing

- Class-A-Surfacing
- Studio Surface
- Through Curves
- Boundary Surface
- Automotive-Standard

### 21.6 Baugruppen

- Massive Assembly Support
- Lightweight Loading
- Arrangement Manager
- Constraints
- Structure Navigator

### 21.7 Simulation (NX CAE)

- FEM
- CFD
- Multiphysics
- Dynamik
- Nichtlinear

### 21.8 CAM (NX CAM)

- 2- bis 5-Achs-Fräsen
- Drehen
- Mill-Turn
- Robotik
- Werkzeugpfade Industrieklasse

### 21.9 Datenmanagement

- Teamcenter
- Versionskontrolle
- Produktstruktur
- Änderungsprozesse

---

# Referenz: CATIA (Dassault Systèmes)

## 22. CATIA – Grundcharakter

- absolutes High-End-CAD
- Weltstandard in Luftfahrt & Automotive
- extrem komplex
- modulbasiert

### 22.1 Dateitypen

- .CATPart
- .CATProduct
- .CATDrawing

### 22.2 Arbeitsumgebungen (Workbenches)

- Part Design
- Assembly Design
- Generative Shape Design
- FreeStyle
- Wireframe
- Drafting
- DMU
- Kinematics
- Composites

### 22.3 Solid Design

- Feature-basiert
- Body-Struktur
- Boolean Bodies
- Parametrik

### 22.4 Surfacing (weltweit führend)

- Class-A-Flächen
- Automotive Oberflächen
- **mathematische Kontinuität:** G0, G1, G2, G3

### 22.5 Baugruppen

- sehr große Produktstrukturen
- DMU Space Analysis
- Clash Detection
- Mockups

### 22.6 Kinematik

- Mechanisms
- Bewegungsdefinition
- Simulation
- Animation

### 22.7 Composite Design

- Faserverbund
- Lagenaufbau
- Luftfahrtstrukturen
- CFK-Bauteile

### 22.8 PLM-Integration

- ENOVIA
- 3DEXPERIENCE
- Lebenszyklusverwaltung
- Freigaben
- Revisionen
