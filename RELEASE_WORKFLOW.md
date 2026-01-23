# GitHub Release Workflow - Automatische Installer-Erstellung

## 🎯 Ziel

Fertige Installer-EXE direkt in GitHub Releases verfügbar, ohne dass Benutzer selbst bauen müssen.

## ✅ Implementiert

### 1. Release-Workflow (`.github/workflows/release.yml`)

**Auslöser:**
- Automatisch bei Git Tags (z.B. `v2.0.0`)
- Manuell über GitHub Actions UI mit Version-Eingabe

**Funktionen:**
- ✅ Automatischer Build der Anwendung
- ✅ NSIS-Installer-Erstellung
- ✅ GitHub Release mit Installer als Download
- ✅ Automatische Versionserkennung

### 2. CI/CD-Pipeline aktualisiert

- ✅ Actions auf v4 aktualisiert (upload-artifact, checkout)
- ✅ CMake Setup korrigiert (jwlawson/actions-setup-cmake)
- ✅ NSIS-Installation in CI/CD
- ✅ Installer-Build für Release-Konfiguration

### 3. NSIS-Script optimiert

- ✅ PROJECT_ROOT Variable für flexible Pfade
- ✅ Alle Branding-Referenzen korrigiert (Hydra CAD)
- ✅ Uninstaller korrigiert

### 4. README aktualisiert

- ✅ Download-Link zu GitHub Releases
- ✅ Einfache Installations-Anleitung
- ✅ Option für eigenen Build (optional)

## 🚀 Verwendung

### Automatisches Release erstellen

1. **Tag erstellen:**
   ```bash
   git tag v2.0.0
   git push origin v2.0.0
   ```

2. **GitHub Actions läuft automatisch:**
   - Build der Anwendung
   - Erstellung des Installers
   - Upload zu GitHub Releases

3. **Release verfügbar:**
   - Gehen Sie zu: https://github.com/drixber/CAD/releases
   - Download `HydraCADSetup.exe`
   - Doppelklick → Installation

### Manuelles Release erstellen

1. GitHub → Actions → "Create Release with Installer"
2. "Run workflow" klicken
3. Version eingeben (z.B. `v2.0.1`)
4. Workflow läuft und erstellt Release

## 📋 Workflow-Schritte

1. **Checkout** - Code auschecken
2. **Setup CMake** - CMake 3.26 installieren
3. **Setup Qt** - Qt 6.5.0 installieren
4. **Install NSIS** - NSIS über Chocolatey
5. **Configure CMake** - Projekt konfigurieren
6. **Build Application** - Release-Build
7. **Copy DLLs** - Qt-DLLs kopieren
8. **Create Installer** - NSIS-Installer bauen
9. **Verify Installer** - Installer prüfen
10. **Extract Version** - Version aus Tag extrahieren
11. **Create Release** - GitHub Release mit Installer

## 📁 Dateien

- `.github/workflows/release.yml` - Release-Workflow
- `.github/workflows/ci-cd.yml` - CI/CD-Pipeline (aktualisiert)
- `installer/hydracad.nsi` - NSIS-Script (optimiert)
- `README.md` - Download-Links hinzugefügt

## ✅ Status

**Alle Aufgaben abgeschlossen:**
- ✅ Release-Workflow erstellt
- ✅ CI/CD-Pipeline aktualisiert
- ✅ NSIS-Script optimiert
- ✅ README mit Download-Links
- ✅ Automatische Installer-Erstellung

**Nächster Schritt:** Tag erstellen und Release testen!

---

**Datum**: 23. Januar 2026  
**Status**: ✅ **Fertig - Bereit für Release**
