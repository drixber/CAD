#include "io/StlReader.h"
#include "topology/Vertex.h"
#include "topology/Edge.h"
#include "topology/Wire.h"
#include "topology/Loop.h"
#include "topology/Face.h"
#include "topology/Shell.h"
#include "topology/Solid.h"
#include "geometry3d/Line3D.h"
#include "geometry3d/PlaneSurface.h"
#include <fstream>
#include <memory>
#include <vector>
#include <cstring>
#include <cstdint>

namespace cad {
namespace kernel {
namespace io {

TriangleMesh readStlMesh(const std::string& filePath) {
    TriangleMesh mesh;
    std::ifstream f(filePath, std::ios::binary);
    if (!f) return mesh;
    char header[80];
    f.read(header, 80);
    bool binary = (std::strstr(header, "solid") == nullptr);
    if (!binary) {
        f.close();
        f.open(filePath);
        std::string line;
        while (std::getline(f, line)) {
            if (line.find("vertex") != std::string::npos) {
                double x, y, z;
                if (sscanf(line.c_str(), " vertex %lf %lf %lf", &x, &y, &z) == 3) {
                    mesh.vertices.push_back(x);
                    mesh.vertices.push_back(y);
                    mesh.vertices.push_back(z);
                }
            }
        }
        for (size_t i = 0; i + 2 < mesh.vertices.size() / 3; i += 3) {
            mesh.indices.push_back(static_cast<unsigned int>(i));
            mesh.indices.push_back(static_cast<unsigned int>(i + 1));
            mesh.indices.push_back(static_cast<unsigned int>(i + 2));
        }
        return mesh;
    }
    uint32_t n = 0;
    f.read(reinterpret_cast<char*>(&n), 4);
    for (uint32_t i = 0; i < n && f; ++i) {
        float norm[3], v0[3], v1[3], v2[3];
        f.read(reinterpret_cast<char*>(norm), 12);
        f.read(reinterpret_cast<char*>(v0), 12);
        f.read(reinterpret_cast<char*>(v1), 12);
        f.read(reinterpret_cast<char*>(v2), 12);
        uint16_t attr;
        f.read(reinterpret_cast<char*>(&attr), 2);
        size_t base = mesh.vertices.size() / 3;
        mesh.vertices.push_back(static_cast<double>(v0[0]));
        mesh.vertices.push_back(static_cast<double>(v0[1]));
        mesh.vertices.push_back(static_cast<double>(v0[2]));
        mesh.vertices.push_back(static_cast<double>(v1[0]));
        mesh.vertices.push_back(static_cast<double>(v1[1]));
        mesh.vertices.push_back(static_cast<double>(v1[2]));
        mesh.vertices.push_back(static_cast<double>(v2[0]));
        mesh.vertices.push_back(static_cast<double>(v2[1]));
        mesh.vertices.push_back(static_cast<double>(v2[2]));
        mesh.indices.push_back(static_cast<unsigned int>(base));
        mesh.indices.push_back(static_cast<unsigned int>(base + 1));
        mesh.indices.push_back(static_cast<unsigned int>(base + 2));
    }
    return mesh;
}

std::shared_ptr<topology::Solid> readStl(const std::string& filePath) {
    TriangleMesh mesh = readStlMesh(filePath);
    if (mesh.vertices.empty() || mesh.indices.size() < 3) return nullptr;
    auto shell = std::make_shared<topology::Shell>();
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        size_t a = mesh.indices[i] * 3, b = mesh.indices[i + 1] * 3, c = mesh.indices[i + 2] * 3;
        math::Point3 p0(mesh.vertices[a], mesh.vertices[a+1], mesh.vertices[a+2]);
        math::Point3 p1(mesh.vertices[b], mesh.vertices[b+1], mesh.vertices[b+2]);
        math::Point3 p2(mesh.vertices[c], mesh.vertices[c+1], mesh.vertices[c+2]);
        auto v0 = std::make_shared<topology::Vertex>(p0, 0);
        auto v1 = std::make_shared<topology::Vertex>(p1, 0);
        auto v2 = std::make_shared<topology::Vertex>(p2, 0);
        auto l0 = std::make_shared<geometry3d::Line3D>(p0, p1);
        auto l1 = std::make_shared<geometry3d::Line3D>(p1, p2);
        auto l2 = std::make_shared<geometry3d::Line3D>(p2, p0);
        auto wire = std::make_shared<topology::Wire>();
        wire->addEdge(std::make_shared<topology::Edge>(v0, v1, l0.get(), 0, 1, 0));
        wire->addEdge(std::make_shared<topology::Edge>(v1, v2, l1.get(), 0, 1, 0));
        wire->addEdge(std::make_shared<topology::Edge>(v2, v0, l2.get(), 0, 1, 0));
        math::Vector3 u = (p1 - p0).normalized();
        math::Vector3 v = (p2 - p0).normalized();
        auto plane = std::make_shared<geometry3d::PlaneSurface>(p0, u, v);
        auto loop = std::make_shared<topology::Loop>(wire);
        shell->addFace(std::make_shared<topology::Face>(plane, loop, std::vector<std::shared_ptr<topology::Loop>>{}, 0));
    }
    auto solid = std::make_shared<topology::Solid>();
    solid->setOuterShell(shell);
    return solid;
}

}  // namespace io
}  // namespace kernel
}  // namespace cad
