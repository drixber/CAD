#include "geometry3d/SphereSurface.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry3d {

SphereSurface::SphereSurface(const math::Point3& center, double radius)
    : center_(center), radius_(radius) {}

math::Point3 SphereSurface::pointAt(double u, double v) const {
    const double sinV = std::sin(v);
    return math::Point3(
        center_.x + radius_ * std::cos(u) * sinV,
        center_.y + radius_ * std::sin(u) * sinV,
        center_.z + radius_ * std::cos(v)
    );
}

math::Vector3 SphereSurface::normalAt(double u, double v) const {
    return math::Vector3(
        std::cos(u) * std::sin(v),
        std::sin(u) * std::sin(v),
        std::cos(v)
    ).normalized();
}

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
