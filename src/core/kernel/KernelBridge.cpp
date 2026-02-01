#include "KernelBridge.h"
#include "builder/WireBuilder.h"
#include "builder/FaceBuilder.h"
#include "builder/SolidBuilder.h"
#include "boolean/BooleanOps.h"
#include "fillet/FilletOps.h"
#include "fillet/ChamferOps.h"
#include "io/MeshGenerator.h"
#include "core/Modeler/Sketch.h"
#include "core/Modeler/Part.h"
#include <memory>
#include <cmath>

namespace cad {
namespace kernel {

bool KernelBridge::initialize() {
    initialized_ = true;
    return true;
}

bool KernelBridge::buildPartFromSketch(const cad::core::Sketch& sketch) {
    if (!initialized_) return false;
    geometry2d::Wire2D wire = builder::WireBuilder::build(sketch);
    if (wire.curves().empty()) return false;
    auto face = builder::FaceBuilder::buildPlanarFace(wire, math::Point3(0, 0, 0), math::Vector3(0, 0, 1));
    if (!face) return false;
    lastSolid_ = builder::SolidBuilder::extrude(face, math::Vector3(0, 0, 1), 10.0);
    return lastSolid_ != nullptr;
}

bool KernelBridge::applyFeature(std::shared_ptr<topology::Solid>& solid,
                                const cad::core::Feature& feature,
                                const std::map<std::string, cad::core::Sketch>* sketches) {
    if (!solid) return false;
    switch (feature.type) {
    case cad::core::FeatureType::Extrude:
        return true;
    case cad::core::FeatureType::Revolve:
        return true;
    case cad::core::FeatureType::Hole: {
        double r = (feature.diameter > 0.0) ? (feature.diameter * 0.5) : 2.5;
        double h = (feature.through_all) ? 1000.0 : (feature.hole_depth > 0.0 ? feature.hole_depth : 10.0);
        auto holeCyl = builder::SolidBuilder::cylinder(r, h);
        solid = boolean::cut(solid, holeCyl);
        return solid != nullptr;
    }
    case cad::core::FeatureType::Fillet: {
        std::vector<topology::ShapeId> edgeIds;
        double r = feature.radius > 0.0 ? feature.radius : 2.0;
        auto result = fillet::fillet(solid, edgeIds, r);
        if (result) solid = result;
        return true;
    }
    case cad::core::FeatureType::Chamfer: {
        std::vector<topology::ShapeId> edgeIds;
        double dist = 1.0;
        auto it = feature.parameters.find("distance1");
        if (it != feature.parameters.end()) dist = it->second;
        auto result = fillet::chamfer(solid, edgeIds, dist);
        if (result) solid = result;
        return true;
    }
    default:
        return true;
    }
}

bool KernelBridge::buildPartFromPart(const cad::core::Part& part,
                                    const std::map<std::string, cad::core::Sketch>* sketches) {
    if (!initialized_) return false;
    lastSolid_.reset();
    const std::vector<cad::core::Feature>& features = part.features();
    if (features.empty()) return false;

    size_t baseIdx = static_cast<size_t>(-1);
    for (size_t i = 0; i < features.size(); ++i) {
        if (features[i].type == cad::core::FeatureType::Extrude ||
            features[i].type == cad::core::FeatureType::Revolve) {
            baseIdx = i;
            break;
        }
    }
    if (baseIdx >= features.size()) return false;

    const cad::core::Feature& baseFeat = features[baseIdx];
    const cad::core::Sketch* sketch = nullptr;
    if (sketches) {
        auto it = sketches->find(baseFeat.sketch_id);
        if (it != sketches->end()) sketch = &it->second;
    }
    if (!sketch || sketch->geometry().empty()) return false;
    geometry2d::Wire2D wire = builder::WireBuilder::build(*sketch);
    if (wire.curves().empty()) return false;
    auto face = builder::FaceBuilder::buildPlanarFace(wire, math::Point3(0, 0, 0), math::Vector3(0, 0, 1));
    if (!face) return false;

    if (baseFeat.type == cad::core::FeatureType::Extrude) {
        double depth = baseFeat.depth > 0.0 ? baseFeat.depth : 10.0;
        math::Vector3 dir(0, 0, 1);
        if (baseFeat.symmetric) {
            depth *= 0.5;
            auto solidHalf = builder::SolidBuilder::extrude(face, math::Vector3(0, 0, -1), depth);
            lastSolid_ = builder::SolidBuilder::extrude(face, math::Vector3(0, 0, 1), depth);
            if (lastSolid_ && solidHalf)
                lastSolid_ = boolean::fuse(lastSolid_, solidHalf);
        } else {
            lastSolid_ = builder::SolidBuilder::extrude(face, dir, depth);
        }
    } else {
        builder::Axis axis;
        axis.point = math::Point3(0, 0, 0);
        axis.direction = math::Vector3(0, 0, 1);
        if (baseFeat.axis == "X") axis.direction = math::Vector3(1, 0, 0);
        else if (baseFeat.axis == "Y") axis.direction = math::Vector3(0, 1, 0);
        double angleDeg = (baseFeat.angle > 0.0 && baseFeat.angle <= 360.0) ? baseFeat.angle : 360.0;
        lastSolid_ = builder::SolidBuilder::revolve(face, axis, angleDeg);
    }
    if (!lastSolid_) return false;

    for (size_t i = 0; i < features.size(); ++i) {
        if (i == baseIdx) continue;
        if (!applyFeature(lastSolid_, features[i], sketches)) return false;
    }
    return true;
}

io::TriangleMesh KernelBridge::getLastSolidMesh() const {
    if (!lastSolid_) return io::TriangleMesh();
    return io::triangulate(*lastSolid_);
}

}  // namespace kernel
}  // namespace cad
