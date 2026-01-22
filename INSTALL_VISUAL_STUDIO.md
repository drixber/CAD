# Visual Studio Installation - Schnellhilfe

## Problem: "Visual Studio nicht gefunden"

Wenn das Build-Script Visual Studio nicht findet, müssen Sie Visual Studio installieren.

## Installation

### Option 1: Visual Studio 2022 (Empfohlen)

1. **Download:**
   - Gehen Sie zu: https://visualstudio.microsoft.com/downloads/
   - Laden Sie "Visual Studio 2022 Community" herunter (kostenlos)

2. **Installation:**
   - Führen Sie den Installer aus
   - **WICHTIG:** Wählen Sie die Workload "Desktop development with C++"
   - Diese enthält:
     - MSVC v143 - VS 2022 C++ x64/x86 build tools
     - Windows 10/11 SDK
     - CMake tools for Windows
   - Klicken Sie auf "Install"

3. **Nach der Installation:**
   - Starten Sie das Terminal neu
   - Führen Sie `.\build_installer.ps1` erneut aus

### Option 2: Visual Studio 2019

1. Download: https://visualstudio.microsoft.com/vs/older-downloads/
2. Installieren Sie "Desktop development with C++" Workload
3. Terminal neu starten

### Option 3: Visual Studio Build Tools (nur Compiler, ohne IDE)

1. Download: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
2. Installieren Sie "C++ build tools"
3. Terminal neu starten

## Prüfen ob Visual Studio installiert ist

Nach der Installation:
```powershell
# Prüfe installierte Visual Studio Versionen
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
```

Oder in PowerShell:
```powershell
Get-ChildItem "C:\Program Files\Microsoft Visual Studio\2022" -ErrorAction SilentlyContinue
```

## Alternative: MinGW-w64 (ohne Visual Studio)

Falls Sie kein Visual Studio installieren möchten, können Sie MinGW-w64 verwenden:

1. Installieren Sie MSYS2: https://www.msys2.org/
2. Installieren Sie MinGW-w64:
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-cmake
   ```
3. Verwenden Sie MinGW Makefiles Generator:
   ```powershell
   cmake -S . -B build -G "MinGW Makefiles" -DCAD_USE_QT=ON
   ```

## Nach erfolgreicher Visual Studio Installation

Führen Sie erneut aus:
```powershell
.\build_installer.ps1
```

Das Script findet jetzt automatisch Ihre Visual Studio Installation!

