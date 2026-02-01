#pragma once

#include "topology/Solid.h"
#include "io/MeshGenerator.h"
#include <string>
#include <memory>

namespace cad {
namespace kernel {
namespace io {

std::shared_ptr<topology::Solid> readStl(const std::string& filePath);
TriangleMesh readStlMesh(const std::string& filePath);

}  // namespace io
}  // namespace kernel
}  // namespace cad
