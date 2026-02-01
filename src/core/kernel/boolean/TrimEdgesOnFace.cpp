#include "boolean/TrimEdgesOnFace.h"

namespace cad {
namespace kernel {
namespace boolean {

std::vector<std::shared_ptr<topology::Face>> trimFace(
    const topology::Face& /*face*/,
    const std::vector<IntersectionSegment>& /*segments*/) {
    return {};
}

}  // namespace boolean
}  // namespace kernel
}  // namespace cad
