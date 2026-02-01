#pragma once

#include "topology/Face.h"
#include "geometry2d/Wire2D.h"
#include "geometry3d/PlaneSurface.h"
#include <memory>

namespace cad {
namespace kernel {
namespace builder {

class FaceBuilder {
public:
    static std::shared_ptr<topology::Face> buildPlanarFace(
        const geometry2d::Wire2D& wire,
        const math::Point3& origin = math::Point3(0, 0, 0),
        const math::Vector3& normal = math::Vector3(0, 0, 1));
};

}  // namespace builder
}  // namespace kernel
}  // namespace cad
