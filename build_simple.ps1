# CADursor - Super einfacher Build
# Einfach ausführen: .\build_simple.ps1

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "CADursor - Einfacher Build" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Prüfe MSYS2
$msys2Path = "C:\msys64\ucrt64\bin"
if (Test-Path $msys2Path) {
    Write-Host "MSYS2 gefunden!" -ForegroundColor Green
    $env:PATH = "$msys2Path;$env:PATH"
} else {
    Write-Host "FEHLER: MSYS2 nicht gefunden in C:\msys64" -ForegroundColor Red
    Write-Host ""
    Write-Host "Bitte installieren Sie MSYS2:" -ForegroundColor Yellow
    Write-Host "1. Download: https://www.msys2.org/" -ForegroundColor Yellow
    Write-Host "2. Installer ausführen" -ForegroundColor Yellow
    Write-Host "3. MSYS2 UCRT64 Terminal öffnen" -ForegroundColor Yellow
    Write-Host "4. Ausführen: pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-qt5-base" -ForegroundColor Yellow
    Write-Host "5. Dieses Script erneut ausführen" -ForegroundColor Yellow
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

# Prüfe GCC
Write-Host "Prüfe GCC..." -ForegroundColor Gray
$gcc = & "$msys2Path\gcc.exe" --version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "GCC nicht gefunden!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Bitte in MSYS2 UCRT64 Terminal ausführen:" -ForegroundColor Yellow
    Write-Host "  pacman -S mingw-w64-ucrt-x86_64-gcc" -ForegroundColor Cyan
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}
Write-Host "GCC OK" -ForegroundColor Green

# Prüfe CMake
Write-Host "Prüfe CMake..." -ForegroundColor Gray
$cmake = & "$msys2Path\cmake.exe" --version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake nicht gefunden!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Bitte in MSYS2 UCRT64 Terminal ausführen:" -ForegroundColor Yellow
    Write-Host "  pacman -S mingw-w64-ucrt-x86_64-cmake" -ForegroundColor Cyan
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}
Write-Host "CMake OK" -ForegroundColor Green
Write-Host ""

# Build
Write-Host "Starte Build..." -ForegroundColor Green
Write-Host ""

# Altes Build-Verzeichnis löschen falls vorhanden
if (Test-Path "build") {
    Write-Host "Lösche altes Build-Verzeichnis..." -ForegroundColor Gray
    Remove-Item -Path "build" -Recurse -Force -ErrorAction SilentlyContinue
}

# Prüfe make
$makePath = "$msys2Path\make.exe"
if (-not (Test-Path $makePath)) {
    Write-Host "make.exe nicht gefunden!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Bitte in MSYS2 UCRT64 Terminal ausführen:" -ForegroundColor Yellow
    Write-Host "  pacman -S make" -ForegroundColor Cyan
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

# CMake konfigurieren
Write-Host "[1/3] CMake konfigurieren..." -ForegroundColor Yellow
& "$msys2Path\cmake.exe" -S . -B build `
    -G "MinGW Makefiles" `
    -DCMAKE_MAKE_PROGRAM="$makePath" `
    -DCMAKE_C_COMPILER="$msys2Path\gcc.exe" `
    -DCMAKE_CXX_COMPILER="$msys2Path\g++.exe" `
    -DCAD_USE_QT=ON
if ($LASTEXITCODE -ne 0) {
    Write-Host "FEHLER bei CMake Konfiguration!" -ForegroundColor Red
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

# Kompilieren
Write-Host "[2/3] Kompilieren (das kann etwas dauern)..." -ForegroundColor Yellow
& "$msys2Path\cmake.exe" --build build
if ($LASTEXITCODE -ne 0) {
    Write-Host "FEHLER beim Kompilieren!" -ForegroundColor Red
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

# Prüfe Ergebnis
Write-Host "[3/3] Prüfe Ergebnis..." -ForegroundColor Yellow
if (Test-Path "build\cad_desktop.exe") {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "ERFOLG!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Die .exe wurde erstellt:" -ForegroundColor Cyan
    Write-Host "  build\cad_desktop.exe" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Sie können die Datei jetzt direkt ausführen!" -ForegroundColor Green
    Write-Host ""
    
    $run = Read-Host "Jetzt ausführen? (j/n)"
    if ($run -eq "j" -or $run -eq "J" -or $run -eq "y" -or $run -eq "Y") {
        Start-Process "build\cad_desktop.exe"
    }
} else {
    Write-Host "FEHLER: cad_desktop.exe wurde nicht erstellt!" -ForegroundColor Red
}

Read-Host "Drücken Sie Enter zum Beenden"

