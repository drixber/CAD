#include "builder/FaceBuilder.h"
#include "geometry2d/Line2D.h"
#include "geometry2d/Circle2D.h"
#include "geometry2d/Arc2D.h"
#include "geometry3d/Line3D.h"
#include "geometry3d/Circle3D.h"
#include "topology/Vertex.h"
#include "topology/Edge.h"
#include "topology/Wire.h"
#include "topology/Loop.h"
#include "topology/Face.h"
#include "math/Transform3.h"
#include <memory>
#include <cmath>

namespace cad {
namespace kernel {
namespace builder {

static void makePlaneAxes(const math::Vector3& normal, math::Vector3& uAxis, math::Vector3& vAxis) {
    math::Vector3 n = normal.normalized();
    if (std::abs(n.x) < 0.9)
        uAxis = math::Vector3(1, 0, 0).cross(n).normalized();
    else
        uAxis = math::Vector3(0, 1, 0).cross(n).normalized();
    vAxis = n.cross(uAxis).normalized();
}

std::shared_ptr<topology::Face> FaceBuilder::buildPlanarFace(
    const geometry2d::Wire2D& wire,
    const math::Point3& origin,
    const math::Vector3& normal) {
    math::Vector3 uAxis, vAxis;
    makePlaneAxes(normal, uAxis, vAxis);
    auto plane = std::make_shared<geometry3d::PlaneSurface>(origin, uAxis, vAxis);

    auto shellWire = std::make_shared<topology::Wire>();
    topology::ShapeId vid = 0, eid = 0;
    std::shared_ptr<topology::Vertex> firstVertex, prevEnd;
    std::vector<std::shared_ptr<geometry3d::Curve3D>> ownedCurves;
    const auto& curves = wire.curves();
    for (size_t i = 0; i < curves.size(); ++i) {
        const auto& curve = curves[i];
        math::Point2 p0 = curve->pointAt(0.0);
        math::Point2 p1 = curve->pointAt(1.0);
        math::Point3 pt0(origin.x + p0.x * uAxis.x + p0.y * vAxis.x,
                         origin.y + p0.x * uAxis.y + p0.y * vAxis.y,
                         origin.z + p0.x * uAxis.z + p0.y * vAxis.z);
        math::Point3 pt1(origin.x + p1.x * uAxis.x + p1.y * vAxis.x,
                         origin.y + p1.x * uAxis.y + p1.y * vAxis.y,
                         origin.z + p1.x * uAxis.z + p1.y * vAxis.z);
        std::shared_ptr<topology::Vertex> v0 = prevEnd ? prevEnd : std::make_shared<topology::Vertex>(pt0, vid++);
        if (!firstVertex) firstVertex = v0;
        auto v1 = std::make_shared<topology::Vertex>(pt1, vid++);
        auto line3d = std::make_shared<geometry3d::Line3D>(pt0, pt1);
        ownedCurves.push_back(line3d);
        auto edge = std::make_shared<topology::Edge>(v0, v1, line3d.get(), 0.0, 1.0, eid++);
        shellWire->addEdge(edge);
        prevEnd = v1;
    }
    auto loop = std::make_shared<topology::Loop>(shellWire);
    auto face = std::make_shared<topology::Face>(plane, loop, std::vector<std::shared_ptr<topology::Loop>>{}, 0);
    (void)ownedCurves;
    return face;
}

}  // namespace builder
}  // namespace kernel
}  // namespace cad
