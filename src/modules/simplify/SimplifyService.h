#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

enum class SimplifyMode {
    ReplaceWithBoundingBox,
    ReplaceWithSimplifiedGeometry,
    RemoveInternalFeatures,
    RemoveSmallFeatures,
    CombineSimilarParts
};

enum class ReplacementType {
    BoundingBox,
    SimplifiedMesh,
    ConvexHull,
    Cylinder,
    Box
};

struct SimplifyRule {
    std::string name;
    SimplifyMode mode;
    double threshold{0.0};  // Size threshold for simplification
    bool apply_to_subassemblies{false};
    std::vector<std::string> excluded_parts;
};

struct SimplifiedComponent {
    std::string original_part_id;
    std::string simplified_part_id;
    ReplacementType replacement_type;
    double simplification_ratio{0.0};  // 0.0 = no simplification, 1.0 = maximum
    std::map<std::string, double> bounding_box;  // min_x, max_x, min_y, max_y, min_z, max_z
};

struct SimplifyRequest {
    std::string targetAssembly;
    std::string replacementType;
    SimplifyMode mode{SimplifyMode::ReplaceWithBoundingBox};
    std::vector<SimplifyRule> rules;
    double detail_threshold{1.0};  // Features smaller than this are simplified
    bool preserve_external_features{true};
    bool preserve_assembly_structure{true};
};

struct SimplifyResult {
    bool success{false};
    std::string message;
    std::string simplified_assembly_id;
    std::vector<SimplifiedComponent> simplified_components;
    int original_component_count{0};
    int simplified_component_count{0};
    double file_size_reduction{0.0};  // Percentage
    double performance_improvement{0.0};  // Percentage
};

class SimplifyService {
public:
    SimplifyResult simplify(const SimplifyRequest& request) const;
    SimplifyResult simplifyWithRules(const SimplifyRequest& request) const;
    SimplifyResult replaceWithBoundingBox(const std::string& assembly_id) const;
    SimplifyResult replaceWithSimplifiedGeometry(const std::string& assembly_id, ReplacementType type) const;
    SimplifyResult removeInternalFeatures(const std::string& assembly_id, double threshold) const;
    SimplifyResult removeSmallFeatures(const std::string& assembly_id, double threshold) const;
    SimplifyResult combineSimilarParts(const std::string& assembly_id) const;
    
    // Simplify queries
    std::vector<SimplifiedComponent> getSimplifiedComponents(const std::string& assembly_id) const;
    double getSimplificationRatio(const std::string& assembly_id) const;
    
    // Presets
    SimplifyRequest createPerformancePreset() const;
    SimplifyRequest createDetailPreset() const;
    SimplifyRequest createBalancedPreset() const;
    
private:
    std::map<std::string, SimplifyResult> simplified_assemblies_;
    
    SimplifiedComponent createSimplifiedComponent(const std::string& part_id, ReplacementType type) const;
    double calculateSimplificationRatio(const std::string& original_id, const std::string& simplified_id) const;
    bool shouldSimplify(const std::string& part_id, const SimplifyRule& rule) const;
};

}  // namespace modules
}  // namespace cad
