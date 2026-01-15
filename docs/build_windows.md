# Windows Build Guide

## Toolchain

- Windows 11 or Windows 10 (x64)
- Visual Studio 2022 (MSVC) or Clang-CL
- CMake 3.26+
- vcpkg (optional, if not using system packages)

## Configure (Native)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCAD_USE_QT=ON -DCAD_USE_FREECAD=OFF
```

## Configure (Superbuild + vcpkg)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCAD_SUPERBUILD=ON -DCAD_DEPENDENCY_PROVIDER=vcpkg `
  -DCAD_USE_QT=ON -DCAD_BUILD_PYTHON=ON
```

## Build

```powershell
cmake --build build --config Release
```

## Run (Qt UI)

```powershell
.\build\Release\cad_desktop.exe
```

## FreeCAD Integration

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCAD_USE_FREECAD=ON -DCAD_FREECAD_ROOT="third_party/FreeCAD"
```
