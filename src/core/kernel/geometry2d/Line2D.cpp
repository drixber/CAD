#include "geometry2d/Line2D.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry2d {

Line2D::Line2D(const math::Point2& start, const math::Point2& end)
    : start_(start), end_(end) {}

math::Point2 Line2D::pointAt(double t) const {
    return math::Point2(
        start_.x + t * (end_.x - start_.x),
        start_.y + t * (end_.y - start_.y)
    );
}

math::Vector2 Line2D::tangentAt(double /*t*/) const {
    math::Vector2 d(end_.x - start_.x, end_.y - start_.y);
    const double len = d.length();
    if (len <= 0.0) return math::Vector2(1, 0);
    return d.normalized();
}

double Line2D::length() const {
    return (end_ - start_).length();
}

BoundingBox2 Line2D::bounds() const {
    BoundingBox2 b;
    b.minX = std::min(start_.x, end_.x);
    b.maxX = std::max(start_.x, end_.x);
    b.minY = std::min(start_.y, end_.y);
    b.maxY = std::max(start_.y, end_.y);
    return b;
}

std::unique_ptr<Curve2D> Line2D::clone() const {
    return std::make_unique<Line2D>(start_, end_);
}

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
