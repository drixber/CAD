# Changelog - Hydra CAD

## [3.0.5] - 2026-01-27

### Changed
- 🔄 Update-System: Kein curl auf Windows mehr nötig – GitHub-API und Download nutzen Qt Network (QNetworkAccessManager)
- 🔄 UpdateChecker: Parser-Funktion `parseGithubReleaseResponse` für API-Antwort; Fallback auf curl nur ohne Qt Network

### Fixed
- ✅ HttpClient: Progress-Callback nur bei vorhandenem Callback aufrufen (kein Crash bei nullptr)
- ✅ HttpClient: Download-Rückgabe nur true, wenn Datei tatsächlich geschrieben wurde
- ✅ installUpdate: Prüfung auf leere download_url; klare Fehlermeldung
- ✅ UpdateService: Regex-Raw-Strings mit Delimiter (Compiler-Warnungen behoben)
- ✅ Fehlerbehandlung bei manueller Update-Prüfung (GitHub-Fehler in Statusleiste)

### Removed
- 🧹 Ungenutzte `parseResponse` in HttpClient entfernt

---

## [2.0.0] - 2026-01-23

### Added

#### UI Enhancements
- ✅ Professional icon system with SVG icons for all ribbon buttons
- ✅ Layout templates (Inventor, SolidWorks, CATIA styles)
- ✅ Customizable dock system with save/load layouts
- ✅ Enhanced status bar (coordinates, units, snap mode)
- ✅ Enhanced tooltips with keyboard shortcuts

#### Modeling Features
- ✅ Sweep feature with path and profile, twist and scale options
- ✅ Helix/Spiral feature for screws, springs, etc.
- ✅ Shell feature with wall thickness and face selection
- ✅ Draft feature for mold design
- ✅ Enhanced Chamfer with multiple types (Equal, Two Distances, Distance-Angle)
- ✅ Mirror feature with merge options
- ✅ Thread feature with standards (ISO, UNC, etc.)
- ✅ Rib feature for reinforcement

### Added (Previous)
- ✅ Vollständige 3D Viewport Rendering mit Coin3D/SoQt Integration
- ✅ FreeCAD Integration mit vollständiger Sketch/Part/Drawing Synchronisation
- ✅ Import/Export für STEP, IGES, STL, DWG, DXF, OBJ, PLY, 3MF, GLTF
- ✅ Professioneller Constraint Solver (Newton-Raphson)
- ✅ Simulation Service: FEA, Motion, Deflection, Optimization, Thermal Analysis
- ✅ Sheet Metal Service: Flange, Bend, Cut, Unfold/Refold
- ✅ Routing Service: A*-basierte Pfadfindung
- ✅ Direct Edit Service: Face-Modifikation, Offset, Deletion, Freeform
- ✅ Drawing Generation mit TechDraw Integration
- ✅ Project Management: Save/Load, Checkpoints, Auto-Save
- ✅ User Authentication: Login, Registration, Session Management
- ✅ AI Integration: OpenAI/Anthropic Support
- ✅ Auto-Update System: In-Place Updates ohne Neuinstallation
- ✅ Modern UI: Inventor-Style Theme mit modularer Dock-Layout
- ✅ NSIS Installer mit Desktop-Shortcuts und File-Associations

### Changed
- 🔄 Branding: CADursor → Hydra CAD
- 🔄 Installer: CADursorSetup.exe → HydraCADSetup.exe
- 🔄 Installationspfad: `C:\Program Files\CADursor` → `C:\Program Files\Hydra CAD`

### Fixed
- ✅ Alle Linter-Fehler behoben
- ✅ Alle fehlenden Includes hinzugefügt
- ✅ Serialization/Deserialization korrigiert
- ✅ Update-System vollständig implementiert

---

**Version**: 2.0.0  
**Status**: Produktionsbereit ✅
