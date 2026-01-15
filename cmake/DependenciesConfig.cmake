set(CAD_FREECAD_GIT_URL "https://github.com/FreeCAD/FreeCAD.git" CACHE STRING "FreeCAD git URL")
# Use the latest upstream development branch for feature parity.
set(CAD_FREECAD_GIT_TAG "master" CACHE STRING "FreeCAD git tag/branch")

set(CAD_VCPKG_TRIPLET "x64-windows" CACHE STRING "Vcpkg triplet")
set(CAD_VCPKG_PACKAGES
    "qtbase"
    "qttools"
    "qtdeclarative"
    "opencascade"
    "coin3d"
    "boost"
    "python3"
    "tbb"
    "eigen3"
    "zlib"
    "freetype"
    "libpng"
    "libjpeg-turbo"
    "harfbuzz"
    "libxml2"
    "openssl"
    CACHE STRING "Vcpkg packages to install"
)
