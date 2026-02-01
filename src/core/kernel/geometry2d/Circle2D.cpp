#include "geometry2d/Circle2D.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry2d {

Circle2D::Circle2D(const math::Point2& center, double radius)
    : center_(center), radius_(radius) {}

math::Point2 Circle2D::pointAt(double t) const {
    const double angle = t * twoPi;
    return math::Point2(
        center_.x + radius_ * std::cos(angle),
        center_.y + radius_ * std::sin(angle)
    );
}

math::Vector2 Circle2D::tangentAt(double t) const {
    const double angle = t * twoPi;
    return math::Vector2(-std::sin(angle), std::cos(angle)).normalized();
}

double Circle2D::length() const {
    return twoPi * radius_;
}

BoundingBox2 Circle2D::bounds() const {
    BoundingBox2 b;
    b.minX = center_.x - radius_;
    b.maxX = center_.x + radius_;
    b.minY = center_.y - radius_;
    b.maxY = center_.y + radius_;
    return b;
}

std::unique_ptr<Curve2D> Circle2D::clone() const {
    return std::make_unique<Circle2D>(center_, radius_);
}

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
