# Plan: 3D-Drucker-Anbindung (vollständig)

Strukturierter Überblick über Umfang und Arbeitsaufwand einer kompletten 3D-Drucker-Integration (OctoPrint / Moonraker/Klipper).

---

## 1. Ziel

- **Direkte Anbindung:** Aktuelles Modell als STL an einen konfigurierten 3D-Drucker senden (OctoPrint oder Moonraker/Klipper).
- **Kein Slicer in der App:** STL-Upload; Slicing erfolgt auf dem Drucker-Host (OctoPrint-Plugin) oder extern (Nutzer öffnet STL in Cura/PrusaSlicer). Optional später: „Mit Slicer öffnen“.

---

## 2. Komponenten-Übersicht

| Komponente | Beschreibung | Abhängigkeiten |
|------------|--------------|-----------------|
| **HttpClient** | Erweiterung: `uploadFile(url, file_path, headers)` für multipart/form-data | Qt Network (bereits für get/post) |
| **PrinterProfile** | Datenmodell: id, name, type (octoprint \| moonraker), base_url, api_key | – |
| **PrinterService** | Drucker-Liste, hinzufügen/entfernen, Verbindung testen, STL hochladen, optional Druck starten | HttpClient, Persistenz |
| **Persistenz** | Drucker-Profile speichern (QSettings oder JSON im App-Datenverzeichnis) | Qt (QSettings) |
| **QtPrintersDialog** | UI: Drucker listieren, hinzufügen/bearbeiten/löschen, Verbindung testen | PrinterService |
| **Send to Printer** | Menü/Ribbon: „An 3D-Drucker senden“ → Drucker wählen → STL exportieren → upload | IoPipeline/Export, PrinterService, QtMainWindow |
| **AppController** | PrinterService einbinden, „Send to Printer“ und Drucker-Dialog anbinden | wie oben |

---

## 3. Arbeitsaufwand (geschätzt)

| # | Aufgabe | Aufwand (Stunden) | Priorität |
|---|---------|-------------------|-----------|
| **3.1** | **HttpClient:** `uploadFile(url, file_path, extra_headers)` (multipart/form-data, nur bei CAD_USE_QT_NETWORK) | 1–2 | Pflicht |
| **3.2** | **PrinterProfile + PrinterService:** Modell, Liste, add/remove, load/save (QSettings), `sendStlToPrinter(id, path)`, `testConnection(id)` für OctoPrint + Moonraker | 3–4 | Pflicht |
| **3.3** | **QtPrintersDialog:** Liste, Hinzufügen/Bearbeiten (Name, Typ, URL, API-Key), Löschen, „Verbindung testen“ | 2–3 | Pflicht |
| **3.4** | **Send to Printer:** Ribbon/Menü-Eintrag, Dialog Drucker wählen, aktuelles Modell als STL exportieren (temp), Upload, optional „Druck starten“ | 2–3 | Pflicht |
| **3.5** | **AppController:** PrinterService, Dialog öffnen, Export→Upload-Flow anbinden | 1–2 | Pflicht |
| **3.6** | **Fehlerbehandlung & Status:** Meldungen bei Upload-Fehler, Timeout, ungültiger URL/API-Key | 0,5–1 | Empfohlen |
| **3.7** | **Dokumentation:** Kurzanleitung in INSTALLATION oder eigenes Doc (OctoPrint/Moonraker einrichten, API-Key) | 0,5–1 | Empfohlen |
| **Optional** | Druckstatus abfragen (OctoPrint/Moonraker Job-Status), Fortschritt anzeigen | 2–3 | Optional |
| **Optional** | „Mit Slicer öffnen“ (STL speichern + Cura/PrusaSlicer starten) | 1–2 | Optional |

**Summe Pflicht (3.1–3.5):** ca. **9–14 Stunden**  
**Mit 3.6–3.7:** ca. **10–16 Stunden**

---

## 4. Technische Details

### 4.1 OctoPrint

- **Upload:** `POST /api/files/local`  
  - Body: multipart/form-data, Feld `file` = STL-Datei  
  - Header: `X-Api-Key: <api_key>`  
  - Form-Felder (optional): `select=true`, `print=false`
