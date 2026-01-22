#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "../Modeler/Assembly.h"

namespace cad {
namespace core {

struct CacheStats {
    std::size_t entries{0};
    std::size_t max_entries{0};
    std::size_t cache_hits{0};
    std::size_t cache_misses{0};
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
    LodMode applied_lod{LodMode::Full};
    std::size_t visible_components{0};
};

struct AssemblyLoadJob {
    std::string path;
    int progress{0};
};

struct CachedAssembly {
    Assembly assembly;
    std::size_t component_count{0};
    LodMode cached_lod{LodMode::Full};
    std::size_t access_count{0};
    double last_access_time{0.0};
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
    void enqueueLoad(const std::string& path);
    AssemblyLoadJob pollLoadProgress();
    LodMode recommendedLod() const;
    
    // Cache management
    void cacheAssembly(const std::string& path, const Assembly& assembly);
    bool getCachedAssembly(const std::string& path, Assembly& out_assembly) const;
    void clearCache();
    std::vector<std::string> getCachedPaths() const;
    
    // LOD filtering
    std::size_t getVisibleComponentCount(const Assembly& assembly, LodMode lod) const;
    std::vector<std::uint64_t> getVisibleComponentIds(const Assembly& assembly, LodMode lod) const;

private:
    void evictOldestCacheEntry();
    std::size_t calculateLodComponentLimit(LodMode lod) const;
    
    LodMode lod_mode_{LodMode::Full};
    double target_fps_{30.0};
    std::size_t max_components_{1000};
    std::size_t cache_limit_{200};
    bool background_loading_{true};
    std::deque<AssemblyLoadJob> load_queue_{};
    
    // Cache storage
    std::map<std::string, CachedAssembly> cache_;
    mutable std::size_t cache_hits_{0};
    mutable std::size_t cache_misses_{0};
    double current_time_{0.0};
};

}  // namespace core
}  // namespace cad
