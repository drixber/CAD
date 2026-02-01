#pragma once

#include "geometry3d/Surface.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry3d {

class SphereSurface : public Surface {
public:
    SphereSurface(const math::Point3& center, double radius);
    math::Point3 pointAt(double u, double v) const override;
    math::Vector3 normalAt(double u, double v) const override;
    double uMin() const override { return 0.0; }
    double uMax() const override { return 2.0 * 3.14159265358979323846; }
    double vMin() const override { return 0.0; }
    double vMax() const override { return 3.14159265358979323846; }
private:
    math::Point3 center_;
    double radius_{0.0};
};

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
