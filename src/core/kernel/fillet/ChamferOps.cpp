#include "fillet/ChamferOps.h"

namespace cad {
namespace kernel {
namespace fillet {

std::shared_ptr<topology::Solid> chamfer(
    const std::shared_ptr<topology::Solid>& solid,
    const std::vector<topology::ShapeId>& /*edgeIds*/,
    double /*distance*/) {
    return solid;
}

}  // namespace fillet
}  // namespace kernel
}  // namespace cad
