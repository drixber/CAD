/**
 * Tests für den eigenen CAD-Kern (cad_eigen_kernel).
 * Prüft alle implementierten Funktionen gemäß PLAN_EIGENER_CAD_KERN.md.
 */
#ifdef CAD_USE_EIGENER_KERN

#include <gtest/gtest.h>
#include <cmath>
#include <fstream>
#include <map>
#include "core/Modeler/Part.h"
#include "core/kernel/math/Vector2.h"
#include "core/kernel/math/Vector3.h"
#include "core/kernel/math/Matrix3.h"
#include "core/kernel/math/Transform3.h"
#include "core/kernel/math/Tolerance.h"
#include "core/kernel/geometry2d/Line2D.h"
#include "core/kernel/geometry2d/Circle2D.h"
#include "core/kernel/geometry2d/Arc2D.h"
#include "core/kernel/geometry2d/Spline2D.h"
#include "core/kernel/geometry2d/Wire2D.h"
#include "core/kernel/topology/Vertex.h"
#include "core/kernel/topology/Edge.h"
#include "core/kernel/topology/Wire.h"
#include "core/kernel/topology/Loop.h"
#include "core/kernel/topology/Face.h"
#include "core/kernel/topology/Shell.h"
#include "core/kernel/topology/Solid.h"
#include "core/kernel/topology/Shape.h"
#include "core/kernel/geometry3d/Line3D.h"
#include "core/kernel/geometry3d/Circle3D.h"
#include "core/kernel/geometry3d/PlaneSurface.h"
#include "core/kernel/geometry3d/CylinderSurface.h"
#include "core/kernel/geometry3d/SphereSurface.h"
#include "core/kernel/builder/WireBuilder.h"
#include "core/kernel/builder/FaceBuilder.h"
#include "core/kernel/builder/SolidBuilder.h"
#include "core/kernel/boolean/BooleanOps.h"
#include "core/kernel/fillet/FilletOps.h"
#include "core/kernel/fillet/ChamferOps.h"
#include "core/kernel/io/MeshGenerator.h"
#include "core/kernel/io/StlWriter.h"
#include "core/kernel/io/StlReader.h"
#include "core/kernel/KernelBridge.h"
#include "core/Modeler/Sketch.h"

using namespace cad::kernel;
using namespace cad::kernel::math;
using namespace cad::kernel::geometry2d;
using namespace cad::kernel::geometry3d;
using namespace cad::kernel::topology;
using namespace cad::kernel::builder;
using namespace cad::kernel::boolean;
using namespace cad::kernel::fillet;
using namespace cad::kernel::io;

static const double kEps = 1e-9;
static const double kPi = 3.14159265358979323846;

// --- Phase 0: Math & Toleranzen ---
TEST(EigenKernel, Vector2LengthAndNormalize) {
    Vector2 v(3, 4);
    EXPECT_NEAR(v.length(), 5.0, kEps);
    Vector2 n = v.normalized();
    EXPECT_NEAR(n.length(), 1.0, kEps);
    EXPECT_NEAR(n.x, 0.6, kEps);
    EXPECT_NEAR(n.y, 0.8, kEps);
}

TEST(EigenKernel, Vector2DotAndAngle) {
    Vector2 a(1, 0), b(0, 1);
    EXPECT_NEAR(a.dot(b), 0.0, kEps);
    EXPECT_NEAR(a.angleTo(b), kPi / 2, kEps);
}

TEST(EigenKernel, Vector3CrossProduct) {
    Vector3 x(1, 0, 0), y(0, 1, 0);
    Vector3 z = x.cross(y);
    EXPECT_NEAR(z.x, 0.0, kEps);
    EXPECT_NEAR(z.y, 0.0, kEps);
    EXPECT_NEAR(z.z, 1.0, kEps);
}

TEST(EigenKernel, Matrix3RotationZ) {
    Matrix3 r = Matrix3::rotationZ(kPi / 2);
    Vector3 v(1, 0, 0);
    Vector3 w = r.apply(v);
    EXPECT_NEAR(w.x, 0.0, kEps);
    EXPECT_NEAR(w.y, 1.0, kEps);
    EXPECT_NEAR(w.z, 0.0, kEps);
}

