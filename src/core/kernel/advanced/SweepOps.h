#pragma once

#include "topology/Solid.h"
#include "topology/Wire.h"
#include "geometry3d/Curve3D.h"
#include <memory>

namespace cad {
namespace kernel {
namespace advanced {

std::shared_ptr<topology::Solid> sweep(
    const std::shared_ptr<topology::Wire>& profile,
    const geometry3d::Curve3D& path);

}  // namespace advanced
}  // namespace kernel
}  // namespace cad
