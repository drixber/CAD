#include "AssemblyManager.h"

namespace cad {
namespace core {

void AssemblyManager::setLodMode(LodMode mode) {
    lod_mode_ = mode;
}

void AssemblyManager::setTargetFps(double fps) {
    target_fps_ = fps <= 0.0 ? 30.0 : fps;
}

void AssemblyManager::setMaxComponents(std::size_t max_components) {
    max_components_ = max_components;
}

AssemblyLoadStats AssemblyManager::loadAssembly(const std::string& path) {
    (void)path;
    AssemblyLoadStats stats;
    stats.component_count = max_components_;
    stats.load_seconds = 0.0;
    return stats;
}

}  // namespace core
}  // namespace cad
