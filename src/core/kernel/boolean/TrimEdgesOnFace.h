#pragma once

#include "topology/Face.h"
#include "boolean/FaceFaceIntersection.h"
#include <vector>
#include <memory>

namespace cad {
namespace kernel {
namespace boolean {

std::vector<std::shared_ptr<topology::Face>> trimFace(
    const topology::Face& face,
    const std::vector<IntersectionSegment>& segments);

}  // namespace boolean
}  // namespace kernel
}  // namespace cad
