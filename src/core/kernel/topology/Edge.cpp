#include "topology/Edge.h"
#include "topology/Vertex.h"

namespace cad {
namespace kernel {
namespace topology {

Edge::Edge(std::shared_ptr<Vertex> start, std::shared_ptr<Vertex> end,
           geometry3d::Curve3D* curve, double t0, double t1, ShapeId id)
    : start_(std::move(start)), end_(std::move(end)), curve_(curve), t0_(t0), t1_(t1), id_(id) {}

math::Point3 Edge::pointAt(double t) const {
    if (curve_) {
        const double tt = t0_ + t * (t1_ - t0_);
        return curve_->pointAt(tt);
    }
    const math::Point3& a = start_->point();
    const math::Point3& b = end_->point();
    return math::Point3(
        a.x + t * (b.x - a.x),
        a.y + t * (b.y - a.y),
        a.z + t * (b.z - a.z)
    );
}

}  // namespace topology
}  // namespace kernel
}  // namespace cad
