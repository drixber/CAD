#pragma once

#include "topology/Types.h"
#include "topology/Shell.h"
#include "math/Vector3.h"
#include <vector>
#include <memory>

namespace cad {
namespace kernel {
namespace topology {

class Solid {
public:
    Solid() = default;
    void setOuterShell(std::shared_ptr<Shell> shell);
    Shell* outerShell() { return outerShell_.get(); }
    const Shell* outerShell() const { return outerShell_.get(); }
    void addInnerShell(std::shared_ptr<Shell> shell);
    double volume() const;
    void bounds(double& minX, double& minY, double& minZ, double& maxX, double& maxY, double& maxZ) const;
private:
    std::shared_ptr<Shell> outerShell_;
    std::vector<std::shared_ptr<Shell>> innerShells_;
};

}  // namespace topology
}  // namespace kernel
}  // namespace cad
