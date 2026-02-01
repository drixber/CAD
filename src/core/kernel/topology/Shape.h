#pragma once

#include "topology/Types.h"
#include "topology/Vertex.h"
#include "topology/Edge.h"
#include "topology/Wire.h"
#include "topology/Face.h"
#include "topology/Shell.h"
#include "topology/Solid.h"
#include <memory>
#include <vector>

namespace cad {
namespace kernel {
namespace topology {

class Shape {
public:
    Shape() = default;
    static Shape vertex(std::shared_ptr<Vertex> v);
    static Shape edge(std::shared_ptr<Edge> e);
    static Shape wire(std::shared_ptr<Wire> w);
    static Shape face(std::shared_ptr<Face> f);
    static Shape shell(std::shared_ptr<Shell> s);
    static Shape solid(std::shared_ptr<Solid> s);

    ShapeType type() const { return type_; }
    bool isNull() const { return type_ == ShapeType::Compound && !solid_; }
    Vertex* vertex();
    const Vertex* vertex() const;
    Edge* edge();
    const Edge* edge() const;
    Wire* wire();
    const Wire* wire() const;
    Face* face();
    const Face* face() const;
    Shell* shell();
    const Shell* shell() const;
    Solid* solid();
    const Solid* solid() const;

    static std::vector<Edge*> getEdges(const Solid& s);
    static std::vector<Face*> getFaces(const Shell& sh);
private:
    ShapeType type_{ShapeType::Compound};
    std::shared_ptr<Vertex> vertex_;
    std::shared_ptr<Edge> edge_;
    std::shared_ptr<Wire> wire_;
    std::shared_ptr<Face> face_;
    std::shared_ptr<Shell> shell_;
    std::shared_ptr<Solid> solid_;
};

}  // namespace topology
}  // namespace kernel
}  // namespace cad
