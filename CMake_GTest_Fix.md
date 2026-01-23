# CMake GTest Fix

## 🔧 Problem

Alle Test-Targets konnten `GTest::gtest` nicht finden, was zu CMake-Fehlern führte.

## ✅ Lösung

GTest wird jetzt automatisch mit FetchContent heruntergeladen, wenn es nicht gefunden wird.

### Änderungen in `tests/CMakeLists.txt`:

```cmake
if(CAD_BUILD_TESTS)
    # Find or fetch GTest
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
    # ... rest of tests
endif()
```

## 📋 Status

- ✅ GTest wird automatisch heruntergeladen, wenn nicht gefunden
- ✅ Funktioniert auf allen Plattformen (Windows, Linux, macOS)
- ✅ Keine manuelle Installation nötig

---

**Datum**: 23. Januar 2026
