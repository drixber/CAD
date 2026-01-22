#pragma once

#include <atomic>
#include <cstddef>
#include <deque>
#include <future>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../Modeler/Assembly.h"

namespace cad {
namespace core {

struct CacheStats {
    std::size_t entries{0};
    std::size_t max_entries{0};
    std::size_t cache_hits{0};
    std::size_t cache_misses{0};
    double hit_rate{0.0};
    std::size_t memory_usage_bytes{0};
    std::size_t evicted_entries{0};
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
    double estimated_memory_mb{0.0};
    bool from_cache{false};
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
    std::size_t estimated_memory_bytes{0};
    double priority_score{0.0};  // Combined score for eviction
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
    
    // Performance optimization
    void setMemoryLimit(std::size_t max_memory_mb);
    void enableAdaptiveLod(bool enabled);
    void preloadAssembly(const std::string& path);
    double getCacheHitRate() const;
    std::size_t getMemoryUsage() const;
    
    // Multi-threaded loading
    void enableMultiThreadedLoading(bool enabled);
    void setThreadPoolSize(std::size_t thread_count);
    std::future<AssemblyLoadStats> loadAssemblyAsync(const std::string& path);
    void waitForLoadCompletion();

private:
    void evictOldestCacheEntry();
    void evictLowPriorityEntries();
    std::size_t calculateLodComponentLimit(LodMode lod) const;
    std::size_t estimateAssemblyMemory(const Assembly& assembly) const;
    double calculatePriorityScore(const CachedAssembly& cached) const;
    LodMode adaptiveLodRecommendation() const;
    
    LodMode lod_mode_{LodMode::Full};
    double target_fps_{30.0};
    std::size_t max_components_{1000};
    std::size_t cache_limit_{200};
    std::size_t memory_limit_mb_{1024};  // 1GB default
    bool background_loading_{true};
    bool adaptive_lod_{true};
    bool multi_threaded_loading_{false};
    std::size_t thread_pool_size_{4};
    std::deque<AssemblyLoadJob> load_queue_{};
    std::vector<std::future<AssemblyLoadStats>> active_loads_{};
    std::mutex cache_mutex_;
    std::mutex load_mutex_;
    
    // Cache storage
    std::map<std::string, CachedAssembly> cache_;
    mutable std::size_t cache_hits_{0};
    mutable std::size_t cache_misses_{0};
    mutable std::size_t evicted_count_{0};
    mutable std::size_t total_memory_usage_{0};
    double current_time_{0.0};
    double last_performance_check_{0.0};
    double measured_fps_{30.0};
};

}  // namespace core
}  // namespace cad
