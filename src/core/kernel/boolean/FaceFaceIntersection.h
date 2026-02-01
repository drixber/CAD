#pragma once

#include "topology/Face.h"
#include "math/Vector3.h"
#include <vector>

namespace cad {
namespace kernel {
namespace boolean {

struct IntersectionSegment {
    math::Point3 start;
    math::Point3 end;
};

std::vector<IntersectionSegment> intersect(const topology::Face& a, const topology::Face& b);

}  // namespace boolean
}  // namespace kernel
}  // namespace cad
