#pragma once

#include "geometry2d/Curve2D.h"
#include <vector>

namespace cad {
namespace kernel {
namespace geometry2d {

class Spline2D : public Curve2D {
public:
    explicit Spline2D(std::vector<math::Point2> points);
    math::Point2 pointAt(double t) const override;
    math::Vector2 tangentAt(double t) const override;
    double length() const override;
    BoundingBox2 bounds() const override;
    std::unique_ptr<Curve2D> clone() const override;
private:
    std::vector<math::Point2> points_;
    std::vector<double> segmentLengths_;
    double totalLength_{0.0};
};

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
