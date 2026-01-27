add_library(cad_core
    Modeler/Modeler.cpp
    FreeCAD/FreeCADAdapter.cpp
    FreeCAD/TechDrawBridge.cpp
    logging/Logger.cpp
    crash/CrashReporter.cpp
    undo/UndoStack.cpp
    perf/PerformanceMonitor.cpp
    perf/PerfSpan.cpp
    assembly/AssemblyManager.cpp
    analysis/InterferenceChecker.cpp
    geometry/OCCTIntegration.cpp
    updates/UpdateChecker.cpp
)

target_include_directories(cad_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

if(CAD_USE_FREECAD)
    target_compile_definitions(cad_core PUBLIC CAD_USE_FREECAD=1)
    if(CAD_FREECAD_ROOT)
        target_include_directories(cad_core
            PUBLIC
                ${CAD_FREECAD_ROOT}/src
        )
    endif()
endif()

option(CAD_USE_OCCT "Enable OpenCASCADE Technology integration" OFF)
if(CAD_USE_OCCT)
    find_package(OpenCASCADE QUIET)
    if(OpenCASCADE_FOUND)
        target_link_libraries(cad_core PUBLIC ${OpenCASCADE_LIBRARIES})
        target_include_directories(cad_core PUBLIC ${OpenCASCADE_INCLUDE_DIR})
        target_compile_definitions(cad_core PUBLIC CAD_USE_OCCT=1)
    else()
        message(WARNING "OpenCASCADE not found. Install via vcpkg: vcpkg install opencascade")
    endif()
endif()
