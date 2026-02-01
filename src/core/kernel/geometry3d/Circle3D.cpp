#include "geometry3d/Circle3D.h"
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

Circle3D::Circle3D(const math::Point3& center, double radius, const math::Vector3& axis)
    : center_(center), radius_(radius), axis_(axis.normalized()) {
    makeAxes(axis_, uAxis_, vAxis_);
}

math::Point3 Circle3D::pointAt(double t) const {
    return math::Point3(
        center_.x + radius_ * (std::cos(t) * uAxis_.x + std::sin(t) * vAxis_.x),
        center_.y + radius_ * (std::cos(t) * uAxis_.y + std::sin(t) * vAxis_.y),
        center_.z + radius_ * (std::cos(t) * uAxis_.z + std::sin(t) * vAxis_.z)
    );
}

math::Vector3 Circle3D::tangentAt(double t) const {
    return math::Vector3(
        -std::sin(t) * uAxis_.x + std::cos(t) * vAxis_.x,
        -std::sin(t) * uAxis_.y + std::cos(t) * vAxis_.y,
        -std::sin(t) * uAxis_.z + std::cos(t) * vAxis_.z
    ).normalized();
}

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