TEST(EigenKernel, Transform3Translate) {
    Transform3 t = Transform3::translate(1, 2, 3);
    Point3 p(0, 0, 0);
    Point3 q = t.apply(p);
    EXPECT_NEAR(q.x, 1.0, kEps);
    EXPECT_NEAR(q.y, 2.0, kEps);
    EXPECT_NEAR(q.z, 3.0, kEps);
}

TEST(EigenKernel, TolerancePointsEqual) {
    Point3 a(0, 0, 0), b(kDistanceTolerance * 0.5, 0, 0);
    EXPECT_TRUE(pointsEqual(a, b));
    Point3 c(1e-5, 0, 0);
    EXPECT_FALSE(pointsEqual(a, c));
}

// --- Phase 1: 2D-Geometrie ---
TEST(EigenKernel, Line2DPointAtAndLength) {
    Line2D line(Point2(0, 0), Point2(10, 0));
    Point2 mid = line.pointAt(0.5);
    EXPECT_NEAR(mid.x, 5.0, kEps);
    EXPECT_NEAR(mid.y, 0.0, kEps);
    EXPECT_NEAR(line.length(), 10.0, kEps);
}

TEST(EigenKernel, Circle2DLength) {
    Circle2D circle(Point2(0, 0), 5.0);
    EXPECT_NEAR(circle.length(), 2 * kPi * 5.0, kEps);
    Point2 p = circle.pointAt(0.0);
    EXPECT_NEAR(p.x, 5.0, kEps);
    EXPECT_NEAR(p.y, 0.0, kEps);
}

TEST(EigenKernel, Arc2DEndpoints) {
    Arc2D arc(Point2(0, 0), 1.0, 0.0, kPi / 2);
    Point2 start = arc.pointAt(0.0);
    Point2 end = arc.pointAt(1.0);
    EXPECT_NEAR(start.x, 1.0, kEps);
    EXPECT_NEAR(start.y, 0.0, kEps);
    EXPECT_NEAR(end.x, 0.0, kEps);
    EXPECT_NEAR(end.y, 1.0, kEps);
}

TEST(EigenKernel, Wire2DClosedRectangle) {
    Wire2D wire;
    wire.add(std::make_shared<Line2D>(Point2(0, 0), Point2(10, 0)));
    wire.add(std::make_shared<Line2D>(Point2(10, 0), Point2(10, 5)));
    wire.add(std::make_shared<Line2D>(Point2(10, 5), Point2(0, 5)));
    wire.add(std::make_shared<Line2D>(Point2(0, 5), Point2(0, 0)));
    EXPECT_TRUE(wire.isClosed());
    EXPECT_NEAR(wire.length(), 30.0, kEps);
}

// --- Phase 2: Topologie ---
TEST(EigenKernel, VertexAndEdge) {
    auto v0 = std::make_shared<Vertex>(Point3(0, 0, 0), 0);
    auto v1 = std::make_shared<Vertex>(Point3(1, 0, 0), 1);
    auto line3d = std::make_shared<Line3D>(Point3(0, 0, 0), Point3(1, 0, 0));
    topology::Edge edge(v0, v1, line3d.get(), 0.0, 1.0, 0);
    Point3 p = edge.pointAt(0.5);
    EXPECT_NEAR(p.x, 0.5, kEps);
    EXPECT_NEAR(p.y, 0.0, kEps);
    EXPECT_NEAR(p.z, 0.0, kEps);
}

TEST(EigenKernel, ShapeSolidAndExplorer) {
    auto solid = SolidBuilder::box(10, 20, 5);
    ASSERT_TRUE(solid != nullptr);
    Shape sh = Shape::solid(solid);
    EXPECT_FALSE(sh.isNull());
    EXPECT_EQ(sh.type(), ShapeType::Solid);
    std::vector<Edge*> edges = Shape::getEdges(*solid);
    EXPECT_GE(edges.size(), 12u);
    const Shell* shell = solid->outerShell();
    ASSERT_TRUE(shell != nullptr);
    std::vector<Face*> faces = Shape::getFaces(*shell);
    EXPECT_EQ(faces.size(), 6u);
}

// --- Phase 3: 3D-Geometrie ---
TEST(EigenKernel, PlaneSurfacePointAndNormal) {
    PlaneSurface plane(Point3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0));
    Point3 p = plane.pointAt(2.0, 3.0);
    EXPECT_NEAR(p.x, 2.0, kEps);
    EXPECT_NEAR(p.y, 3.0, kEps);
    EXPECT_NEAR(p.z, 0.0, kEps);
    Vector3 n = plane.normalAt(0, 0);
    EXPECT_NEAR(n.z, 1.0, kEps);
}

