# UI-Struktur – Ribbon & Befehle

Übersicht, damit jede Funktion eine passende UI hat und die Aufteilung logisch bleibt.

## Tabs (Ribbon)

| Tab | Inhalt | Beschreibung |
|-----|--------|--------------|
| **Sketch** | Create (Line, Rectangle, Circle, Arc), Constraints | Skizzen und Beziehungen |
| **3D Model** (Part) | Features, Sheet Metal, Pattern, Direct | Bauteil-Features, Blech, Muster, Direktbearbeitung |
| **Assembly** | Assemble, Presentation, Routing, Welding, Simplify | Baugruppen, Explosion, Rohr/Schweiß |
| **Drawing** | Views (BaseView, Section, DetailView, Dimension, PartsList) | Zeichnungen und Stücklisten |
| **Inspect** | Analysis, Simulation | Maß, Kollision, Schnitt, FEA/Motion |
| **Manage** | Parameters, Interop, MBD | Parameter, iLogic, Import/Export, MBD |
| **View** | Display, Render | Sichtbarkeit, Darstellung, Rendering |

## Features (Part) – vollständig in UI

- **Extrude, Revolve, Loft, Hole, HoleThroughAll, Fillet, Chamfer, Shell, Mirror** → alle in Ribbon (Features) und Command Line.
- **Pattern:** RectangularPattern, CircularPattern, CurvePattern, **FacePattern** (Flächenmuster) → alle in Ribbon (Pattern) und Command Line.
- **Sheet Metal:** Flange, Bend, Unfold, Refold, Punch, Bead, SheetMetalRules, ExportFlatDXF.

## Assembly – Mates & Darstellung

- **Mate:** Öffnet Dialog (Typ: Coincident, Parallel, Distance, Tangent, Concentric, Perpendicular; optional Wert). Alternativ Einzelbefehle **Flush, Angle, Parallel, Distance** für Schnellzugriff.
- **ExplosionView** → Explosionsdarstellung ein/aus.
- **Suppress** (Browser-Kontextmenü) → unterdrückt ausgewähltes Feature (Backend: `setFeatureSuppressed`).

## Konsistenz

- **RibbonLayout** (nicht-Qt-Layout) und **QtRibbon** (Qt) listen dieselben Befehle pro Tab; **QtCommandLine** `valid_commands_` und **QtMainWindow** `categoryForCommand` sind angeglichen.
- Neue Backend-Funktionen (z. B. Chamfer, Shell, Mirror, FacePattern, Suppress-Anbindung) sind in Ribbon/Command Line und AppController verdrahtet.

## Keine eigene UI (bewusst oder Stub)

- **Rollback-Leiste, Flexible Subassembly, Component Interfaces, PinSlot-Joint, Arrangements** → Backend-API vorhanden; UI später (z. B. Property Panel, Browser-Kontext).
- **Content Center, Reuse Library, Topology Optimization** → Stubs ohne Dialoge.
