#pragma once

#include "geometry3d/Surface.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry3d {

class CylinderSurface : public Surface {
public:
    CylinderSurface(const math::Point3& origin, const math::Vector3& axis, double radius);
    math::Point3 pointAt(double u, double v) const override;
    math::Vector3 normalAt(double u, double v) const override;
    double uMin() const override { return 0.0; }
    double uMax() const override { return 2.0 * 3.14159265358979323846; }
    double vMin() const override { return 0.0; }
    double vMax() const override { return 1.0; }
    void setVMax(double v) { vMax_ = v; }
private:
    math::Point3 origin_;
    math::Vector3 axis_;
    double radius_{0.0};
    math::Vector3 uAxis_, vAxis_;
    double vMax_{1.0};
};

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
