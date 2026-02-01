#pragma once

#include "geometry3d/Curve3D.h"

namespace cad {
namespace kernel {
namespace geometry3d {

class Line3D : public Curve3D {
public:
    Line3D(const math::Point3& start, const math::Point3& end);
    math::Point3 pointAt(double t) const override;
    math::Vector3 tangentAt(double t) const override;
    double tMin() const override { return 0.0; }
    double tMax() const override { return 1.0; }
private:
    math::Point3 start_;
    math::Point3 end_;
};

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
