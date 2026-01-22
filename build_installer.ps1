# CADursor - Automatischer Build und Installer-Erstellung (PowerShell)
# Führen Sie aus mit: .\build_installer.ps1

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "CADursor - Automatischer Build und Installer-Erstellung" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Prüfe CMake
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "FEHLER: CMake nicht gefunden!" -ForegroundColor Red
    Write-Host "Bitte installieren Sie CMake von https://cmake.org/download/" -ForegroundColor Yellow
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

# Schritt 1: CMake konfigurieren
Write-Host "Schritt 1: CMake konfigurieren..." -ForegroundColor Green
$cmakeConfig = cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCAD_USE_QT=ON
if ($LASTEXITCODE -ne 0) {
    Write-Host "FEHLER: CMake Konfiguration fehlgeschlagen!" -ForegroundColor Red
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

Write-Host ""

# Schritt 2: Projekt kompilieren
Write-Host "Schritt 2: Projekt kompilieren..." -ForegroundColor Green
cmake --build build --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Host "FEHLER: Kompilierung fehlgeschlagen!" -ForegroundColor Red
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

Write-Host ""

# Schritt 3: Prüfe ob .exe erstellt wurde
Write-Host "Schritt 3: Prüfe ob cad_desktop.exe erstellt wurde..." -ForegroundColor Green
if (-not (Test-Path "build\Release\cad_desktop.exe")) {
    Write-Host "FEHLER: cad_desktop.exe wurde nicht erstellt!" -ForegroundColor Red
    Write-Host "Bitte prüfen Sie die Build-Ausgabe auf Fehler." -ForegroundColor Yellow
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

Write-Host "OK: cad_desktop.exe wurde erfolgreich erstellt!" -ForegroundColor Green
Write-Host ""

if ($buildOnly) {
    Write-Host "Die .exe finden Sie hier: build\Release\cad_desktop.exe" -ForegroundColor Cyan
    Write-Host "Sie können diese direkt ausführen." -ForegroundColor Cyan
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 0
}

# Schritt 4: Installer erstellen
Write-Host "Schritt 4: Installer erstellen..." -ForegroundColor Green
Push-Location installer
& $nsisPath cadursor.nsi
$installerSuccess = $LASTEXITCODE -eq 0
Pop-Location

if (-not $installerSuccess) {
    Write-Host "FEHLER: Installer-Erstellung fehlgeschlagen!" -ForegroundColor Red
    Read-Host "Drücken Sie Enter zum Beenden"
    exit 1
}

if (Test-Path "installer\CADursorSetup.exe") {
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host "ERFOLG!" -ForegroundColor Green
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Der Installer wurde erfolgreich erstellt:" -ForegroundColor Cyan
    Write-Host "installer\CADursorSetup.exe" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Sie können diese Datei jetzt doppelklicken, um CADursor zu installieren!" -ForegroundColor Green
    Write-Host ""
    
    # Frage ob Installer geöffnet werden soll
    $open = Read-Host "Möchten Sie den Installer jetzt öffnen? (j/n)"
    if ($open -eq "j" -or $open -eq "J" -or $open -eq "y" -or $open -eq "Y") {
        Start-Process "installer\CADursorSetup.exe"
    }
} else {
    Write-Host "WARNUNG: CADursorSetup.exe wurde nicht gefunden!" -ForegroundColor Yellow
    Write-Host "Bitte prüfen Sie die NSIS-Ausgabe auf Fehler." -ForegroundColor Yellow
}

Read-Host "Drücken Sie Enter zum Beenden"

