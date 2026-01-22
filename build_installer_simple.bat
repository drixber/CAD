@echo off
REM Einfache Version ohne komplexe PATH-Manipulation
echo ============================================================
echo CADursor - Automatischer Build und Installer-Erstellung
echo ============================================================
echo.

REM Prüfe ob CMake vorhanden ist
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: CMake nicht gefunden!
    echo.
    echo Bitte installieren Sie CMake von: https://cmake.org/download/
    echo WICHTIG: Aktivieren Sie "Add CMake to system PATH" bei der Installation!
    echo.
    echo Oder starten Sie cmd.exe (nicht PowerShell) und versuchen Sie es erneut.
    echo.
    pause
    exit /b 1
)

REM Prüfe ob NSIS vorhanden ist
set "NSIS_PATH=C:\Program Files (x86)\NSIS\makensis.exe"
if not exist "%NSIS_PATH%" (
    set "NSIS_PATH=C:\Program Files\NSIS\makensis.exe"
)
if not exist "%NSIS_PATH%" (
    echo WARNUNG: NSIS nicht gefunden!
    echo Bitte installieren Sie NSIS von https://nsis.sourceforge.io/Download
    echo.
    echo Der Installer kann nicht erstellt werden, aber die .exe wird gebaut.
    set BUILD_ONLY=1
)

echo Schritt 1: Suche nach Visual Studio...
cmake -G "Visual Studio 17 2022" -S . -B build_test >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS_GENERATOR=Visual Studio 17 2022"
    rmdir /s /q build_test 2>nul
    goto :found_vs
)
cmake -G "Visual Studio 16 2019" -S . -B build_test >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS_GENERATOR=Visual Studio 16 2019"
    rmdir /s /q build_test 2>nul
    goto :found_vs
)
cmake -G "Visual Studio 15 2017" -S . -B build_test >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS_GENERATOR=Visual Studio 15 2017"
    rmdir /s /q build_test 2>nul
    goto :found_vs
)

echo FEHLER: Keine Visual Studio Installation gefunden!
echo.
echo Bitte installieren Sie Visual Studio:
echo   - Visual Studio 2022: https://visualstudio.microsoft.com/downloads/
echo   - Oder Visual Studio 2019/2017
echo.
echo WICHTIG: Installieren Sie die "Desktop development with C++" Workload!
echo.
pause
exit /b 1

:found_vs
echo Visual Studio gefunden: %VS_GENERATOR%
echo.

echo Schritt 2: CMake konfigurieren...
cmake -S . -B build -G "%VS_GENERATOR%" -A x64 -DCAD_USE_QT=ON
if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: CMake Konfiguration fehlgeschlagen!
    pause
    exit /b 1
)

echo.
echo Schritt 3: Projekt kompilieren...
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: Kompilierung fehlgeschlagen!
    pause
    exit /b 1
)

echo.
echo Schritt 4: Prüfe ob cad_desktop.exe erstellt wurde...
if not exist "build\Release\cad_desktop.exe" (
    echo FEHLER: cad_desktop.exe wurde nicht erstellt!
    echo Bitte prüfen Sie die Build-Ausgabe auf Fehler.
    pause
    exit /b 1
)

echo OK: cad_desktop.exe wurde erfolgreich erstellt!
echo.

if "%BUILD_ONLY%"=="1" (
    echo Die .exe finden Sie hier: build\Release\cad_desktop.exe
    echo Sie können diese direkt ausführen.
    pause
    exit /b 0
)

echo Schritt 5: Installer erstellen...
cd installer
"%NSIS_PATH%" cadursor.nsi
cd ..

if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: Installer-Erstellung fehlgeschlagen!
    pause
    exit /b 1
)

if exist "installer\CADursorSetup.exe" (
    echo.
    echo ============================================================
    echo ERFOLG!
    echo ============================================================
    echo.
    echo Der Installer wurde erfolgreich erstellt:
    echo installer\CADursorSetup.exe
    echo.
    echo Sie können diese Datei jetzt doppelklicken, um CADursor zu installieren!
    echo.
) else (
    echo WARNUNG: CADursorSetup.exe wurde nicht gefunden!
    echo Bitte prüfen Sie die NSIS-Ausgabe auf Fehler.
)

pause

