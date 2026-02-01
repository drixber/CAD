#include "geometry3d/Line3D.h"

namespace cad {
namespace kernel {
namespace geometry3d {

Line3D::Line3D(const math::Point3& start, const math::Point3& end)
    : start_(start), end_(end) {}

math::Point3 Line3D::pointAt(double t) const {
    return math::Point3(
        start_.x + t * (end_.x - start_.x),
        start_.y + t * (end_.y - start_.y),
        start_.z + t * (end_.z - start_.z)
    );
}

math::Vector3 Line3D::tangentAt(double /*t*/) const {
    math::Vector3 d(end_.x - start_.x, end_.y - start_.y, end_.z - start_.z);
    const double len = d.length();
    if (len <= 0.0) return math::Vector3(1, 0, 0);
    return d.normalized();
}

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
