#include "topology/Shell.h"

namespace cad {
namespace kernel {
namespace topology {

void Shell::addFace(std::shared_ptr<Face> face) {
    faces_.push_back(std::move(face));
}

}  // namespace topology
}  // namespace kernel
}  // namespace cad
