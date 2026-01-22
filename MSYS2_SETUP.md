# MSYS2 Setup für CADursor Build

## Schritt 1: MSYS2 Terminal öffnen

Öffnen Sie das **MSYS2 UCRT64** Terminal:
- Startmenü → MSYS2 → MSYS2 UCRT64
- Oder: `C:\msys64\ucrt64.exe`

## Schritt 2: System aktualisieren

```bash
pacman -Syu
```

Falls es fragt, ob Sie die Terminal-Session beenden möchten, bejahen Sie und starten Sie das Terminal neu.

## Schritt 3: GCC und CMake installieren

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-cmake
```

Bei der Abfrage "Proceed with installation? [Y/n]" → **Y** drücken

## Schritt 4: Qt installieren (für CADursor UI)

```bash
pacman -S mingw-w64-ucrt-x86_64-qt5-base
pacman -S mingw-w64-ucrt-x86_64-qt5-tools
```

## Schritt 5: PATH für PowerShell hinzufügen

**Option A: Temporär (nur für diese PowerShell-Session):**
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
```

**Option B: Permanently (für alle PowerShell-Sessions):**
```powershell
# PowerShell als Administrator ausführen
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\msys64\ucrt64\bin", [EnvironmentVariableTarget]::Machine)
```

**Option C: Über Systemsteuerung:**
1. Windows-Taste + Pause → "Erweiterte Systemeinstellungen"
2. "Umgebungsvariablen" → "Path" → "Bearbeiten"
3. "Neu" → `C:\msys64\ucrt64\bin` hinzufügen
4. OK → Terminal neu starten

## Schritt 6: Prüfen ob alles funktioniert

In PowerShell:
```powershell
gcc --version
cmake --version
```

Sollte die Versionen anzeigen.

## Schritt 7: Build ausführen

```powershell
cd "C:\Users\Darius Herrmann\cad\CAD"
.\build_installer.ps1
```

Das Script sollte jetzt MinGW automatisch finden und verwenden!

## Alternative: Direkt aus MSYS2 Terminal bauen

Falls PATH-Probleme auftreten, können Sie direkt aus dem MSYS2 Terminal bauen:

```bash
# Im MSYS2 UCRT64 Terminal
cd /c/Users/Darius\ Herrmann/cad/CAD
cmake -S . -B build -G "MinGW Makefiles" -DCAD_USE_QT=ON
cmake --build build
```

Die .exe finden Sie dann in: `build/cad_desktop.exe`

