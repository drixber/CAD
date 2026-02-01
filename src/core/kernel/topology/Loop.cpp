#include "topology/Loop.h"

namespace cad {
namespace kernel {
namespace topology {

Loop::Loop(std::shared_ptr<Wire> wire) : wire_(std::move(wire)) {}

bool Loop::isClosed() const {
    return wire_ && wire_->isClosed();
}

}  // namespace topology
}  // namespace kernel
}  // namespace cad
