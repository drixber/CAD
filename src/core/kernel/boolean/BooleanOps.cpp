#include "boolean/BooleanOps.h"

namespace cad {
namespace kernel {
namespace boolean {

std::shared_ptr<topology::Solid> fuse(
    const std::shared_ptr<topology::Solid>& a,
    const std::shared_ptr<topology::Solid>& /*b*/) {
    return a;
}

std::shared_ptr<topology::Solid> cut(
    const std::shared_ptr<topology::Solid>& a,
    const std::shared_ptr<topology::Solid>& /*b*/) {
    return a;
}

std::shared_ptr<topology::Solid> common(
    const std::shared_ptr<topology::Solid>& a,
    const std::shared_ptr<topology::Solid>& /*b*/) {
    return a;
}

}  // namespace boolean
}  // namespace kernel
}  // namespace cad
