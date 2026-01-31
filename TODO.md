# Hydra CAD – Offene Punkte & Deep Research

Stand: Nach vollständiger Codebase-Analyse. Diese Liste fasst zusammen, was noch fehlt, nicht vollständig funktioniert oder sinnvoll ergänzt werden kann.

---

## 1. Release & Build

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 1 | **Portable ZIP** | ✅ Erledigt | `release.yml` erstellt `app-windows.zip` und lädt es als Asset hoch. |
| 2 | **Version aus Git-Tag** | ✅ Erledigt | Version wird früh extrahiert und als `APP_VERSION` an CMake übergeben. |
| 3 | **Code-Signing** | Optional | Signierung ist im `release.yml` vorgesehen (wenn `WINDOWS_PFX_*` gesetzt). Optional: in README/docs kurz dokumentieren, welche Secrets nötig sind und dass dann EXE + ggf. zukünftiges ZIP signiert werden. |

---

## 2. Übersetzungen & Sprache

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 4 | **Übersetzungsdateien** | ✅ Erledigt | `.ts` für DE/ZH/JA in `src/ui/resources/i18n/`, lrelease in CMake, `.qm` neben EXE. |

---

## 3. Import/Export

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 5 | **Import/Export-Pfad** | ✅ Erledigt | File-Dialog in QtMainWindow, Handler setzen Pfad/Format an IoPipeline. |

---

## 4. Undo/Redo & Ribbon

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 6 | **Undo/Redo** | ✅ Erledigt | UndoStack in AppController, executeCommand "Undo"/"Redo", Shortcuts Ctrl+Z/Y. |
| 7 | **Ribbon-Klick führt Befehl aus** | ✅ Erledigt | Ribbon ruft command_handler_ (executeCommand) auf. |

---

## 5. Projekt & Dateien

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 8 | **Unsaved-Changes-Dialog** | ✅ Erledigt | askUnsavedChanges/getSavePath, Dialog Save/Discard/Cancel beim Öffnen. |
| 9 | **Projekt-Serialisierung** | Vereinfacht | `ProjectFileService::serializeAssembly` speichert nur Assembly-Komponenten (Part-Name + Transform) und Mates. Part-Features und Sketch-Geometrie werden nicht serialisiert; `deserializeAssembly` erstellt leere Parts mit Namen. Für vollständige Projekte: Part-Features und Sketches (Geometrie, Constraints) in das Projektformat aufnehmen und beim Laden wiederherstellen. |
| 10 | **Checkpoints-UI** | ✅ Erledigt | File → Manage Checkpoints…, Liste/Open/Delete. |

---

## 6. User & Einstellungen

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 11 | **Profil-Menü** | ✅ Erledigt | Profile-Handler zeigt Nutzer/Email in QMessageBox. |

---

## 7. AI

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 12 | **Anthropic** | Geplant | Im Code (z. B. `AppController::setupAIService`, AI-Settings) ist Anthropic als Option erwähnt, aber auskommentiert ("will be implemented later"). README nennt "Anthropic geplant". Offen: Anthropic-Provider analog zu OpenAI/Grok implementieren und in der UI wählbar machen. |

---

## 8. CI/CD & Tests

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 13 | **Tests in CI** | ✅ Erledigt | `CAD_BUILD_TESTS=ON` in ci-cd.yml, ctest-Schritt (continue-on-error). |
| 14 | **Linux/macOS** | Pausiert | Laut README ist CI für Linux/macOS derzeit pausiert. Wenn wieder gewünscht: entsprechende Jobs in `ci-cd.yml` (oder separatem Workflow) reaktivieren. |

---

## 9. Dokumentation

| # | Thema | Status | Details |
|---|--------|--------|---------|
| 15 | **INSTALLATION.md** | ✅ Erledigt | Qt 6, CMake 3.26, aktuelle Build-Schritte. |

---

## 10. Optional / Nice-to-have

- **Grok/SSE**: Streaming nutzt derzeit externes `curl`; optional später auf natives HTTP/SSE umstellen, falls gewünscht.
- **UndoStack Snapshots**: `serializeState`/`deserializeState` sind Platzhalter (Commands werden nicht serialisiert); für echte Checkpoints/Snapshots müsste die Command-Serialisierung implementiert werden.
- **Ribbon vs. Command-Line**: Klären, ob Ribbon nur "Vorschau" in der Command-Line sein soll oder immer sofort ausführen soll (siehe Punkt 7).

---

## Kurz-Übersicht (Priorität)

1. **Release**: Portable ZIP erstellen + hochladen; Version aus Tag an Build übergeben.
2. **Ribbon**: Klick soll Befehl ausführen; Undo/Redo an UndoStack anbinden.
3. **Import/Export**: File-Dialog und echte Pfad/Format-Weitergabe.
4. **Projekt**: Unsaved-Changes-Dialog; langfristig vollständige Serialisierung (Features/Sketches).
5. **Sprache**: Übersetzungsdateien anlegen und einbinden.
6. **Profil**: Profile-Action verbinden oder entfernen.
7. **Doku**: INSTALLATION.md auf Qt 6 und aktuelle Schritte aktualisieren.
8. **CI**: Optional Tests aktivieren; optional Linux/macOS wieder aktivieren.

Alle Punkte sind in der Cursor-TODO-Liste als Aufgaben hinterlegt und können dort abgearbeitet werden.
