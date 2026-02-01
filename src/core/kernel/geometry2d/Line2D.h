#pragma once

#include "geometry2d/Curve2D.h"

namespace cad {
namespace kernel {
namespace geometry2d {

class Line2D : public Curve2D {
public:
    Line2D(const math::Point2& start, const math::Point2& end);
    math::Point2 pointAt(double t) const override;
    math::Vector2 tangentAt(double t) const override;
    double length() const override;
    BoundingBox2 bounds() const override;
    std::unique_ptr<Curve2D> clone() const override;
private:
    math::Point2 start_;
    math::Point2 end_;
};

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
