#pragma once

#include "topology/Types.h"
#include "topology/Face.h"
#include <vector>
#include <memory>

namespace cad {
namespace kernel {
namespace topology {

class Shell {
public:
    Shell() = default;
    void addFace(std::shared_ptr<Face> face);
    const std::vector<std::shared_ptr<Face>>& faces() const { return faces_; }
private:
    std::vector<std::shared_ptr<Face>> faces_;
};

}  // namespace topology
}  // namespace kernel
}  // namespace cad
