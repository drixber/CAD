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
    cache_entries_ = cache_limit_ > 0 ? cache_limit_ / 2 : 0;
    stats.used_background_loading = background_loading_;
    return stats;
}

CacheStats AssemblyManager::cacheStats() const {
    CacheStats stats;
    stats.entries = cache_entries_;
    stats.max_entries = cache_limit_;
    return stats;
}

void AssemblyManager::setCacheLimit(std::size_t max_entries) {
    cache_limit_ = max_entries;
    if (cache_entries_ > cache_limit_) {
        cache_entries_ = cache_limit_;
    }
}

void AssemblyManager::enableBackgroundLoading(bool enabled) {
    background_loading_ = enabled;
}

}  // namespace core
}  // namespace cad
