#include "geometry3d/PlaneSurface.h"

namespace cad {
namespace kernel {
namespace geometry3d {

PlaneSurface::PlaneSurface(const math::Point3& origin, const math::Vector3& uAxis, const math::Vector3& vAxis)
    : origin_(origin), uAxis_(uAxis), vAxis_(vAxis) {}

math::Point3 PlaneSurface::pointAt(double u, double v) const {
    return math::Point3(
        origin_.x + u * uAxis_.x + v * vAxis_.x,
        origin_.y + u * uAxis_.y + v * vAxis_.y,
        origin_.z + u * uAxis_.z + v * vAxis_.z
    );
}

math::Vector3 PlaneSurface::normalAt(double /*u*/, double /*v*/) const {
    return uAxis_.cross(vAxis_).normalized();
}

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
