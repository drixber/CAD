#include "geometry2d/Spline2D.h"
#include <cmath>
#include <algorithm>

namespace cad {
namespace kernel {
namespace geometry2d {

Spline2D::Spline2D(std::vector<math::Point2> points) : points_(std::move(points)) {
    segmentLengths_.resize(points_.size() > 1 ? points_.size() - 1 : 0);
    for (size_t i = 0; i + 1 < points_.size(); ++i) {
        segmentLengths_[i] = (points_[i + 1] - points_[i]).length();
        totalLength_ += segmentLengths_[i];
    }
}

math::Point2 Spline2D::pointAt(double t) const {
    if (points_.empty()) return math::Point2(0, 0);
    if (points_.size() == 1) return points_[0];
    if (t <= 0.0) return points_.front();
    if (t >= 1.0) return points_.back();
    const double targetLen = t * totalLength_;
    double acc = 0.0;
    for (size_t i = 0; i < segmentLengths_.size(); ++i) {
        if (acc + segmentLengths_[i] >= targetLen) {
            const double localT = (targetLen - acc) / segmentLengths_[i];
            return math::Point2(
                points_[i].x + localT * (points_[i + 1].x - points_[i].x),
                points_[i].y + localT * (points_[i + 1].y - points_[i].y)
            );
        }
        acc += segmentLengths_[i];
    }
    return points_.back();
}

math::Vector2 Spline2D::tangentAt(double t) const {
    const double dt = 1e-6;
    math::Point2 p0 = pointAt(std::max(0.0, t - dt));
    math::Point2 p1 = pointAt(std::min(1.0, t + dt));
    math::Vector2 d(p1.x - p0.x, p1.y - p0.y);
    const double len = d.length();
    if (len <= 0.0) return math::Vector2(1, 0);
    return d.normalized();
}

double Spline2D::length() const {
    return totalLength_;
}

BoundingBox2 Spline2D::bounds() const {
    BoundingBox2 b{0, 0, 0, 0};
    if (points_.empty()) return b;
    b.minX = b.maxX = points_[0].x;
    b.minY = b.maxY = points_[0].y;
    for (const auto& p : points_) {
        b.minX = std::min(b.minX, p.x);
        b.maxX = std::max(b.maxX, p.x);
        b.minY = std::min(b.minY, p.y);
        b.maxY = std::max(b.maxY, p.y);
    }
    return b;
}

std::unique_ptr<Curve2D> Spline2D::clone() const {
    return std::make_unique<Spline2D>(points_);
}

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
