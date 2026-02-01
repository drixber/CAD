#pragma once

#include "geometry3d/Surface.h"

namespace cad {
namespace kernel {
namespace geometry3d {

class PlaneSurface : public Surface {
public:
    PlaneSurface(const math::Point3& origin, const math::Vector3& uAxis, const math::Vector3& vAxis);
    math::Point3 pointAt(double u, double v) const override;
    math::Vector3 normalAt(double u, double v) const override;
    double uMin() const override { return uMin_; }
    double uMax() const override { return uMax_; }
    double vMin() const override { return vMin_; }
    double vMax() const override { return vMax_; }
private:
    math::Point3 origin_;
    math::Vector3 uAxis_;
    math::Vector3 vAxis_;
    double uMin_{0.0}, uMax_{1.0}, vMin_{0.0}, vMax_{1.0};
};

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
