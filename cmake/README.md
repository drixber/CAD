# CMake Tooling

This directory hosts build helper files for Windows-only development.

## Superbuild
- Enable with `-DCAD_SUPERBUILD=ON`.
- Superbuild prepares third-party dependencies and will be expanded after
  toolchain validation.
 - Default dependency provider: `vcpkg` (configure via `CAD_DEPENDENCY_PROVIDER`).
 - Dependency versions/branches are configured in `cmake/DependenciesConfig.cmake`.

## Vcpkg
- Packages list configured in `cmake/Vcpkg.cmake`.
- Customize triplet with `-DCAD_VCPKG_TRIPLET=x64-windows`.

## Toolchains
- `toolchains/windows-msvc.cmake` for MSVC/Clang-CL builds.
