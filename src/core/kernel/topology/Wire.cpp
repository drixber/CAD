#include "topology/Wire.h"
#include "topology/Edge.h"
#include "topology/Vertex.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace topology {

void Wire::addEdge(std::shared_ptr<Edge> edge) {
    edges_.push_back(std::move(edge));
}

bool Wire::isClosed() const {
    if (edges_.size() < 2) return false;
    const math::Point3 firstStart = edges_.front()->startVertex()->point();
    const math::Point3 lastEnd = edges_.back()->endVertex()->point();
    return math::pointsEqual(firstStart, lastEnd);
}

std::vector<const Vertex*> Wire::vertices() const {
    std::vector<const Vertex*> v;
    for (const auto& e : edges_)
        if (e->startVertex()) v.push_back(e->startVertex());
    if (!edges_.empty() && edges_.back()->endVertex())
        v.push_back(edges_.back()->endVertex());
    return v;
}

}  // namespace topology
}  // namespace kernel
}  // namespace cad
