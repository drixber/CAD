#pragma once

#include "geometry3d/Curve3D.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry3d {

class Circle3D : public Curve3D {
public:
    Circle3D(const math::Point3& center, double radius, const math::Vector3& axis);
    math::Point3 pointAt(double t) const override;
    math::Vector3 tangentAt(double t) const override;
    double tMin() const override { return 0.0; }
    double tMax() const override { return 2.0 * 3.14159265358979323846; }
private:
    math::Point3 center_;
    double radius_{0.0};
    math::Vector3 axis_;
    math::Vector3 uAxis_;
    math::Vector3 vAxis_;
};

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
