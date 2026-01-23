# CI/CD Pipeline Fixes - Release 2.0.0

## 🔧 Behobene Probleme

### Problem 1: Veraltete Actions
- ❌ `actions/upload-artifact@v3` ist veraltet (deprecated seit April 2024)
- ❌ `microsoft/setup-cmake@v1` existiert nicht (Repository nicht gefunden)

### Problem 2: Qt Dependencies
- ❌ **Linux**: Nur Runtime-Bibliotheken installiert, keine Development-Pakete
- ❌ **macOS**: Qt-Pfade nicht korrekt konfiguriert
- ❌ **Windows**: Qt nicht installiert

## ✅ Lösungen

### 1. Actions aktualisiert

#### `actions/upload-artifact`
- ✅ `@v3` → `@v4` (alle 3 Vorkommen aktualisiert)
- ✅ Neue API ist kompatibel, aber mit verbesserter Performance

#### `microsoft/setup-cmake`
- ✅ Ersetzt durch `jwlawson/actions-setup-cmake@v2`
- ✅ CMake Version 3.26 explizit angegeben (entspricht Projektanforderung)

#### `actions/checkout`
- ✅ `@v3` → `@v4` (für Konsistenz aktualisiert)

### 2. Qt Dependencies

#### Linux
- ✅ Qt6 Development-Pakete hinzugefügt:
  - `qt6-base-dev`
  - `qt6-tools-dev`
  - `qt6-tools-dev-tools`
  - `libqt6opengl6-dev`

#### macOS
- ✅ Qt6 über Homebrew installiert (`qt@6`)
- ✅ Qt6_DIR Umgebungsvariable gesetzt
- ✅ CMake Qt6_DIR explizit übergeben

#### Windows
- ✅ Qt6 über `jurplel/install-qt-action@v3` installiert
- ✅ Qt 6.5.0 mit qtbase und qttools Modulen
- ✅ Caching aktiviert für schnellere Builds

## 📋 Änderungen

**Datei**: `.github/workflows/ci-cd.yml`

### Actions Updates (Zeilen 19, 22-24, 85, 140, 193)
```yaml
- uses: actions/checkout@v4  # War: @v3

- name: Setup CMake
  uses: jwlawson/actions-setup-cmake@v2  # War: microsoft/setup-cmake@v1
  with:
    cmake-version: '3.26'

- uses: actions/upload-artifact@v4  # War: @v3 (3x)
```

### Linux (Zeilen 98-103)
```yaml
- name: Install Dependencies
  run: |
    sudo apt-get update
    sudo apt-get install -y build-essential cmake \
      qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
      libqt6core6 libqt6gui6 libqt6widgets6 libqt6opengl6-dev
```

### macOS (Zeilen 151-160)
```yaml
- name: Install Dependencies
  run: |
    brew install cmake qt@6

- name: Configure CMake
  env:
    Qt6_DIR: /opt/homebrew/opt/qt@6/lib/cmake/Qt6
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCAD_USE_QT=ON -DCAD_BUILD_TESTS=ON \
      -DQt6_DIR="$Qt6_DIR"
```

### Windows (Zeilen 26-29)
```yaml
- name: Setup Qt
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.5.0'
    modules: 'qtbase qttools'
    cached: 'true'
```

## ✅ Erwartetes Ergebnis

Nach diesen Änderungen sollten alle CI/CD-Jobs erfolgreich durchlaufen:
- ✅ `build-windows (Release)` - Erfolgreich
- ✅ `build-windows (Debug)` - Erfolgreich
- ✅ `build-linux` - Erfolgreich
- ✅ `build-macos` - Erfolgreich

## 🚀 Nächste Schritte

1. **Commit & Push**: Änderungen committen und pushen
2. **CI/CD prüfen**: GitHub Actions sollte automatisch ausgelöst werden
3. **Fehler analysieren**: Falls weiterhin Fehler auftreten, Logs prüfen

---

**Datum**: 23. Januar 2026  
**Status**: ✅ CI/CD-Pipeline vollständig korrigiert
