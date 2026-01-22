#include "AssemblyManager.h"

#include <algorithm>
#include <chrono>

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
    AssemblyLoadStats stats;
    
    // Use adaptive LOD if enabled
    LodMode effective_lod = lod_mode_;
    if (adaptive_lod_) {
        effective_lod = adaptiveLodRecommendation();
    }
    
    // Try to get from cache first
    Assembly assembly;
    if (getCachedAssembly(path, assembly)) {
        cache_hits_++;
        stats.component_count = assembly.components().size();
        stats.applied_lod = effective_lod;
        stats.visible_components = getVisibleComponentCount(assembly, effective_lod);
        stats.load_seconds = 0.0;  // Cached, instant load
        stats.used_background_loading = false;
        stats.from_cache = true;
        stats.estimated_memory_mb = static_cast<double>(estimateAssemblyMemory(assembly)) / (1024.0 * 1024.0);
        return stats;
    }
    
    cache_misses_++;
    // Simulate loading (in real implementation, would load from file)
    stats.component_count = max_components_;
    stats.applied_lod = effective_lod;
    stats.visible_components = getVisibleComponentCount(assembly, effective_lod);
    stats.load_seconds = 0.1;  // Simulated load time
    stats.used_background_loading = background_loading_;
    stats.from_cache = false;
    stats.estimated_memory_mb = static_cast<double>(estimateAssemblyMemory(assembly)) / (1024.0 * 1024.0);
    
    return stats;
}

CacheStats AssemblyManager::cacheStats() const {
    CacheStats stats;
    stats.entries = cache_.size();
    stats.max_entries = cache_limit_;
    stats.cache_hits = cache_hits_;
    stats.cache_misses = cache_misses_;
    stats.evicted_entries = evicted_count_;
    stats.memory_usage_bytes = total_memory_usage_;
    
    std::size_t total_requests = cache_hits_ + cache_misses_;
    if (total_requests > 0) {
        stats.hit_rate = static_cast<double>(cache_hits_) / static_cast<double>(total_requests);
    }
    
    return stats;
}

void AssemblyManager::setCacheLimit(std::size_t max_entries) {
    cache_limit_ = max_entries;
    while (cache_.size() > cache_limit_) {
        evictOldestCacheEntry();
    }
}

void AssemblyManager::enableBackgroundLoading(bool enabled) {
    background_loading_ = enabled;
}

void AssemblyManager::enqueueLoad(const std::string& path) {
    AssemblyLoadJob job;
    job.path = path;
    job.progress = 0;
    load_queue_.push_back(job);
}

AssemblyLoadJob AssemblyManager::pollLoadProgress() {
    if (load_queue_.empty()) {
        return {};
    }
    AssemblyLoadJob& job = load_queue_.front();
    if (job.progress < 100) {
        job.progress += 25;
    }
    if (job.progress >= 100) {
        AssemblyLoadJob done = job;
        load_queue_.pop_front();
        return done;
    }
    return job;
}

LodMode AssemblyManager::recommendedLod() const {
    if (max_components_ >= 5000 || target_fps_ >= 60.0) {
        return LodMode::BoundingBoxes;
    }
    if (max_components_ >= 2000 || target_fps_ >= 45.0) {
        return LodMode::Simplified;
    }
    return LodMode::Full;
}

void AssemblyManager::cacheAssembly(const std::string& path, const Assembly& assembly) {
    // Evict if cache is full (by entries or memory)
    while ((cache_.size() >= cache_limit_ || 
            total_memory_usage_ >= memory_limit_mb_ * 1024 * 1024) && 
           !cache_.empty()) {
        evictLowPriorityEntries();
    }
    
    CachedAssembly cached;
    cached.assembly = assembly;
    cached.component_count = assembly.components().size();
    cached.cached_lod = lod_mode_;
    cached.access_count = 1;
    cached.estimated_memory_bytes = estimateAssemblyMemory(assembly);
    
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    cached.last_access_time = std::chrono::duration<double>(duration).count();
    cached.priority_score = calculatePriorityScore(cached);
    
    // Update memory usage
    auto it = cache_.find(path);
    if (it != cache_.end()) {
        total_memory_usage_ -= it->second.estimated_memory_bytes;
    }
    
    cache_[path] = cached;
    total_memory_usage_ += cached.estimated_memory_bytes;
    current_time_ = cached.last_access_time;
}

bool AssemblyManager::getCachedAssembly(const std::string& path, Assembly& out_assembly) const {
    auto it = cache_.find(path);
    if (it != cache_.end()) {
        // Update access info
        CachedAssembly& cached = const_cast<CachedAssembly&>(it->second);
        cached.access_count++;
        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        cached.last_access_time = std::chrono::duration<double>(duration).count();
        cached.priority_score = calculatePriorityScore(cached);
        
        out_assembly = cached.assembly;
        return true;
    }
    return false;
}

