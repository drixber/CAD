#pragma once

#include <cstddef>
#include <string>

namespace cad {
namespace core {

enum class LodMode {
    Full,
    Simplified,
    BoundingBoxes
};

struct AssemblyLoadStats {
    std::size_t component_count{0};
    double load_seconds{0.0};
};

class AssemblyManager {
public:
    void setLodMode(LodMode mode);
    void setTargetFps(double fps);
    void setMaxComponents(std::size_t max_components);
    AssemblyLoadStats loadAssembly(const std::string& path);

private:
    LodMode lod_mode_{LodMode::Full};
    double target_fps_{30.0};
    std::size_t max_components_{1000};
};

}  // namespace core
}  // namespace cad
