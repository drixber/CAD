# Changelog - Hydra CAD

## [Unreleased]

_Nothing yet._

---

## [3.0.16] - 2026-01-31

### Added
- **Referenz-Inventor/darius.txt – umgesetzte Punkte:**
  - **Navigationssystem:** Zoom auf Auswahl (fitToSelection) – Button „Zoom Selection“ in Viewport-Toolbar; SoQtViewerIntegration/Viewport3D rahmen Auswahl per Bounding Box.
  - **Startfenster/Help:** Help-Menü mit Get Started, Tutorials, Sample Projects, About Hydra CAD.
  - **Skizzen:** Constraint-Typen Symmetric und Fixed; Geometrietypen Ellipse, Polygon, Spline, Text (addEllipse/addPolygon/addSpline/addText); Solver-Unterstützung; Serialisierung in ProjectFileService; Eigen-Kern.
  - **3D-Features:** Extrusionsmodus Join/Cut/Intersect/NewBody (ExtrudeMode in Feature, createExtrude/applyExtrude, Serialisierung FEXTRUDE_MODE).
  - **Arbeitselemente (Ref §9.4):** ReferenceGeometry.h (Point3D, Vector3D, WorkPlane, WorkAxis, WorkPoint, CoordinateSystem); Part: addWorkPlane/addWorkPlaneOffset, addWorkAxis/addWorkAxisBase, addWorkPoint, addCoordinateSystem; Serialisierung WP/WA/WPT/CS in ProjectFileService.
  - **Modellbrowser:** Reference Geometry mit Work Planes, Work Axes, Work Points; setWorkPlanes/setWorkAxes/setWorkPoints; QtMainWindow::setReferenceGeometry; AppController füllt Browser aus erstem Part (Demo-Arbeitsebene „XY Offset 10“).
  - **Baugruppen-Mates (Ref §13):** MateType Concentric und Tangent; createConcentric/createTangent; solveMates und getDegreesOfFreedom erweitert.
  - **Projektverwaltung (Ref §4):** ProjectConfig (working_directory, search_paths, library_paths); .hcadproject-Datei (loadProjectConfig/saveProjectConfig); beim Laden/Speichern eines Projekts wird .hcadproject im gleichen Verzeichnis geladen/gespeichert (projectConfigPathForProject).
- **User-Backend & Lizenzen (ohne Cloud):**
  - **Backend (Phase 1+3):** `backend/` – FastAPI, SQLite (users + licenses), JWT (python-jose), bcrypt (passlib). Endpoints: `POST /api/auth/register`, `POST /api/auth/login`, `POST /api/auth/refresh`, `GET /api/auth/me`, `POST /api/license/activate`, `POST /api/license/check`. Admin-Script `admin_create_license.py` für Lizenzschlüssel.
  - **App an API (Phase 2):** `UserAuthService::registerViaApi`/`loginViaApi` mit HttpClient (Qt Network); Token in QSettings; Auto-Login per Refresh/Me beim Start; `getAccessToken()`, `saveSession`/`clearStoredTokens`, Logout löscht Tokens.
  - **App Lizenzen (Phase 4):** `LicenseService` (check, activate, isAllowed, Machine-ID in QSettings); UI „Lizenz aktivieren“ (Settings → Activate License), `QtLicenseDialog`.
- Doku: `docs/TODO_USER_BACKEND_LICENSES.md` auf Stand (Phasen 1–4 erledigt); `backend/README.md` (API, Env, Lizenzschlüssel anlegen).

### Fixed
- **Release-Assets-Workflow:** `actions/checkout` im Upload-Job (behebt „not a git repository“ bei `gh release upload`); `shell: bash` für Schritt „Tag ermitteln“ unter Windows (PowerShell-ParserError vermieden).

---

## [3.0.14] - 2026-01-31

