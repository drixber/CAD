#pragma once

#include "topology/Solid.h"
#include "math/Vector3.h"
#include <vector>

namespace cad {
namespace kernel {
namespace io {

struct TriangleMesh {
    std::vector<double> vertices;
    std::vector<unsigned int> indices;
};

TriangleMesh triangulate(const topology::Solid& solid);

}  // namespace io
}  // namespace kernel
}  // namespace cad
