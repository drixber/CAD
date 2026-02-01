#include "builder/SolidBuilder.h"
#include "topology/Vertex.h"
#include "topology/Edge.h"
#include "topology/Wire.h"
#include "topology/Loop.h"
#include "topology/Face.h"
#include "topology/Shell.h"
#include "topology/Solid.h"
#include "geometry3d/Line3D.h"
#include "geometry3d/Circle3D.h"
#include "geometry3d/PlaneSurface.h"
#include "geometry3d/CylinderSurface.h"
#include "geometry3d/SphereSurface.h"
#include "math/Transform3.h"
#include <memory>
#include <cmath>
#include <vector>

namespace cad {
namespace kernel {
namespace builder {

static const double pi = 3.14159265358979323846;
static const double deg2rad = pi / 180.0;

std::shared_ptr<topology::Solid> SolidBuilder::box(double wx, double wy, double hz) {
    auto shell = std::make_shared<topology::Shell>();
    topology::ShapeId vid = 0, eid = 0;
    std::vector<std::shared_ptr<topology::Vertex>> verts(8);
    verts[0] = std::make_shared<topology::Vertex>(math::Point3(0, 0, 0), vid++);
    verts[1] = std::make_shared<topology::Vertex>(math::Point3(wx, 0, 0), vid++);
    verts[2] = std::make_shared<topology::Vertex>(math::Point3(wx, wy, 0), vid++);
    verts[3] = std::make_shared<topology::Vertex>(math::Point3(0, wy, 0), vid++);
    verts[4] = std::make_shared<topology::Vertex>(math::Point3(0, 0, hz), vid++);
    verts[5] = std::make_shared<topology::Vertex>(math::Point3(wx, 0, hz), vid++);
    verts[6] = std::make_shared<topology::Vertex>(math::Point3(wx, wy, hz), vid++);
    verts[7] = std::make_shared<topology::Vertex>(math::Point3(0, wy, hz), vid++);

    auto addFace = [&](int a, int b, int c, int d, const math::Vector3& uAx, const math::Vector3& vAx) {
        auto plane = std::make_shared<geometry3d::PlaneSurface>(verts[a]->point(), uAx, vAx);
        auto l0 = std::make_shared<geometry3d::Line3D>(verts[a]->point(), verts[b]->point());
        auto l1 = std::make_shared<geometry3d::Line3D>(verts[b]->point(), verts[c]->point());
        auto l2 = std::make_shared<geometry3d::Line3D>(verts[c]->point(), verts[d]->point());
        auto l3 = std::make_shared<geometry3d::Line3D>(verts[d]->point(), verts[a]->point());
        auto wire = std::make_shared<topology::Wire>();
        wire->addEdge(std::make_shared<topology::Edge>(verts[a], verts[b], l0.get(), 0, 1, eid++));
        wire->addEdge(std::make_shared<topology::Edge>(verts[b], verts[c], l1.get(), 0, 1, eid++));
        wire->addEdge(std::make_shared<topology::Edge>(verts[c], verts[d], l2.get(), 0, 1, eid++));
        wire->addEdge(std::make_shared<topology::Edge>(verts[d], verts[a], l3.get(), 0, 1, eid++));
        auto loop = std::make_shared<topology::Loop>(wire);
        shell->addFace(std::make_shared<topology::Face>(plane, loop, std::vector<std::shared_ptr<topology::Loop>>{}, 0));
    };
    addFace(0, 1, 2, 3, math::Vector3(wx,0,0), math::Vector3(0,wy,0));
    addFace(4, 5, 6, 7, math::Vector3(wx,0,0), math::Vector3(0,wy,0));
    addFace(0, 1, 5, 4, math::Vector3(1,0,0), math::Vector3(0,0,1));
    addFace(2, 3, 7, 6, math::Vector3(-1,0,0), math::Vector3(0,0,1));
    addFace(1, 2, 6, 5, math::Vector3(0,1,0), math::Vector3(0,0,1));
    addFace(3, 0, 4, 7, math::Vector3(0,-1,0), math::Vector3(0,0,1));

    auto solid = std::make_shared<topology::Solid>();
    solid->setOuterShell(shell);
    return solid;
}

std::shared_ptr<topology::Solid> SolidBuilder::cylinder(double radius, double height) {
    auto shell = std::make_shared<topology::Shell>();
    math::Point3 origin(0, 0, 0);
    math::Vector3 axis(0, 0, 1);
    auto cylSurf = std::make_shared<geometry3d::CylinderSurface>(origin, axis, radius);
    cylSurf->setVMax(height);
    auto bottomCircle = std::make_shared<geometry3d::Circle3D>(origin, radius, axis);
    math::Point3 topCenter(0, 0, height);
    auto topCircle = std::make_shared<geometry3d::Circle3D>(topCenter, radius, axis);
    math::Point3 pSeamBottom(radius, 0, 0);
    math::Point3 pSeamTop(radius, 0, height);
    auto vBottom = std::make_shared<topology::Vertex>(pSeamBottom, 0);
    auto vTop = std::make_shared<topology::Vertex>(pSeamTop, 1);
    topology::ShapeId eid = 0;
    auto edgeBottomCircle = std::make_shared<topology::Edge>(vBottom, vBottom, bottomCircle.get(), 0, 2*pi, eid++);
    auto edgeTopCircle = std::make_shared<topology::Edge>(vTop, vTop, topCircle.get(), 0, 2*pi, eid++);
    auto lineUp = std::make_shared<geometry3d::Line3D>(pSeamBottom, pSeamTop);
    auto lineDown = std::make_shared<geometry3d::Line3D>(pSeamTop, pSeamBottom);
    auto edgeLineUp = std::make_shared<topology::Edge>(vBottom, vTop, lineUp.get(), 0, 1, eid++);
    auto edgeLineDown = std::make_shared<topology::Edge>(vTop, vBottom, lineDown.get(), 0, 1, eid++);
    auto wireBottom = std::make_shared<topology::Wire>();
    wireBottom->addEdge(edgeBottomCircle);
    auto loopBottom = std::make_shared<topology::Loop>(wireBottom);
    auto planeBottom = std::make_shared<geometry3d::PlaneSurface>(origin, math::Vector3(1,0,0), math::Vector3(0,1,0));
    shell->addFace(std::make_shared<topology::Face>(planeBottom, loopBottom, std::vector<std::shared_ptr<topology::Loop>>{}, 0));
    auto wireTop = std::make_shared<topology::Wire>();
    wireTop->addEdge(edgeTopCircle);
    auto loopTop = std::make_shared<topology::Loop>(wireTop);
    auto planeTop = std::make_shared<geometry3d::PlaneSurface>(topCenter, math::Vector3(1,0,0), math::Vector3(0,1,0));
    shell->addFace(std::make_shared<topology::Face>(planeTop, loopTop, std::vector<std::shared_ptr<topology::Loop>>{}, 1));
    auto wireMantle = std::make_shared<topology::Wire>();
    wireMantle->addEdge(edgeBottomCircle);
    wireMantle->addEdge(edgeLineUp);
    wireMantle->addEdge(edgeTopCircle);
    wireMantle->addEdge(edgeLineDown);
    auto loopMantle = std::make_shared<topology::Loop>(wireMantle);
    shell->addFace(std::make_shared<topology::Face>(cylSurf, loopMantle, std::vector<std::shared_ptr<topology::Loop>>{}, 2));
    auto solid = std::make_shared<topology::Solid>();
    solid->setOuterShell(shell);
    return solid;
}

std::shared_ptr<topology::Solid> SolidBuilder::sphere(double radius) {
    auto shell = std::make_shared<topology::Shell>();
    auto sphereSurf = std::make_shared<geometry3d::SphereSurface>(math::Point3(0,0,0), radius);
    auto loop = std::make_shared<topology::Loop>(std::make_shared<topology::Wire>());
    shell->addFace(std::make_shared<topology::Face>(sphereSurf, loop, std::vector<std::shared_ptr<topology::Loop>>{}, 0));
    auto solid = std::make_shared<topology::Solid>();
    solid->setOuterShell(shell);
    return solid;
}

std::shared_ptr<topology::Solid> SolidBuilder::extrude(
    const std::shared_ptr<topology::Face>& baseFace,
    const math::Vector3& direction,
    double length) {
    auto shell = std::make_shared<topology::Shell>();
    math::Vector3 dir = direction.normalized() * length;
    auto tr = math::Transform3::translate(dir.x, dir.y, dir.z);
    const topology::Loop* loop = baseFace->outerLoop();
    if (loop && loop->wire()) {
        for (const auto& edge : loop->wire()->edges()) {
            math::Point3 p0 = edge->startVertex()->point();
            math::Point3 p1 = edge->endVertex()->point();
            math::Point3 p0t = tr.apply(p0);
            math::Point3 p1t = tr.apply(p1);
            auto v0 = std::make_shared<topology::Vertex>(p0, 0);
            auto v1 = std::make_shared<topology::Vertex>(p1, 0);
            auto v0t = std::make_shared<topology::Vertex>(p0t, 0);
            auto v1t = std::make_shared<topology::Vertex>(p1t, 0);
            auto lineBot = std::make_shared<geometry3d::Line3D>(p0, p1);
            auto lineTop = std::make_shared<geometry3d::Line3D>(p0t, p1t);
            auto lineV0 = std::make_shared<geometry3d::Line3D>(p0, p0t);
            auto lineV1 = std::make_shared<geometry3d::Line3D>(p1, p1t);
            auto wire = std::make_shared<topology::Wire>();
            wire->addEdge(std::make_shared<topology::Edge>(v0, v1, lineBot.get(), 0, 1, 0));
            wire->addEdge(std::make_shared<topology::Edge>(v1, v1t, lineV1.get(), 0, 1, 0));
            wire->addEdge(std::make_shared<topology::Edge>(v1t, v0t, lineTop.get(), 0, 1, 0));
            wire->addEdge(std::make_shared<topology::Edge>(v0t, v0, lineV0.get(), 0, 1, 0));
            math::Vector3 n = (p1 - p0).cross(dir).normalized();
            auto plane = std::make_shared<geometry3d::PlaneSurface>(p0, p1 - p0, dir);
            shell->addFace(std::make_shared<topology::Face>(plane, std::make_shared<topology::Loop>(wire), std::vector<std::shared_ptr<topology::Loop>>{}, 0));
        }
    }
    auto solid = std::make_shared<topology::Solid>();
    solid->setOuterShell(shell);
    return solid;
}

std::shared_ptr<topology::Solid> SolidBuilder::revolve(
    const std::shared_ptr<topology::Face>& baseFace,
    const Axis& axis,
    double angleDeg) {
    const topology::Loop* loop = baseFace->outerLoop();
    if (!loop || !loop->wire()) {
        auto solid = std::make_shared<topology::Solid>();
        solid->setOuterShell(std::make_shared<topology::Shell>());
        return solid;
    }
    double rMin = 1e30, rMax = -1e30;
    double zMin = 1e30, zMax = -1e30;
    math::Vector3 ax = axis.direction.normalized();
    for (const auto& edge : loop->wire()->edges()) {
        math::Point3 p0 = edge->startVertex()->point();
        math::Point3 p1 = edge->endVertex()->point();
        double d0 = std::sqrt((p0.x - axis.point.x) * (p0.x - axis.point.x) +
                              (p0.y - axis.point.y) * (p0.y - axis.point.y) +
                              (p0.z - axis.point.z) * (p0.z - axis.point.z));
        double d1 = std::sqrt((p1.x - axis.point.x) * (p1.x - axis.point.x) +
                              (p1.y - axis.point.y) * (p1.y - axis.point.y) +
                              (p1.z - axis.point.z) * (p1.z - axis.point.z));
        double proj0 = (p0.x - axis.point.x) * ax.x + (p0.y - axis.point.y) * ax.y + (p0.z - axis.point.z) * ax.z;
        double proj1 = (p1.x - axis.point.x) * ax.x + (p1.y - axis.point.y) * ax.y + (p1.z - axis.point.z) * ax.z;
        double r0 = std::sqrt(std::max(0.0, d0*d0 - proj0*proj0));
        double r1 = std::sqrt(std::max(0.0, d1*d1 - proj1*proj1));
        rMin = std::min(rMin, std::min(r0, r1));
        rMax = std::max(rMax, std::max(r0, r1));
        zMin = std::min(zMin, std::min(p0.z, p1.z));
        zMax = std::max(zMax, std::max(p0.z, p1.z));
    }
    double radius = (rMax > rMin) ? rMax : (rMin > 1e-29 ? rMin : 1.0);
    double height = (zMax > zMin) ? (zMax - zMin) : 1.0;
    if (radius < 1e-10) radius = 1.0;
    if (height < 1e-10) height = 1.0;
    std::shared_ptr<topology::Solid> solid = cylinder(radius, height);
    if (angleDeg < 360.0 && angleDeg > 0.0) {
        (void)angleDeg;
    }
    return solid;
}

}  // namespace builder
}  // namespace kernel
}  // namespace cad
