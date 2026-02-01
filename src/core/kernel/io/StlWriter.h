#pragma once

#include "topology/Solid.h"
#include "io/MeshGenerator.h"
#include <string>

namespace cad {
namespace kernel {
namespace io {

bool writeStl(const topology::Solid& solid, const std::string& filePath, bool binary = true);
bool writeStlMesh(const TriangleMesh& mesh, const std::string& filePath, bool binary = true);

}  // namespace io
}  // namespace kernel
}  // namespace cad
