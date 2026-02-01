#include "topology/Face.h"

namespace cad {
namespace kernel {
namespace topology {

Face::Face(std::shared_ptr<geometry3d::Surface> surface, std::shared_ptr<Loop> outerLoop,
           std::vector<std::shared_ptr<Loop>> innerLoops, ShapeId id)
    : surface_(std::move(surface)), outerLoop_(std::move(outerLoop)), innerLoops_(std::move(innerLoops)), id_(id) {}

math::Vector3 Face::normalAt(double u, double v) const {
    if (surface_) return surface_->normalAt(u, v);
    return math::Vector3(0, 0, 1);
}

}  // namespace topology
}  // namespace kernel
}  // namespace cad
