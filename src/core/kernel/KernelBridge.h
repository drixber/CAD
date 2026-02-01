#pragma once

#include "topology/Solid.h"
#include "io/MeshGenerator.h"
#include <memory>
#include <string>
#include <map>

namespace cad {
namespace core {
class Sketch;
class Part;
struct Feature;
}
namespace kernel {

class KernelBridge {
public:
    KernelBridge() = default;
    bool initialize();
    /** Build solid from single sketch (extrude with default depth). */
    bool buildPartFromSketch(const cad::core::Sketch& sketch);
    /** Build solid from Part and its feature list; sketches map sketch_id -> Sketch. */
    bool buildPartFromPart(const cad::core::Part& part,
                           const std::map<std::string, cad::core::Sketch>* sketches);
    std::shared_ptr<topology::Solid> getLastSolid() const { return lastSolid_; }
    io::TriangleMesh getLastSolidMesh() const;
    bool isAvailable() const { return initialized_; }
private:
    bool applyFeature(std::shared_ptr<topology::Solid>& solid,
                     const cad::core::Feature& feature,
                     const std::map<std::string, cad::core::Sketch>* sketches);
    bool initialized_{false};
    std::shared_ptr<topology::Solid> lastSolid_;
};

}  // namespace kernel
}  // namespace cad
