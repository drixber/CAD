#pragma once

#include "topology/Solid.h"
#include <memory>
#include <vector>

namespace cad {
namespace kernel {
namespace advanced {

std::shared_ptr<topology::Solid> shell(
    const std::shared_ptr<topology::Solid>& solid,
    double wallThickness,
    const std::vector<topology::ShapeId>& faceIdsToRemove = {});

}  // namespace advanced
}  // namespace kernel
}  // namespace cad
