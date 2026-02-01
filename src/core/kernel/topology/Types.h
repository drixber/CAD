#pragma once

#include <cstddef>
#include <string>

namespace cad {
namespace kernel {
namespace topology {

enum class ShapeType {
    Vertex,
    Edge,
    Wire,
    Face,
    Shell,
    Solid,
    Compound
};

using ShapeId = size_t;

}  // namespace topology
}  // namespace kernel
}  // namespace cad
