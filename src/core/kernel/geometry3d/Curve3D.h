#pragma once

#include "math/Vector3.h"

namespace cad {
namespace kernel {
namespace geometry3d {

class Curve3D {
public:
    virtual ~Curve3D() = default;
    virtual math::Point3 pointAt(double t) const = 0;
    virtual math::Vector3 tangentAt(double t) const = 0;
    virtual double tMin() const = 0;
    virtual double tMax() const = 0;
};

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
