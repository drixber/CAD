cmake_minimum_required(VERSION 3.26)

project(InventorStyleCAD VERSION 2.0.0 LANGUAGES CXX)

set(PROJECT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(PROJECT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(PROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CAD_BUILD_TESTS "Build test targets" OFF)
option(CAD_BUILD_EXAMPLES "Build example projects" OFF)
option(CAD_SUPERBUILD "Enable superbuild for dependencies" OFF)
option(CAD_USE_QT "Build Qt UI layer" OFF)
option(CAD_USE_FREECAD "Enable FreeCAD integration" OFF)
option(CAD_BUILD_PYTHON "Build Python bindings" OFF)
set(CAD_DEPENDENCY_PROVIDER "vcpkg" CACHE STRING "Dependency provider (vcpkg/none)")
set_property(CACHE CAD_DEPENDENCY_PROVIDER PROPERTY STRINGS vcpkg none)
set(CAD_FREECAD_ROOT "" CACHE PATH "Path to FreeCAD source root")

if(CAD_SUPERBUILD)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/Superbuild.cmake)
    return()
endif()

add_subdirectory(src)

if(CAD_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(CAD_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(CAD_USE_QT)
    add_executable(cad_desktop
        src/main.cpp
    )
    target_link_libraries(cad_desktop PRIVATE cad_app cad_ui)
    
    # macOS App Bundle
    if(APPLE)
        set_target_properties(cad_desktop PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME "Hydra CAD"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.hydracad.app"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        )
    endif()
    
    # Linux AppImage deployment
    if(UNIX AND NOT APPLE)
        find_package(Qt6 QUIET COMPONENTS Core)
        if(Qt6_FOUND)
            include(GNUInstallDirs)
            install(TARGETS cad_desktop
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
        endif()
    endif()
endif()

if(CAD_BUILD_PYTHON)
    add_subdirectory(python)
endif()
