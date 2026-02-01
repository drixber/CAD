#pragma once

#include "topology/Wire.h"
#include <memory>

namespace cad {
namespace kernel {
namespace topology {

class Loop {
public:
    explicit Loop(std::shared_ptr<Wire> wire);
    Wire* wire() { return wire_.get(); }
    const Wire* wire() const { return wire_.get(); }
    bool isClosed() const;
private:
    std::shared_ptr<Wire> wire_;
};

}  // namespace topology
}  // namespace kernel
}  // namespace cad
