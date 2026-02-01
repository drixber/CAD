#pragma once

#include "geometry2d/Curve2D.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry2d {

class Arc2D : public Curve2D {
public:
    Arc2D(const math::Point2& center, double radius, double startAngle, double endAngle);
    math::Point2 pointAt(double t) const override;
    math::Vector2 tangentAt(double t) const override;
    double length() const override;
    BoundingBox2 bounds() const override;
    std::unique_ptr<Curve2D> clone() const override;
private:
    math::Point2 center_;
    double radius_{0.0};
    double startAngle_{0.0};
    double endAngle_{0.0};
};

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
