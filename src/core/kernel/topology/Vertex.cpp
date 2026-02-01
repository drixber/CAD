#include "topology/Vertex.h"

namespace cad {
namespace kernel {
namespace topology {

Vertex::Vertex(const math::Point3& position, ShapeId id)
    : position_(position), id_(id) {}

}  // namespace topology
}  // namespace kernel
}  // namespace cad
