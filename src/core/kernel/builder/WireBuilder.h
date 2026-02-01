#pragma once

#include "geometry2d/Wire2D.h"
#include <vector>

namespace cad {
namespace core {
class Sketch;
}
namespace kernel {
namespace builder {

class WireBuilder {
public:
    static geometry2d::Wire2D build(const cad::core::Sketch& sketch);
    static std::vector<geometry2d::Wire2D> buildWires(const cad::core::Sketch& sketch);
};

}  // namespace builder
}  // namespace kernel
}  // namespace cad
