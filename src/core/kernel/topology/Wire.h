#pragma once

#include "topology/Types.h"
#include "math/Tolerance.h"
#include <vector>
#include <memory>

namespace cad {
namespace kernel {
namespace topology {

class Edge;
class Vertex;

class Wire {
public:
    Wire() = default;
    void addEdge(std::shared_ptr<Edge> edge);
    bool isClosed() const;
    const std::vector<std::shared_ptr<Edge>>& edges() const { return edges_; }
    std::vector<const Vertex*> vertices() const;
private:
    std::vector<std::shared_ptr<Edge>> edges_;
};

}  // namespace topology
}  // namespace kernel
}  // namespace cad
