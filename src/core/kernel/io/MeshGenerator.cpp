#include "io/MeshGenerator.h"
#include "topology/Face.h"
#include "topology/Edge.h"
#include "topology/Vertex.h"
#include "geometry3d/PlaneSurface.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace io {

TriangleMesh triangulate(const topology::Solid& solid) {
    TriangleMesh mesh;
    const topology::Shell* sh = solid.outerShell();
    if (!sh) return mesh;
    for (const auto& face : sh->faces()) {
        const topology::Loop* loop = face->outerLoop();
        if (!loop || !loop->wire() || loop->wire()->edges().size() < 3) continue;
        const auto& edges = loop->wire()->edges();
        if (edges.size() == 3) {
            math::Point3 p0 = edges[0]->startVertex()->point();
            math::Point3 p1 = edges[1]->startVertex()->point();
            math::Point3 p2 = edges[2]->startVertex()->point();
            size_t base = mesh.vertices.size() / 3;
            mesh.vertices.push_back(p0.x); mesh.vertices.push_back(p0.y); mesh.vertices.push_back(p0.z);
            mesh.vertices.push_back(p1.x); mesh.vertices.push_back(p1.y); mesh.vertices.push_back(p1.z);
            mesh.vertices.push_back(p2.x); mesh.vertices.push_back(p2.y); mesh.vertices.push_back(p2.z);
            mesh.indices.push_back(static_cast<unsigned int>(base));
            mesh.indices.push_back(static_cast<unsigned int>(base + 1));
            mesh.indices.push_back(static_cast<unsigned int>(base + 2));
        } else if (edges.size() == 4) {
            math::Point3 p0 = edges[0]->startVertex()->point();
            math::Point3 p1 = edges[1]->startVertex()->point();
            math::Point3 p2 = edges[2]->startVertex()->point();
            math::Point3 p3 = edges[3]->startVertex()->point();
            size_t base = mesh.vertices.size() / 3;
            mesh.vertices.push_back(p0.x); mesh.vertices.push_back(p0.y); mesh.vertices.push_back(p0.z);
            mesh.vertices.push_back(p1.x); mesh.vertices.push_back(p1.y); mesh.vertices.push_back(p1.z);
            mesh.vertices.push_back(p2.x); mesh.vertices.push_back(p2.y); mesh.vertices.push_back(p2.z);
            mesh.vertices.push_back(p3.x); mesh.vertices.push_back(p3.y); mesh.vertices.push_back(p3.z);
            mesh.indices.push_back(static_cast<unsigned int>(base));
            mesh.indices.push_back(static_cast<unsigned int>(base + 1));
            mesh.indices.push_back(static_cast<unsigned int>(base + 2));
            mesh.indices.push_back(static_cast<unsigned int>(base));
            mesh.indices.push_back(static_cast<unsigned int>(base + 2));
            mesh.indices.push_back(static_cast<unsigned int>(base + 3));
        }
    }
    return mesh;
}

}  // namespace io
}  // namespace kernel
}  // namespace cad
