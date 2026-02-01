#pragma once

#include "math/Vector3.h"
#include "topology/Types.h"

namespace cad {
namespace kernel {
namespace topology {

class Vertex {
public:
    explicit Vertex(const math::Point3& position, ShapeId id = 0);
    const math::Point3& point() const { return position_; }
    ShapeId id() const { return id_; }
private:
    math::Point3 position_;
    ShapeId id_;
};

}  // namespace topology
}  // namespace kernel
}  // namespace cad
