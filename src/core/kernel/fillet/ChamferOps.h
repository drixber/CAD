#pragma once

#include "topology/Solid.h"
#include "topology/Types.h"
#include <memory>
#include <vector>

namespace cad {
namespace kernel {
namespace fillet {

std::shared_ptr<topology::Solid> chamfer(
    const std::shared_ptr<topology::Solid>& solid,
    const std::vector<topology::ShapeId>& edgeIds,
    double distance);

}  // namespace fillet
}  // namespace kernel
}  // namespace cad