TEST(EigenKernel, CylinderSurfacePointAt) {
    CylinderSurface cyl(Point3(0, 0, 0), Vector3(0, 0, 1), 5.0);
    Point3 p = cyl.pointAt(0.0, 0.0);
    EXPECT_NEAR(p.z, 0.0, kEps);
    double distFromAxis = std::sqrt(p.x * p.x + p.y * p.y);
    EXPECT_NEAR(distFromAxis, 5.0, kEps);
}

// --- Phase 4: Sketch → Wire2D → Face ---
TEST(EigenKernel, WireBuilderFromSketchRectangle) {
    cad::core::Sketch sketch("Test");
    sketch.addRectangle({0, 0}, 10, 5);
    Wire2D wire = WireBuilder::build(sketch);
    EXPECT_GE(wire.curves().size(), 4u);
    EXPECT_TRUE(wire.isClosed());
}

TEST(EigenKernel, WireBuilderFromSketchCircle) {
    cad::core::Sketch sketch("Test");
    sketch.addCircle({5, 5}, 3.0);
    Wire2D wire = WireBuilder::build(sketch);
    EXPECT_EQ(wire.curves().size(), 1u);
    EXPECT_TRUE(wire.isClosed());
    EXPECT_NEAR(wire.length(), 2 * kPi * 3.0, kEps);
}

TEST(EigenKernel, FaceBuilderPlanarFace) {
    Wire2D wire;
    wire.add(std::make_shared<Line2D>(Point2(0, 0), Point2(10, 0)));
    wire.add(std::make_shared<Line2D>(Point2(10, 0), Point2(10, 5)));
    wire.add(std::make_shared<Line2D>(Point2(10, 5), Point2(0, 5)));
    wire.add(std::make_shared<Line2D>(Point2(0, 5), Point2(0, 0)));
    auto face = FaceBuilder::buildPlanarFace(wire, Point3(0, 0, 0), Vector3(0, 0, 1));
    ASSERT_TRUE(face != nullptr);
    EXPECT_TRUE(face->surface() != nullptr);
    const Loop* loop = face->outerLoop();
    ASSERT_TRUE(loop != nullptr);
    ASSERT_TRUE(loop->wire() != nullptr);
    EXPECT_EQ(loop->wire()->edges().size(), 4u);
}

// --- Phase 5: SolidBuilder ---
TEST(EigenKernel, SolidBuilderBox) {
    auto solid = SolidBuilder::box(10, 20, 5);
    ASSERT_TRUE(solid != nullptr);
    const Shell* sh = solid->outerShell();
    ASSERT_TRUE(sh != nullptr);
    EXPECT_EQ(sh->faces().size(), 6u);
    std::vector<Edge*> edges = Shape::getEdges(*solid);
    EXPECT_GE(edges.size(), 12u);
    double vol = solid->volume();
    EXPECT_GT(vol, 0.0);
}

TEST(EigenKernel, SolidBuilderExtrude) {
    Wire2D wire;
    wire.add(std::make_shared<Line2D>(Point2(0, 0), Point2(5, 0)));
    wire.add(std::make_shared<Line2D>(Point2(5, 0), Point2(5, 5)));
    wire.add(std::make_shared<Line2D>(Point2(5, 5), Point2(0, 5)));
    wire.add(std::make_shared<Line2D>(Point2(0, 5), Point2(0, 0)));
    auto face = FaceBuilder::buildPlanarFace(wire, Point3(0, 0, 0), Vector3(0, 0, 1));
    ASSERT_TRUE(face != nullptr);
    auto solid = SolidBuilder::extrude(face, Vector3(0, 0, 1), 10.0);
    ASSERT_TRUE(solid != nullptr);
    const Shell* sh = solid->outerShell();
    ASSERT_TRUE(sh != nullptr);
    EXPECT_GT(sh->faces().size(), 0u);
}

// --- Phase 6: Boolean (Stub) ---
TEST(EigenKernel, BooleanFuseReturnsSolid) {
    auto a = SolidBuilder::box(5, 5, 5);
    auto b = SolidBuilder::box(2, 2, 2);
    ASSERT_TRUE(a != nullptr);
    ASSERT_TRUE(b != nullptr);
    auto c = fuse(a, b);
    ASSERT_TRUE(c != nullptr);
}

