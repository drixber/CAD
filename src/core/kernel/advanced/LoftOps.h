#pragma once

#include "topology/Solid.h"
#include "topology/Wire.h"
#include <memory>
#include <vector>

namespace cad {
namespace kernel {
namespace advanced {

std::shared_ptr<topology::Solid> loft(
    const std::vector<std::shared_ptr<topology::Wire>>& profiles);

}  // namespace advanced
}  // namespace kernel
}  // namespace cad