- **Druck starten:** Nach Upload `POST /api/files/local` mit `print=true` oder `POST /api/job` mit `command: start` und `filename`.

### 4.2 Moonraker (Klipper)

- **Upload:** `POST /server/files/upload`  
  - Body: multipart/form-data, Feld `file` = STL oder G-Code  
  - Header: `X-Api-Key: <api_key>` (wenn Auth aktiv)  
  - Form (optional): `print=true` (für G-Code), `root=gcodes`
- **Druck starten:** `POST /printer/print/start?filename=...` (nach Upload, Dateiname relativ zu gcodes).

### 4.3 Ablauf „An 3D-Drucker senden“

1. Nutzer wählt „An 3D-Drucker senden“ (Ribbon oder Menü).
2. Wenn keine Drucker konfiguriert: Hinweis + Öffnen des Drucker-Dialogs (Einstellungen → 3D-Drucker).
3. Dialog „Drucker wählen“: Dropdown/Liste der gespeicherten Drucker, optional „Druck nach Upload starten“ (nur bei G-Code sinnvoll; bei STL: nein oder Hinweis „Bitte auf Host slicen“).
4. Aktuelles Modell als STL exportieren: z. B. `io_service_.exportStl(temp_path)` oder Export über bestehenden Export-Pfad; bei Assembly/Part entscheiden (z. B. erste Part oder gesamte Szene).
5. `PrinterService::sendStlToPrinter(printer_id, temp_path)` aufrufen.
6. Bei Erfolg: Meldung „Datei an [Druckername] gesendet.“; Temp-Datei optional löschen.

---

## 5. Dateien (neu/geändert)

| Datei | Aktion |
|-------|--------|
| `docs/PLAN_3D_DRUCKER_ANBINDUNG.md` | Neu (dieses Dokument) |
| `src/app/HttpClient.h` / `.cpp` | Erweiterung: `uploadFile()` |
| `src/app/PrinterProfile.h` | Neu (Struktur: id, name, type, base_url, api_key) |
| `src/app/PrinterService.h` / `.cpp` | Neu |
| `src/ui/qt/QtPrintersDialog.h` / `.cpp` | Neu |
| `src/app/AppController.h` / `.cpp` | PrinterService, Send-to-Printer, Dialog öffnen |
| `src/ui/qt/QtMainWindow.h` / `.cpp` | Menü/Ribbon „An 3D-Drucker senden“, Drucker-Dialog aus Einstellungen |
| `src/app/CMakeLists.txt` | PrinterService, ggf. PrinterProfile |
| `src/ui/qt/CMakeLists.txt` | QtPrintersDialog |

---

## 6. Kurzfassung Arbeitsaufwand

| Bereich | Stunden (min–max) |
|---------|--------------------|
| HTTP Upload (HttpClient) | 1–2 |
| PrinterProfile + PrinterService | 3–4 |
| QtPrintersDialog | 2–3 |
| Send-to-Printer Flow (Export + UI) | 2–3 |
| AppController-Anbindung | 1–2 |
| Fehlerbehandlung + Doku | 1–2 |
| **Gesamt (Pflicht + empfohlen)** | **ca. 10–16 h** |

---

## 7. Umgesetzt (Stand)

- **HttpClient:** `uploadFile(url, file_path, headers)` (multipart/form-data, nur bei CAD_USE_QT_NETWORK).
- **PrinterProfile + PrinterService:** Liste, add/remove/update, QSettings-Persistenz, `testConnection(id)`, `sendStlToPrinter(id, stl_path)` für OctoPrint und Moonraker.
- **QtPrintersDialog:** Drucker listieren, hinzufügen/bearbeiten/löschen, Verbindung testen; `setSendMode(true)` für „Send to Printer“ (nur Liste + „Send current model“).
- **Menü:** **File → Send to 3D Printer…** und **Settings → 3D Printers…**.
- **AppController:** PrinterService mit HttpClient, Handler für beide Dialoge; „Send to 3D Printer“ exportiert aktuelles Modell als STL (temp), lädt auf gewählten Drucker hoch, Meldung bei Erfolg/Fehler.

**Hinweis:** Ein eventueller Linkerfehler zu `Viewport3D::setPreferredDragMode` ist projektseitig und nicht durch die 3D-Drucker-Anbindung verursacht.
