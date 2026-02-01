# Native Dateiformate (Ref. §2.1)

Äquivalent zu Inventor (.ipt, .iam, .idw, .ipj). Speichern/Laden erfolgt über **ProjectFileService**.

## Dateitypen

| Erweiterung     | Inhalt              | Beschreibung |
|-----------------|---------------------|--------------|
| **.hcad**       | Bauteil (Part)      | Einzelteil: Skizzen, Features, Arbeitselemente, Parameter, Konfigurationen. |
| **.hcasm**      | Baugruppe (Assembly)| Zusammenbau: Komponenten (.hcad/Referenzen), Mates, Gelenke, Explosion, Konfigurationen. |
| **.hcdrw**      | Zeichnung (Drawing) | Technische Zeichnung: Blätter, Ansichten, Bemaßung, Stückliste (optional). |
| **.hcadproject**| Projektkonfiguration| Arbeitsbereich, Suchpfade, Bibliotheken, Vorlagen (Inventor-.ipj-Äquivalent). |

## Speicherort & Verknüpfung

- Projektdatei (z. B. `MeinProjekt.hcasm`) und Konfiguration (`MeinProjekt.hcadproject`) liegen im gleichen Verzeichnis (gleicher Basisname).
- `ProjectFileService::projectConfigPathForProject(project_file_path)` liefert den Pfad zur .hcadproject-Datei.
- Laden/Speichern: `ProjectFileService::loadProject` / `saveProject` (Assembly inkl. eingebetteter Part-Daten); Skizzen optional mitgeliefert.

## Format (aktuell)

- Textbasiert: Header `CADURSOR_PROJECT`, `VERSION`, `ASSEMBLY_DATA:` mit serialisierten Assembly-/Part-/Sketch-Daten, `END_PROJECT`.
- Erweiterungen: Konfigurationen (Part/Assembly), Zeichnungsblätter und .hcdrw-Export können bei Bedarf ergänzt werden.
