#include "topology/Shape.h"

namespace cad {
namespace kernel {
namespace topology {

Shape Shape::vertex(std::shared_ptr<Vertex> v) {
    Shape s;
    s.type_ = ShapeType::Vertex;
    s.vertex_ = std::move(v);
    return s;
}
Shape Shape::edge(std::shared_ptr<Edge> e) {
    Shape s;
    s.type_ = ShapeType::Edge;
    s.edge_ = std::move(e);
    return s;
}
Shape Shape::wire(std::shared_ptr<Wire> w) {
    Shape s;
    s.type_ = ShapeType::Wire;
    s.wire_ = std::move(w);
    return s;
}
Shape Shape::face(std::shared_ptr<Face> f) {
    Shape s;
    s.type_ = ShapeType::Face;
    s.face_ = std::move(f);
    return s;
}
Shape Shape::shell(std::shared_ptr<Shell> sh) {
    Shape s;
    s.type_ = ShapeType::Shell;
    s.shell_ = std::move(sh);
    return s;
}
Shape Shape::solid(std::shared_ptr<Solid> s) {
    Shape sh;
    sh.type_ = ShapeType::Solid;
    sh.solid_ = std::move(s);
    return sh;
}

Vertex* Shape::vertex() { return vertex_.get(); }
const Vertex* Shape::vertex() const { return vertex_.get(); }
Edge* Shape::edge() { return edge_.get(); }
const Edge* Shape::edge() const { return edge_.get(); }
Wire* Shape::wire() { return wire_.get(); }
const Wire* Shape::wire() const { return wire_.get(); }
Face* Shape::face() { return face_.get(); }
const Face* Shape::face() const { return face_.get(); }
Shell* Shape::shell() { return shell_.get(); }
const Shell* Shape::shell() const { return shell_.get(); }
Solid* Shape::solid() { return solid_.get(); }
const Solid* Shape::solid() const { return solid_.get(); }

std::vector<Edge*> Shape::getEdges(const Solid& s) {
    std::vector<Edge*> out;
    const Shell* sh = s.outerShell();
    if (!sh) return out;
    for (const auto& f : sh->faces()) {
        const Loop* loop = f->outerLoop();
        if (!loop || !loop->wire()) continue;
        for (const auto& e : loop->wire()->edges())
            out.push_back(e.get());
    }
    return out;
}

std::vector<Face*> Shape::getFaces(const Shell& sh) {
    std::vector<Face*> out;
    for (const auto& f : sh.faces())
        out.push_back(f.get());
    return out;
}

}  // namespace topology
}  // namespace kernel
}  // namespace cad
