#include "fillet/FilletOps.h"

namespace cad {
namespace kernel {
namespace fillet {

std::shared_ptr<topology::Solid> fillet(
    const std::shared_ptr<topology::Solid>& solid,
    const std::vector<topology::ShapeId>& /*edgeIds*/,
    double /*radius*/) {
    return solid;
}

}  // namespace fillet
}  // namespace kernel
}  // namespace cad
