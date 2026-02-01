#include "advanced/DraftOps.h"

namespace cad {
namespace kernel {
namespace advanced {

std::shared_ptr<topology::Solid> draft(
    const std::shared_ptr<topology::Solid>& solid,
    double /*angleDeg*/,
    const std::string& /*draftPlane*/,
    const std::vector<topology::ShapeId>& /*faceIds*/) {
    return solid;
}

}  // namespace advanced
}  // namespace kernel
}  // namespace cad
