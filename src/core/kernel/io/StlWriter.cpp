#include "io/StlWriter.h"
#include <fstream>
#include <cstring>
#include <cstdint>

namespace cad {
namespace kernel {
namespace io {

bool writeStlMesh(const TriangleMesh& mesh, const std::string& filePath, bool binary) {
    if (binary) {
        std::ofstream f(filePath, std::ios::binary);
        if (!f) return false;
        char header[80] = "HydraCAD kernel STL";
        f.write(header, 80);
        uint32_t n = static_cast<uint32_t>(mesh.indices.size() / 3);
        f.write(reinterpret_cast<const char*>(&n), 4);
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            float norm[3] = {0, 0, 1};
            f.write(reinterpret_cast<const char*>(norm), 12);
            for (int k = 0; k < 3; ++k) {
                size_t idx = mesh.indices[i + k] * 3;
                float v[3] = {
                    static_cast<float>(mesh.vertices[idx]),
                    static_cast<float>(mesh.vertices[idx + 1]),
                    static_cast<float>(mesh.vertices[idx + 2])
                };
                f.write(reinterpret_cast<const char*>(v), 12);
            }
            uint16_t attr = 0;
            f.write(reinterpret_cast<const char*>(&attr), 2);
        }
        return f.good();
    }
    std::ofstream f(filePath);
    if (!f) return false;
    f << "solid HydraCAD\n";
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        f << "  facet normal 0 0 1\n    outer loop\n";
        for (int k = 0; k < 3; ++k) {
            size_t idx = mesh.indices[i + k] * 3;
            f << "      vertex " << mesh.vertices[idx] << " " << mesh.vertices[idx+1] << " " << mesh.vertices[idx+2] << "\n";
        }
        f << "    endloop\n  endfacet\n";
    }
    f << "endsolid HydraCAD\n";
    return f.good();
}

bool writeStl(const topology::Solid& solid, const std::string& filePath, bool binary) {
    TriangleMesh mesh = triangulate(solid);
    return writeStlMesh(mesh, filePath, binary);
}

}  // namespace io
}  // namespace kernel
}  // namespace cad