### Added
- **GUI Inventor-Niveau (TODO_GUI_INVENTOR_LEVEL) – umgesetzte Punkte:**
  - Ribbon: **Suchleiste** „Search Help & Commands…“ rechts neben den Tabs; **Dokument-Tabs** unter dem Ribbon (Placeholder „Document 1"); **Extrude-Dropdown** (Normal/Reverse/Both) in der Features-Gruppe.
  - Model Browser: **Filter-Leiste** und **„+“-Button** in der Titelleiste des Panels; **Sichtbarkeits-Spalte** (Auge-Icon) neben Einträgen.
  - Property-Panel: **Breadcrumb** unter der Titelleiste (`setBreadcrumb`); Anzeige bei Kontext (z. B. „Extrude > Sketch1").

### Changed
- QtMainWindow: Ribbon-Bereich als Container mit Suchzeile und Dokument-Tab-Leiste; Model-Browser-Dock mit Filter und „+“ in der Titelleiste.
- QtRibbon: `searchLineEdit()` und `documentTabBar()` für Integration; Extrude als MenuButton mit Normal/Reverse/Both.
- QtBrowserTree: zweite Spalte für Sichtbarkeits-Icon; Icons für View, Origin, Solid Bodies, Sketches, Parts, Assemblies, Drawings, Mates, Recent, End of Part.
- QtPropertyPanel: `setBreadcrumb()`; Breadcrumb-Label unter Titelleiste.
- docs/TODO_GUI_INVENTOR_LEVEL.md: 1.5, 1.6, 1.8, 2.4, 2.5, 3.2 als erledigt markiert.

### Cleanup
- **darius.txt** entfernt (redundant zu docs/REFERENZ_AUTODESK_INVENTOR.md).
- **.gitignore:** `backend/tests/*.db`, `backend/tests/.pytest_cache/` ergänzt.
- **Doppelte Includes** entfernt: QtMainWindow.cpp (QVBoxLayout, QSettings), QtPropertyPanel.h (QWidget).

---

## [3.0.13] - 2026-01-31

### Added
- **GUI auf Inventor-Niveau (Roadmap):** docs/TODO_GUI_INVENTOR_LEVEL.md – aus Referenz-Screenshots abgeleitete TODO (Ribbon, Model Browser, Property-Panels, Viewport/ViewCube, Layout). AI-Chat bleibt rechts.
- Ribbon: Command-Groups pro Tab **horizontal** (Inventor-Style); große Icons (32×32) mit Text unter Icon; Gruppen als QFrame mit Border und Hover.
- Model Browser: **Kontextmenü** (Rechtsklick) mit Visibility, Suppress, Create New Component, Place From File, Edit, Properties, Rename, Delete, Copy, Paste, Search, Show Dependencies (Aktionen noch Stubs).
- Property-Panel: **Sektionen** „Selection“ und „Status“ als QGroupBox (flach, Inventor-Style); ThemeManager-Styling für QGroupBox und Ribbon-Group.

### Changed
- ThemeManager: QGroupBox-Styling (Border, Radius, Padding, Title); QFrame#ribbonGroup und QLabel#ribbonGroupLabel für Ribbon-Gruppen.
- Ribbon: buildCommandTab nutzt QHBoxLayout für Gruppen; buildGroup liefert QFrame mit Label und ToolButtons (TextUnderIcon, 56×52 min).

---

## [3.0.12] - 2026-01-27

### Added
- Update-Dialog: Changelog aus GitHub-API (Release-Body), Fallback „Siehe Release-Seite“
- Update-Checker: flexible Asset-Namen (HydraCADSetup.exe, app-windows.zip, hydracad-linux-portable.tar.gz) für direkten Download
- Dynamischer Dateiname beim Update-Download aus Asset-URL; unter Windows nur bei .exe automatischer Installer-Start, sonst Hinweis zum manuellen Entpacken/Ausführen
- GitHub-Workflow „Release bei Tag-Push erstellen“ (create-release-on-tag.yml) für automatisches Release bei Tag v*
- Workflow „Release – Assets anhängen“: manueller Trigger (workflow_dispatch) mit tag_name zum Nachziehen von Assets
- AUR-Skript packaging/arch/aur-upload.sh für automatische AUR-Paketaktualisierung
- Dokumentation: RELEASE_RUNDUMFLUG.md (Checkliste bis neues Release), INSTALLATION.md erweitert (Release-Troubleshooting)

### Changed
- ThemeManager: moderneres UI (Dialoge, ProgressBar, CheckBox, Ribbon, Buttons, Listen, Statusleiste, Scrollbars) mit einheitlicher Palette und Border-Radius
- QtUpdateDialog: Hardcoded-Farben durch ThemeManager-Palette ersetzt (inkl. Dark Mode)
- Arbeitsverzeichnis der App beim Start auf Installationsverzeichnis gesetzt (verhindert „schwarzes Fenster“ nach Update)
- Linux .desktop: Icon=applications-engineering, TryExec=cad_desktop

### Fixed
- Qt6-Deprecations: QMouseEvent::x()/y() → position().x()/y() (Viewport3D); QCryptographicHash::addData mit QByteArrayView (UserAuthService)
- tmpnam durch portable Temp-Datei (std::filesystem::temp_directory_path + std::random_device) in GrokProvider und OpenAIProvider ersetzt
- create-release-on-tag: actions/checkout@v4 ergänzt („not a git repository“ behoben)

---

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
- ✅ Eigen-Kern als alleiniges Geometrie-Backend (Sketch/Part/Drawing)
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

**Version**: 3.0.13  
**Status**: Produktionsbereit ✅
