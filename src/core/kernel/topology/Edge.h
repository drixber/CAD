#pragma once

#include "math/Vector3.h"
#include "topology/Types.h"
#include "geometry3d/Curve3D.h"
#include <memory>

namespace cad {
namespace kernel {
namespace topology {

class Vertex;

class Edge {
public:
    Edge(std::shared_ptr<Vertex> start, std::shared_ptr<Vertex> end,
         geometry3d::Curve3D* curve, double t0, double t1, ShapeId id = 0);
    geometry3d::Curve3D* curve() { return curve_; }
    const geometry3d::Curve3D* curve() const { return curve_; }
    Vertex* startVertex() { return start_.get(); }
    const Vertex* startVertex() const { return start_.get(); }
    Vertex* endVertex() { return end_.get(); }
    const Vertex* endVertex() const { return end_.get(); }
    math::Point3 pointAt(double t) const;
    ShapeId id() const { return id_; }
    double t0() const { return t0_; }
    double t1() const { return t1_; }
private:
    std::shared_ptr<Vertex> start_;
    std::shared_ptr<Vertex> end_;
    geometry3d::Curve3D* curve_;
    double t0_{0.0}, t1_{1.0};
    ShapeId id_;
};

}  // namespace topology
}  // namespace kernel
}  // namespace cad
