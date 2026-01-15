include(ExternalProject)

set(CAD_VCPKG_GIT_URL "https://github.com/microsoft/vcpkg.git")
set(CAD_VCPKG_GIT_TAG "master")

set(CAD_VCPKG_PREFIX ${CAD_SUPERBUILD_PREFIX}/vcpkg)
set(CAD_VCPKG_ROOT ${CAD_VCPKG_PREFIX}/src/Vcpkg)

if(WIN32)
    set(CAD_VCPKG_BOOTSTRAP ${CAD_VCPKG_ROOT}/bootstrap-vcpkg.bat)
else()
    set(CAD_VCPKG_BOOTSTRAP ${CAD_VCPKG_ROOT}/bootstrap-vcpkg.sh)
endif()

ExternalProject_Add(Vcpkg
    PREFIX ${CAD_VCPKG_PREFIX}
    GIT_REPOSITORY ${CAD_VCPKG_GIT_URL}
    GIT_TAG ${CAD_VCPKG_GIT_TAG}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CAD_VCPKG_BOOTSTRAP} -disableMetrics
    INSTALL_COMMAND ""
)

ExternalProject_Add(VcpkgDeps
    PREFIX ${CAD_VCPKG_PREFIX}/deps
    DEPENDS Vcpkg
    SOURCE_DIR ${CAD_VCPKG_ROOT}
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CAD_VCPKG_ROOT}/vcpkg install ${CAD_VCPKG_PACKAGES} --triplet ${CAD_VCPKG_TRIPLET}
)
