#include "advanced/LoftOps.h"
#include "topology/Shell.h"

namespace cad {
namespace kernel {
namespace advanced {

std::shared_ptr<topology::Solid> loft(
    const std::vector<std::shared_ptr<topology::Wire>>& /*profiles*/) {
    auto solid = std::make_shared<topology::Solid>();
    solid->setOuterShell(std::make_shared<topology::Shell>());
    return solid;
}

}  // namespace advanced
}  // namespace kernel
}  // namespace cad
