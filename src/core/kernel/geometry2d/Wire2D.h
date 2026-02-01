#pragma once

#include "geometry2d/Curve2D.h"
#include "math/Tolerance.h"
#include <vector>
#include <memory>

namespace cad {
namespace kernel {
namespace geometry2d {

class Wire2D {
public:
    Wire2D() = default;
    void add(std::shared_ptr<Curve2D> curve);
    bool isClosed() const;
    BoundingBox2 bounds() const;
    double length() const;
    const std::vector<std::shared_ptr<Curve2D>>& curves() const { return curves_; }
private:
    std::vector<std::shared_ptr<Curve2D>> curves_;
};

}  // namespace geometry2d
}  // namespace kernel
}  // namespace cad