void AssemblyManager::clearCache() {
    cache_.clear();
    cache_hits_ = 0;
    cache_misses_ = 0;
    evicted_count_ = 0;
    total_memory_usage_ = 0;
}

std::vector<std::string> AssemblyManager::getCachedPaths() const {
    std::vector<std::string> paths;
    paths.reserve(cache_.size());
    for (const auto& [path, cached] : cache_) {
        paths.push_back(path);
    }
    return paths;
}

std::size_t AssemblyManager::getVisibleComponentCount(const Assembly& assembly, LodMode lod) const {
    std::size_t total_components = assembly.components().size();
    std::size_t limit = calculateLodComponentLimit(lod);
    return std::min(total_components, limit);
}

std::vector<std::uint64_t> AssemblyManager::getVisibleComponentIds(const Assembly& assembly, LodMode lod) const {
    std::vector<std::uint64_t> visible_ids;
    const auto& components = assembly.components();
    std::size_t limit = calculateLodComponentLimit(lod);
    
    visible_ids.reserve(std::min(components.size(), limit));
    for (std::size_t i = 0; i < components.size() && i < limit; ++i) {
        visible_ids.push_back(components[i].id);
    }
    
    return visible_ids;
}

void AssemblyManager::evictOldestCacheEntry() {
    if (cache_.empty()) {
        return;
    }
    
    // Find entry with oldest access time
    auto oldest_it = cache_.begin();
    for (auto it = cache_.begin(); it != cache_.end(); ++it) {
        if (it->second.last_access_time < oldest_it->second.last_access_time) {
            oldest_it = it;
        }
    }
    
    total_memory_usage_ -= oldest_it->second.estimated_memory_bytes;
    cache_.erase(oldest_it);
    evicted_count_++;
}

void AssemblyManager::evictLowPriorityEntries() {
    if (cache_.empty()) {
        return;
    }
    
    // Find entry with lowest priority score
    auto lowest_it = cache_.begin();
    for (auto it = cache_.begin(); it != cache_.end(); ++it) {
        if (it->second.priority_score < lowest_it->second.priority_score) {
            lowest_it = it;
        }
    }
    
    total_memory_usage_ -= lowest_it->second.estimated_memory_bytes;
    cache_.erase(lowest_it);
    evicted_count_++;
}

std::size_t AssemblyManager::calculateLodComponentLimit(LodMode lod) const {
    switch (lod) {
        case LodMode::Full:
            return max_components_;
        case LodMode::Simplified:
            return std::min(max_components_, static_cast<std::size_t>(max_components_ * 0.5));
        case LodMode::BoundingBoxes:
            return std::min(max_components_, static_cast<std::size_t>(max_components_ * 0.1));
        default:
            return max_components_;
    }
}

void AssemblyManager::setMemoryLimit(std::size_t max_memory_mb) {
    memory_limit_mb_ = max_memory_mb;
    while (total_memory_usage_ >= memory_limit_mb_ * 1024 * 1024 && !cache_.empty()) {
        evictLowPriorityEntries();
    }
}

void AssemblyManager::enableAdaptiveLod(bool enabled) {
    adaptive_lod_ = enabled;
}

void AssemblyManager::preloadAssembly(const std::string& path) {
    // Preload assembly in background
    enqueueLoad(path);
}

double AssemblyManager::getCacheHitRate() const {
    std::size_t total_requests = cache_hits_ + cache_misses_;
    if (total_requests == 0) {
        return 0.0;
    }
    return static_cast<double>(cache_hits_) / static_cast<double>(total_requests);
}

std::size_t AssemblyManager::getMemoryUsage() const {
    return total_memory_usage_;
}

std::size_t AssemblyManager::estimateAssemblyMemory(const Assembly& assembly) const {
    // Estimate memory usage: base size + component overhead
    std::size_t base_size = 1024;  // Base assembly structure
    std::size_t component_size = 256;  // Per component overhead
    return base_size + (assembly.components().size() * component_size);
}

double AssemblyManager::calculatePriorityScore(const CachedAssembly& cached) const {
    // Priority = access_count * 0.6 + recency * 0.4
    // Higher score = higher priority (less likely to be evicted)
    double recency_factor = (current_time_ - cached.last_access_time + 1.0) / 1000.0;  // Normalize
    double access_factor = static_cast<double>(cached.access_count);
    return (access_factor * 0.6) + (recency_factor * 0.4);
}

LodMode AssemblyManager::adaptiveLodRecommendation() const {
    // Adaptive LOD based on measured performance and component count
    if (measured_fps_ < target_fps_ * 0.8) {
        // Performance is below target, use more aggressive LOD
        if (max_components_ >= 3000) {
            return LodMode::BoundingBoxes;
        }
        if (max_components_ >= 1500) {
            return LodMode::Simplified;
        }
    }
    
    // Use standard recommendation
    return recommendedLod();
}

}  // namespace core
}  // namespace cad
