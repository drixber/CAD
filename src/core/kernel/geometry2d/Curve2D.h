#pragma once

#include "math/Vector2.h"
#include <memory>

namespace cad {
namespace kernel {
namespace geometry2d {

struct BoundingBox2 {
    double minX{0.0}, maxX{0.0}, minY{0.0}, maxY{0.0};
};

class Curve2D {
public:
    virtual ~Curve2D() = default;
    virtual math::Point2 pointAt(double t) const = 0;
    virtual math::Vector2 tangentAt(double t) const = 0;
    virtual double length() const = 0;
    virtual BoundingBox2 bounds() const = 0;
    virtual std::unique_ptr<Curve2D> clone() const = 0;
};

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
