include(ExternalProject)

set(CAD_SUPERBUILD_PREFIX ${CMAKE_BINARY_DIR}/_deps)
set(CAD_SUPERBUILD_INSTALL ${CMAKE_BINARY_DIR}/_install)

message(STATUS "Superbuild enabled")
message(STATUS "Prefix: ${CAD_SUPERBUILD_PREFIX}")
message(STATUS "Install: ${CAD_SUPERBUILD_INSTALL}")

include(${CMAKE_CURRENT_LIST_DIR}/Dependencies.cmake)
