# Hydra CAD - Automatischer Build und Installer-Erstellung (PowerShell)
# Führen Sie aus mit: .\build_installer.ps1

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Hydra CAD - Automatischer Build und Installer-Erstellung" -ForegroundColor Cyan
Write-Host "Version 2.0.0 - Phase 20-22 Complete" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Prüfe CMake
$cmakeFound = $false
$cmakePaths = @(
    "C:\Program Files\CMake\bin\cmake.exe",
    "C:\Program Files (x86)\CMake\bin\cmake.exe"
)

if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $cmakeFound = $true
    Write-Host "CMake gefunden im PATH" -ForegroundColor Green
} else {
    foreach ($path in $cmakePaths) {
        if (Test-Path $path) {
            $cmakeDir = Split-Path $path -Parent
            $env:PATH = "$cmakeDir;$env:PATH"
            Write-Host "CMake gefunden in: $cmakeDir" -ForegroundColor Green
            $cmakeFound = $true
            break
        }
    }
}

if (-not $cmakeFound) {
    Write-Host "FEHLER: CMake nicht gefunden!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Bitte installieren Sie CMake von: https://cmake.org/download/" -ForegroundColor Yellow
    Write-Host "Oder fügen Sie CMake zum PATH hinzu." -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

# Prüfe NSIS
$nsisPaths = @(
    "C:\Program Files (x86)\NSIS\makensis.exe",
    "C:\Program Files\NSIS\makensis.exe"
)
$nsisPath = $null
foreach ($path in $nsisPaths) {
    if (Test-Path $path) {
        $nsisPath = $path
        break
    }
}

if (-not $nsisPath) {
    Write-Host "WARNUNG: NSIS nicht gefunden!" -ForegroundColor Yellow
    Write-Host "Bitte installieren Sie NSIS von https://nsis.sourceforge.io/Download" -ForegroundColor Yellow
    Write-Host "Der Installer kann nicht erstellt werden, aber die .exe wird gebaut." -ForegroundColor Yellow
    Write-Host ""
    $buildOnly = $true
} else {
    $buildOnly = $false
}

# Schritt 1: Build-System finden
Write-Host "Schritt 1: Suche nach Build-System..." -ForegroundColor Green

# Prüfe zuerst nach MinGW/GCC
$foundGenerator = $null
$useMinGW = $false

# Prüfe ob gcc verfügbar ist
if (Get-Command gcc -ErrorAction SilentlyContinue) {
    Write-Host "  GCC/MinGW gefunden!" -ForegroundColor Green
    $foundGenerator = "MinGW Makefiles"
    $useMinGW = $true
} else {
    # Prüfe typische MinGW-Pfade
    $mingwPaths = @(
        "C:\msys64\mingw64\bin\gcc.exe",
        "C:\mingw64\bin\gcc.exe",
        "C:\Program Files\mingw-w64\*\mingw64\bin\gcc.exe"
    )
    
    foreach ($path in $mingwPaths) {
        $resolved = Resolve-Path $path -ErrorAction SilentlyContinue
        if ($resolved) {
            $gccDir = Split-Path (Split-Path $resolved -Parent) -Parent
            $env:PATH = "$gccDir\bin;$env:PATH"
            Write-Host "  MinGW gefunden in: $gccDir" -ForegroundColor Green
            $foundGenerator = "MinGW Makefiles"
            $useMinGW = $true
            break
        }
    }
}

# Falls kein MinGW, suche nach Visual Studio
if (-not $foundGenerator) {
    Write-Host "  Suche nach Visual Studio..." -ForegroundColor Gray
    $generators = @(
        "Visual Studio 17 2022",
        "Visual Studio 16 2019",
        "Visual Studio 15 2017",
        "Visual Studio 14 2015"
    )

    foreach ($gen in $generators) {
        Write-Host "    Prüfe: $gen..." -ForegroundColor Gray
        $test = cmake -G $gen -S . -B build_test 2>&1 | Out-Null
        if ($LASTEXITCODE -eq 0) {
            $foundGenerator = $gen
            Remove-Item -Path "build_test" -Recurse -Force -ErrorAction SilentlyContinue
            break
        }
        Remove-Item -Path "build_test" -Recurse -Force -ErrorAction SilentlyContinue
    }
}

# Falls immer noch nichts gefunden, versuche Ninja
if (-not $foundGenerator) {
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        Write-Host "  Ninja gefunden!" -ForegroundColor Green
        $foundGenerator = "Ninja"
    }
}

