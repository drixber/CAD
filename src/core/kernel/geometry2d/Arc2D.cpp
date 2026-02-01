#include "geometry2d/Arc2D.h"
#include <cmath>
#include <algorithm>

namespace cad {
namespace kernel {
namespace geometry2d {

Arc2D::Arc2D(const math::Point2& center, double radius, double startAngle, double endAngle)
    : center_(center), radius_(radius), startAngle_(startAngle), endAngle_(endAngle) {}

math::Point2 Arc2D::pointAt(double t) const {
    const double angle = startAngle_ + t * (endAngle_ - startAngle_);
    return math::Point2(
        center_.x + radius_ * std::cos(angle),
        center_.y + radius_ * std::sin(angle)
    );
}

math::Vector2 Arc2D::tangentAt(double t) const {
    const double angle = startAngle_ + t * (endAngle_ - startAngle_);
    return math::Vector2(-std::sin(angle), std::cos(angle)).normalized();
}

double Arc2D::length() const {
    return std::abs(endAngle_ - startAngle_) * radius_;
}

BoundingBox2 Arc2D::bounds() const {
    BoundingBox2 b;
    b.minX = center_.x - radius_;
    b.maxX = center_.x + radius_;
    b.minY = center_.y - radius_;
    b.maxY = center_.y + radius_;
    for (int i = 0; i <= 16; ++i) {
        double t = i / 16.0;
        math::Point2 p = pointAt(t);
        b.minX = std::min(b.minX, p.x);
        b.maxX = std::max(b.maxX, p.x);
        b.minY = std::min(b.minY, p.y);
        b.maxY = std::max(b.maxY, p.y);
    }
    return b;
}

std::unique_ptr<Curve2D> Arc2D::clone() const {
    return std::make_unique<Arc2D>(center_, radius_, startAngle_, endAngle_);
}

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
