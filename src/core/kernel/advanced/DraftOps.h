#pragma once

#include "topology/Solid.h"
#include <memory>

namespace cad {
namespace kernel {
namespace advanced {

std::shared_ptr<topology::Solid> draft(
    const std::shared_ptr<topology::Solid>& solid,
    double angleDeg,
    const std::string& draftPlane,
    const std::vector<topology::ShapeId>& faceIds);

}  // namespace advanced
}  // namespace kernel
}  // namespace cad