if (-not $foundGenerator) {
    Write-Host "FEHLER: Kein Build-System gefunden!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Bitte installieren Sie eines der folgenden:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Option 1: MinGW-w64 (Empfohlen, ohne Visual Studio):" -ForegroundColor Cyan
    Write-Host "  - MSYS2: https://www.msys2.org/" -ForegroundColor Yellow
    Write-Host "  - Dann: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Option 2: Visual Studio:" -ForegroundColor Cyan
    Write-Host "  - Visual Studio 2022: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
    Write-Host "  - Workload: 'Desktop development with C++'" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

Write-Host "Build-System gefunden: $foundGenerator" -ForegroundColor Green
Write-Host ""

# Schritt 2: CMake konfigurieren
Write-Host "Schritt 2: CMake konfigurieren..." -ForegroundColor Green

if ($useMinGW) {
    # MinGW Makefiles verwenden
    $cmakeConfig = cmake -S . -B build -G "MinGW Makefiles" -DCAD_USE_QT=ON
} elseif ($foundGenerator -eq "Ninja") {
    # Ninja verwenden
    $cmakeConfig = cmake -S . -B build -G "Ninja" -DCAD_USE_QT=ON
} else {
    # Visual Studio verwenden
    $cmakeConfig = cmake -S . -B build -G $foundGenerator -A x64 -DCAD_USE_QT=ON
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "FEHLER: CMake Konfiguration fehlgeschlagen!" -ForegroundColor Red
    Write-Host "Fehlerdetails:" -ForegroundColor Yellow
    Write-Host $cmakeConfig -ForegroundColor Gray
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

Write-Host ""

# Schritt 3: Projekt kompilieren
Write-Host "Schritt 3: Projekt kompilieren..." -ForegroundColor Green

if ($useMinGW -or $foundGenerator -eq "Ninja") {
    # MinGW/Ninja: Kein --config Parameter
    cmake --build build
} else {
    # Visual Studio: --config Release
    cmake --build build --config Release
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "FEHLER: Kompilierung fehlgeschlagen!" -ForegroundColor Red
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

Write-Host ""

# Schritt 4: Prüfe ob .exe erstellt wurde
Write-Host "Schritt 4: Prüfe ob cad_desktop.exe erstellt wurde..." -ForegroundColor Green

# Prüfe verschiedene mögliche Pfade
$exePath = $null
if ($useMinGW -or $foundGenerator -eq "Ninja") {
    # MinGW/Ninja: Direkt im build-Verzeichnis
    $exePath = "build\cad_desktop.exe"
} else {
    # Visual Studio: In build\Release\
    $exePath = "build\Release\cad_desktop.exe"
}

if (-not (Test-Path $exePath)) {
    Write-Host "FEHLER: cad_desktop.exe wurde nicht erstellt!" -ForegroundColor Red
    Write-Host "Erwarteter Pfad: $exePath" -ForegroundColor Yellow
    Write-Host "Bitte prüfen Sie die Build-Ausgabe auf Fehler." -ForegroundColor Yellow
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

Write-Host "OK: cad_desktop.exe wurde erfolgreich erstellt!" -ForegroundColor Green
Write-Host "Pfad: $exePath" -ForegroundColor Gray
Write-Host ""

if ($buildOnly) {
    Write-Host "Die .exe finden Sie hier: $exePath" -ForegroundColor Cyan
    Write-Host "Sie können diese direkt ausführen." -ForegroundColor Cyan
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 0
}

# Schritt 5: Installer erstellen
Write-Host "Schritt 5: Installer erstellen..." -ForegroundColor Green

# Kopiere .exe nach Release-Verzeichnis für NSIS (falls MinGW/Ninja)
if ($useMinGW -or $foundGenerator -eq "Ninja") {
    $releaseDir = "build\Release"
    if (-not (Test-Path $releaseDir)) {
        New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
    }
    Copy-Item $exePath "$releaseDir\cad_desktop.exe" -Force
    Write-Host "  .exe nach build\Release kopiert für Installer" -ForegroundColor Gray
}

# Qt deployment (if available)
$windeployqt = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($windeployqt) {
    $releaseExe = "build\Release\cad_desktop.exe"
    if (Test-Path $releaseExe) {
        Write-Host "  Qt deployment mit windeployqt..." -ForegroundColor Gray
        & $windeployqt $releaseExe
        if (-not (Test-Path "build\Release\Qt6Gui.dll")) {
            Write-Host "FEHLER: Qt6Gui.dll wurde nicht gefunden. Installer waere defekt." -ForegroundColor Red
            Read-Host "Druecken Sie Enter zum Beenden"
            exit 1
        }
    } else {
        Write-Host "  WARNUNG: $releaseExe nicht gefunden für windeployqt" -ForegroundColor Yellow
    }
} else {
    Write-Host "  WARNUNG: windeployqt nicht gefunden; Qt-DLLs werden evtl. fehlen" -ForegroundColor Yellow
}

Push-Location installer
& $nsisPath hydracad.nsi
$installerSuccess = $LASTEXITCODE -eq 0
Pop-Location

if (-not $installerSuccess) {
    Write-Host "FEHLER: Installer-Erstellung fehlgeschlagen!" -ForegroundColor Red
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

if (Test-Path "installer\HydraCADSetup.exe") {
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host "ERFOLG!" -ForegroundColor Green
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Der Installer wurde erfolgreich erstellt:" -ForegroundColor Cyan
    Write-Host "installer\HydraCADSetup.exe" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Sie können diese Datei jetzt doppelklicken, um Hydra CAD zu installieren!" -ForegroundColor Green
    Write-Host ""
    
    # Frage ob Installer geöffnet werden soll
    $open = Read-Host "Möchten Sie den Installer jetzt öffnen? (j/n)"
    if ($open -eq "j" -or $open -eq "J" -or $open -eq "y" -or $open -eq "Y") {
        Start-Process "installer\HydraCADSetup.exe"
    }
} else {
    Write-Host "WARNUNG: HydraCADSetup.exe wurde nicht gefunden!" -ForegroundColor Yellow
    Write-Host "Bitte prüfen Sie die NSIS-Ausgabe auf Fehler." -ForegroundColor Yellow
}

Read-Host "Drücken Sie Enter zum Beenden"