TEST(EigenKernel, BooleanCutReturnsSolid) {
    auto a = SolidBuilder::box(5, 5, 5);
    auto b = SolidBuilder::box(2, 2, 2);
    auto c = cut(a, b);
    ASSERT_TRUE(c != nullptr);
}

// --- Phase 7: Fillet/Chamfer (Stub) ---
TEST(EigenKernel, FilletReturnsSolid) {
    auto solid = SolidBuilder::box(5, 5, 5);
    std::vector<ShapeId> edgeIds = {0};
    auto result = fillet::fillet(solid, edgeIds, 1.0);
    ASSERT_TRUE(result != nullptr);
}

TEST(EigenKernel, ChamferReturnsSolid) {
    auto solid = SolidBuilder::box(5, 5, 5);
    std::vector<ShapeId> edgeIds = {0};
    auto result = fillet::chamfer(solid, edgeIds, 1.0);
    ASSERT_TRUE(result != nullptr);
}

// --- Phase 9: MeshGenerator & STL ---
TEST(EigenKernel, MeshGeneratorTriangulateBox) {
    auto solid = SolidBuilder::box(2, 2, 2);
    TriangleMesh mesh = triangulate(*solid);
    EXPECT_GT(mesh.vertices.size(), 0u);
    EXPECT_GT(mesh.indices.size(), 0u);
    EXPECT_EQ(mesh.indices.size() % 3, 0u);
}

TEST(EigenKernel, StlWriteAndRead) {
    auto solid = SolidBuilder::box(1, 1, 1);
    std::string path = "eigen_kernel_test_box.stl";
    ASSERT_TRUE(writeStl(*solid, path, true));
    auto readBack = readStl(path);
    ASSERT_TRUE(readBack != nullptr);
    TriangleMesh mesh = triangulate(*readBack);
    EXPECT_GT(mesh.vertices.size(), 0u);
    std::remove(path.c_str());
}

// --- Phase 10: KernelBridge ---
TEST(EigenKernel, KernelBridgeInitialize) {
    KernelBridge bridge;
    EXPECT_TRUE(bridge.initialize());
    EXPECT_TRUE(bridge.isAvailable());
}

TEST(EigenKernel, KernelBridgeBuildPartFromSketch) {
    KernelBridge bridge;
    ASSERT_TRUE(bridge.initialize());
    cad::core::Sketch sketch("Test");
    sketch.addRectangle({0, 0}, 10, 5);
    bool ok = bridge.buildPartFromSketch(sketch);
    EXPECT_TRUE(ok);
    auto solid = bridge.getLastSolid();
    ASSERT_TRUE(solid != nullptr);
}

TEST(EigenKernel, KernelBridgeGetLastSolidMesh) {
    KernelBridge bridge;
    ASSERT_TRUE(bridge.initialize());
    cad::core::Sketch sketch("Test");
    sketch.addCircle({0, 0}, 5.0);
    bridge.buildPartFromSketch(sketch);
    io::TriangleMesh mesh = bridge.getLastSolidMesh();
    EXPECT_GT(mesh.vertices.size(), 0u);
}

TEST(EigenKernel, SolidBuilderCylinder) {
    auto solid = builder::SolidBuilder::cylinder(3.0, 5.0);
    ASSERT_TRUE(solid != nullptr);
    ASSERT_TRUE(solid->outerShell() != nullptr);
    EXPECT_GE(solid->outerShell()->faces().size(), 3u);
}

TEST(EigenKernel, KernelBridgeBuildPartFromPart) {
    KernelBridge bridge;
    ASSERT_TRUE(bridge.initialize());
    cad::core::Sketch sketch("Sketch1");
    sketch.addRectangle({0, 0}, 8, 4);
    cad::core::Part part(sketch.name());
    part.createExtrude(sketch.name(), 6.0, false);
    std::map<std::string, cad::core::Sketch> sketches;
    sketches.insert(std::make_pair(sketch.name(), sketch));
    bool ok = bridge.buildPartFromPart(part, &sketches);
    EXPECT_TRUE(ok);
    auto solid = bridge.getLastSolid();
    ASSERT_TRUE(solid != nullptr);
    io::TriangleMesh mesh = bridge.getLastSolidMesh();
    EXPECT_GT(mesh.vertices.size(), 0u);
}

#endif // CAD_USE_EIGENER_KERN
