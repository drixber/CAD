#pragma once

#include "topology/Solid.h"
#include "topology/Face.h"
#include "math/Vector3.h"
#include <memory>
#include <vector>

namespace cad {
namespace kernel {
namespace builder {

struct Axis {
    math::Point3 point{0, 0, 0};
    math::Vector3 direction{0, 0, 1};
};

class SolidBuilder {
public:
    static std::shared_ptr<topology::Solid> box(double wx, double wy, double hz);
    static std::shared_ptr<topology::Solid> cylinder(double radius, double height);
    static std::shared_ptr<topology::Solid> sphere(double radius);
    static std::shared_ptr<topology::Solid> extrude(
        const std::shared_ptr<topology::Face>& baseFace,
        const math::Vector3& direction,
        double length);
    static std::shared_ptr<topology::Solid> revolve(
        const std::shared_ptr<topology::Face>& baseFace,
        const Axis& axis,
        double angleDeg);
};

}  // namespace builder
}  // namespace kernel
}  // namespace cad
