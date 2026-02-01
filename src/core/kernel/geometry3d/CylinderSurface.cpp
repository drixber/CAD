#include "geometry3d/CylinderSurface.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry3d {

static void makeAxes(const math::Vector3& axis, math::Vector3& uAxis, math::Vector3& vAxis) {
    math::Vector3 n = axis.normalized();
    if (std::abs(n.x) < 0.9)
        uAxis = math::Vector3(1, 0, 0).cross(n).normalized();
    else
        uAxis = math::Vector3(0, 1, 0).cross(n).normalized();
    vAxis = n.cross(uAxis).normalized();
}

CylinderSurface::CylinderSurface(const math::Point3& origin, const math::Vector3& axis, double radius)
    : origin_(origin), axis_(axis.normalized()), radius_(radius) {
    makeAxes(axis_, uAxis_, vAxis_);
}

math::Point3 CylinderSurface::pointAt(double u, double v) const {
    const math::Vector3 radial = std::cos(u) * uAxis_ + std::sin(u) * vAxis_;
    return math::Point3(
        origin_.x + radius_ * radial.x + v * axis_.x,
        origin_.y + radius_ * radial.y + v * axis_.y,
        origin_.z + radius_ * radial.z + v * axis_.z
    );
}

math::Vector3 CylinderSurface::normalAt(double u, double /*v*/) const {
    return (std::cos(u) * uAxis_ + std::sin(u) * vAxis_).normalized();
}

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
