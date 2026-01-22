@echo off
echo ============================================================
echo CADursor - Automatischer Build und Installer-Erstellung
echo ============================================================
echo.

REM Prüfe ob CMake vorhanden ist
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    REM Versuche typische Installationspfade
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set "PATH=%PATH%;C:\Program Files\CMake\bin"
        echo CMake gefunden in: C:\Program Files\CMake\bin
    ) else if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" (
        set "PATH=%PATH%;C:\Program Files (x86)\CMake\bin"
        echo CMake gefunden in: C:\Program Files (x86)\CMake\bin
    ) else (
        echo FEHLER: CMake nicht gefunden!
        echo.
        echo Bitte installieren Sie CMake von: https://cmake.org/download/
        echo Oder fügen Sie CMake zum PATH hinzu.
        echo.
        pause
        exit /b 1
    )
)

REM Prüfe ob NSIS vorhanden ist
set NSIS_PATH=C:\Program Files (x86)\NSIS\makensis.exe
if not exist "%NSIS_PATH%" (
    set NSIS_PATH=C:\Program Files\NSIS\makensis.exe
)
if not exist "%NSIS_PATH%" (
    echo WARNUNG: NSIS nicht gefunden!
    echo Bitte installieren Sie NSIS von https://nsis.sourceforge.io/Download
    echo.
    echo Der Installer kann nicht erstellt werden, aber die .exe wird gebaut.
    set BUILD_ONLY=1
)

echo Schritt 1: CMake konfigurieren...
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCAD_USE_QT=ON
if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: CMake Konfiguration fehlgeschlagen!
    pause
    exit /b 1
)

echo.
echo Schritt 2: Projekt kompilieren...
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo FEHLER: Kompilierung fehlgeschlagen!
    pause
    exit /b 1
)

echo.
echo Schritt 3: Prüfe ob cad_desktop.exe erstellt wurde...
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

echo Schritt 4: Installer erstellen...
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

