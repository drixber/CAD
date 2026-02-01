#include "advanced/SweepOps.h"

namespace cad {
namespace kernel {
namespace advanced {

std::shared_ptr<topology::Solid> sweep(
    const std::shared_ptr<topology::Wire>& /*profile*/,
    const geometry3d::Curve3D& /*path*/) {
    auto solid = std::make_shared<topology::Solid>();
    solid->setOuterShell(std::make_shared<topology::Shell>());
    return solid;
}

}  // namespace advanced
}  // namespace kernel
}  // namespace cad
