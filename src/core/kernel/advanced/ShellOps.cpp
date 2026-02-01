#include "advanced/ShellOps.h"

namespace cad {
namespace kernel {
namespace advanced {

std::shared_ptr<topology::Solid> shell(
    const std::shared_ptr<topology::Solid>& solid,
    double /*wallThickness*/,
    const std::vector<topology::ShapeId>& /*faceIdsToRemove*/) {
    return solid;
}

}  // namespace advanced
}  // namespace kernel
}  // namespace cad
