#pragma once

#include <string>

namespace cad {
namespace core {

/** 3D point for reference geometry */
struct Point3D {
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

/** 3D direction (normalized or not) */
struct Vector3D {
    double x{0.0};
    double y{0.0};
    double z{1.0};
};

/** Work plane: origin + normal; or reference to base plane (XY/YZ/XZ) + offset */
struct WorkPlane {
    std::string id;
    std::string name;
    Point3D origin{0, 0, 0};
    Vector3D normal{0, 0, 1};
    /** Base plane reference: "XY", "YZ", "XZ" or empty if custom */
    std::string base_plane;
    double offset{0.0};
    /** Fusion: "Standard", "Tangent" (tangent plane), "AsBuilt" (as-built). */
    std::string plane_type{"Standard"};
};

/** Work axis: point + direction; or reference to X/Y/Z axis */
struct WorkAxis {
    std::string id;
    std::string name;
    Point3D point{0, 0, 0};
    Vector3D direction{0, 0, 1};
    /** Base axis: "X", "Y", "Z" or empty if custom */
    std::string base_axis;
};

/** Work point (3D point) */
struct WorkPoint {
    std::string id;
    std::string name;
    Point3D point{0, 0, 0};
};

/** Coordinate system: origin + X and Y directions (Z = X × Y) */
struct CoordinateSystem {
    std::string id;
    std::string name;
    Point3D origin{0, 0, 0};
    Vector3D direction_x{1, 0, 0};
    Vector3D direction_y{0, 1, 0};
};

}  // namespace core
}  // namespace cad
