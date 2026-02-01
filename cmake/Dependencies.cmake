include(ExternalProject)
include(${CMAKE_CURRENT_LIST_DIR}/DependenciesConfig.cmake)

if(CAD_DEPENDENCY_PROVIDER STREQUAL "vcpkg")
    include(${CMAKE_CURRENT_LIST_DIR}/Vcpkg.cmake)
endif()
