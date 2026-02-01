# Eigenes CAD-Kern-Modul (Option 2)
# Aktivierung: -DCAD_USE_EIGENER_KERN=ON
# Target: cad_eigen_kernel (src/kernel verwendet bereits cad_kernel)

cmake_minimum_required(VERSION 3.16)

add_library(cad_eigen_kernel STATIC
    math/Vector2.cpp
    math/Vector3.cpp
    math/Matrix3.cpp
    math/Transform3.cpp
    math/Tolerance.cpp
    geometry2d/Curve2D.cpp
    geometry2d/Line2D.cpp
    geometry2d/Circle2D.cpp
    geometry2d/Arc2D.cpp
    geometry2d/Spline2D.cpp
    geometry2d/Wire2D.cpp
    topology/Types.cpp
    topology/Vertex.cpp
    topology/Edge.cpp
    topology/Wire.cpp
    topology/Loop.cpp
    topology/Face.cpp
    topology/Shell.cpp
    topology/Solid.cpp
    topology/Shape.cpp
    geometry3d/Curve3D.cpp
    geometry3d/Line3D.cpp
    geometry3d/Circle3D.cpp
    geometry3d/Surface.cpp
    geometry3d/PlaneSurface.cpp
    geometry3d/CylinderSurface.cpp
    geometry3d/SphereSurface.cpp
    builder/WireBuilder.cpp
    builder/FaceBuilder.cpp
    builder/SolidBuilder.cpp
    boolean/FaceFaceIntersection.cpp
    boolean/TrimEdgesOnFace.cpp
    boolean/BooleanOps.cpp
    fillet/FilletOps.cpp
    fillet/ChamferOps.cpp
    advanced/ShellOps.cpp
    advanced/DraftOps.cpp
    advanced/LoftOps.cpp
    advanced/SweepOps.cpp
    io/MeshGenerator.cpp
    io/StlWriter.cpp
    io/StlReader.cpp
    KernelBridge.cpp
)

target_include_directories(cad_eigen_kernel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(cad_eigen_kernel PUBLIC cxx_std_17)
