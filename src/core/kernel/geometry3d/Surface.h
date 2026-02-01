#pragma once

#include "math/Vector3.h"

namespace cad {
namespace kernel {
namespace geometry3d {

class Surface {
public:
    virtual ~Surface() = default;
    virtual math::Point3 pointAt(double u, double v) const = 0;
    virtual math::Vector3 normalAt(double u, double v) const = 0;
    virtual double uMin() const = 0;
    virtual double uMax() const = 0;
    virtual double vMin() const = 0;
    virtual double vMax() const = 0;
};

}  // namespace geometry3d
}  // namespace kernel
}  // namespace cad
