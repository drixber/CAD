#include "geometry2d/Wire2D.h"
#include <algorithm>
#include <cmath>

namespace cad {
namespace kernel {
namespace geometry2d {

void Wire2D::add(std::shared_ptr<Curve2D> curve) {
    curves_.push_back(std::move(curve));
}

bool Wire2D::isClosed() const {
    if (curves_.empty()) return false;
    const math::Point2 firstStart = curves_.front()->pointAt(0.0);
    const math::Point2 lastEnd = curves_.back()->pointAt(1.0);
    const double dx = firstStart.x - lastEnd.x, dy = firstStart.y - lastEnd.y;
    return (dx*dx + dy*dy) <= math::kDistanceTolerance * math::kDistanceTolerance;
}

BoundingBox2 Wire2D::bounds() const {
    BoundingBox2 b{0, 0, 0, 0};
    bool first = true;
    for (const auto& c : curves_) {
        BoundingBox2 cb = c->bounds();
        if (first) {
            b = cb;
            first = false;
        } else {
            b.minX = std::min(b.minX, cb.minX);
            b.maxX = std::max(b.maxX, cb.maxX);
            b.minY = std::min(b.minY, cb.minY);
            b.maxY = std::max(b.maxY, cb.maxY);
        }
    }
    return b;
}

double Wire2D::length() const {
    double len = 0.0;
    for (const auto& c : curves_)
        len += c->length();
    return len;
}

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
