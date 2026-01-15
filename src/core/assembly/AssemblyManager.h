#pragma once

#include <cstddef>
#include <string>

namespace cad {
namespace core {

struct CacheStats {
    std::size_t entries{0};
    std::size_t max_entries{0};
};

enum class LodMode {
    Full,
    Simplified,
    BoundingBoxes
};

struct AssemblyLoadStats {
    std::size_t component_count{0};
    double load_seconds{0.0};
    bool used_background_loading{false};
};

class AssemblyManager {
public:
    void setLodMode(LodMode mode);
    void setTargetFps(double fps);
    void setMaxComponents(std::size_t max_components);
    AssemblyLoadStats loadAssembly(const std::string& path);
    CacheStats cacheStats() const;
    void setCacheLimit(std::size_t max_entries);
    void enableBackgroundLoading(bool enabled);

private:
    LodMode lod_mode_{LodMode::Full};
    double target_fps_{30.0};
    std::size_t max_components_{1000};
    std::size_t cache_entries_{0};
    std::size_t cache_limit_{200};
    bool background_loading_{true};
};

}  // namespace core
}  // namespace cad
