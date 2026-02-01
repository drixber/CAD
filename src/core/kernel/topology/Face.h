#pragma once

#include "topology/Types.h"
#include "topology/Loop.h"
#include "geometry3d/Surface.h"
#include <vector>
#include <memory>

namespace cad {
namespace kernel {
namespace topology {

class Face {
public:
    Face(std::shared_ptr<geometry3d::Surface> surface, std::shared_ptr<Loop> outerLoop,
         std::vector<std::shared_ptr<Loop>> innerLoops = {}, ShapeId id = 0);
    geometry3d::Surface* surface() { return surface_.get(); }
    const geometry3d::Surface* surface() const { return surface_.get(); }
    Loop* outerLoop() { return outerLoop_.get(); }
    const Loop* outerLoop() const { return outerLoop_.get(); }
    const std::vector<std::shared_ptr<Loop>>& innerLoops() const { return innerLoops_; }
    math::Vector3 normalAt(double u, double v) const;
    ShapeId id() const { return id_; }
private:
    std::shared_ptr<geometry3d::Surface> surface_;
    std::shared_ptr<Loop> outerLoop_;
    std::vector<std::shared_ptr<Loop>> innerLoops_;
    ShapeId id_;
};

}  // namespace topology
}  // namespace kernel
}  // namespace cad
