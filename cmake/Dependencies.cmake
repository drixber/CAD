include(ExternalProject)
include(${CMAKE_CURRENT_LIST_DIR}/DependenciesConfig.cmake)

if(CAD_DEPENDENCY_PROVIDER STREQUAL "vcpkg")
    include(${CMAKE_CURRENT_LIST_DIR}/Vcpkg.cmake)
endif()

ExternalProject_Add(FreeCAD
    PREFIX ${CAD_SUPERBUILD_PREFIX}/freecad
    GIT_REPOSITORY ${CAD_FREECAD_GIT_URL}
    GIT_TAG ${CAD_FREECAD_GIT_TAG}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
