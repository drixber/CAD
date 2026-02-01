#pragma once

#include "topology/Solid.h"
#include <memory>

namespace cad {
namespace kernel {
namespace boolean {

std::shared_ptr<topology::Solid> fuse(
    const std::shared_ptr<topology::Solid>& a,
    const std::shared_ptr<topology::Solid>& b);
std::shared_ptr<topology::Solid> cut(
    const std::shared_ptr<topology::Solid>& a,
    const std::shared_ptr<topology::Solid>& b);
std::shared_ptr<topology::Solid> common(
    const std::shared_ptr<topology::Solid>& a,
    const std::shared_ptr<topology::Solid>& b);

}  // namespace boolean
}  // namespace kernel
}  // namespace cad
