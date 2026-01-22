# Build ohne Visual Studio

## Option 1: MinGW-w64 (Empfohlen)

### Installation mit MSYS2

1. **MSYS2 installieren:**
   - Download: https://www.msys2.org/
   - Installer ausführen
   - Terminal öffnen (MSYS2 UCRT64)

2. **MinGW-w64 und CMake installieren:**
   ```bash
   pacman -Syu
   pacman -S mingw-w64-ucrt-x86_64-gcc
   pacman -S mingw-w64-ucrt-x86_64-cmake
   pacman -S mingw-w64-ucrt-x86_64-qt5-base
   pacman -S mingw-w64-ucrt-x86_64-qt5-tools
   ```

3. **PATH hinzufügen:**
   - Fügen Sie `C:\msys64\ucrt64\bin` zum System-PATH hinzu
   - Oder starten Sie PowerShell aus MSYS2 Terminal

4. **Build ausführen:**
   ```powershell
   .\build_installer.ps1
   ```

### Installation mit Standalone MinGW

1. **Download:**
   - https://www.mingw-w64.org/downloads/
   - Oder: https://sourceforge.net/projects/mingw-w64/

2. **Installation:**
   - Installieren Sie nach `C:\mingw64`
   - Fügen Sie `C:\mingw64\bin` zum PATH hinzu

3. **CMake installieren:**
   - Download: https://cmake.org/download/
   - Aktivieren Sie "Add CMake to system PATH"

4. **Build ausführen:**
   ```powershell
   .\build_installer.ps1
   ```

## Option 2: Clang (LLVM)

1. **LLVM installieren:**
   - Download: https://llvm.org/builds/
   - Oder: https://github.com/llvm/llvm-project/releases

2. **PATH hinzufügen:**
   - Fügen Sie `C:\Program Files\LLVM\bin` zum PATH hinzu

3. **CMake konfigurieren:**
   ```powershell
   cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCAD_USE_QT=ON
   cmake --build build
   ```

## Option 3: WSL (Windows Subsystem for Linux)

Falls Sie WSL haben:

```bash
# In WSL
sudo apt update
sudo apt install build-essential cmake qt5-default
cd /mnt/c/Users/Darius\ Herrmann/cad/CAD
cmake -S . -B build -DCAD_USE_QT=ON
cmake --build build
```

## Manueller Build (ohne Script)

```powershell
# Mit MinGW
cmake -S . -B build -G "MinGW Makefiles" -DCAD_USE_QT=ON
cmake --build build

# Die .exe finden Sie in: build\cad_desktop.exe
```

## Qt für MinGW

Falls Qt nicht gefunden wird:

1. **Qt installieren:**
   - Download: https://www.qt.io/download-open-source
   - Wählen Sie MinGW-Version

2. **CMake Qt-Pfad setzen:**
   ```powershell
   cmake -S . -B build -G "MinGW Makefiles" `
     -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\mingw_64" `
     -DCAD_USE_QT=ON
   ```

## Troubleshooting

### "gcc not found"
- Stellen Sie sicher, dass MinGW im PATH ist
- Prüfen Sie mit: `gcc --version`

### "Qt not found"
- Installieren Sie Qt für MinGW
- Setzen Sie `CMAKE_PREFIX_PATH` auf Qt-Installationsverzeichnis

### "CMake Error: No CMAKE_C_COMPILER could be found"
- Installieren Sie MinGW-w64
- Fügen Sie MinGW\bin zum PATH hinzu
- Terminal neu starten

## Vorteile von MinGW

- ✅ Keine Visual Studio Installation nötig
- ✅ Kleinere Installationsgröße
- ✅ Open Source Compiler
- ✅ Kompatibel mit Linux-Tools

