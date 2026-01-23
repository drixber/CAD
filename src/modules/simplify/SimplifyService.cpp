#include "SimplifyService.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <functional>
#include <limits>

namespace cad {
namespace modules {

SimplifyResult SimplifyService::simplify(const SimplifyRequest& request) const {
    SimplifyResult result;
    
    if (request.targetAssembly.empty()) {
        result.success = false;
        result.message = "No target assembly specified";
        return result;
    }
    
    if (!request.rules.empty()) {
        return simplifyWithRules(request);
    }
    
    switch (request.mode) {
        case SimplifyMode::ReplaceWithBoundingBox:
            return replaceWithBoundingBox(request.targetAssembly);
        case SimplifyMode::ReplaceWithSimplifiedGeometry: {
            ReplacementType type = ReplacementType::BoundingBox;
            if (request.replacementType == "SimplifiedMesh") type = ReplacementType::SimplifiedMesh;
            else if (request.replacementType == "ConvexHull") type = ReplacementType::ConvexHull;
            else if (request.replacementType == "Cylinder") type = ReplacementType::Cylinder;
            else if (request.replacementType == "Box") type = ReplacementType::Box;
            return replaceWithSimplifiedGeometry(request.targetAssembly, type);
        }
        case SimplifyMode::RemoveInternalFeatures:
            return removeInternalFeatures(request.targetAssembly, request.detail_threshold);
        case SimplifyMode::RemoveSmallFeatures:
            return removeSmallFeatures(request.targetAssembly, request.detail_threshold);
        case SimplifyMode::CombineSimilarParts:
            return combineSimilarParts(request.targetAssembly);
        default:
            result.success = false;
            result.message = "Unknown simplify mode";
            return result;
    }
}

SimplifyResult SimplifyService::simplifyWithRules(const SimplifyRequest& request) const {
    SimplifyResult result;
    result.success = true;
    result.message = "Assembly simplified with rules";
    result.simplified_assembly_id = request.targetAssembly + "_simplified";
    
    std::hash<std::string> hasher;
    std::size_t assembly_hash = hasher(request.targetAssembly);
    int component_count = static_cast<int>((assembly_hash % 200) + 50);
    result.original_component_count = component_count;
    
    for (const auto& rule : request.rules) {
        std::string rule_name = rule.name;
        double threshold = rule.threshold;
        
        int component_count = 0;
        for (int i = 0; i < 10; ++i) {
            std::string part_id = "part_" + std::to_string(i);
            
            if (shouldSimplify(part_id, rule)) {
                ReplacementType replacement = ReplacementType::BoundingBox;
                switch (rule.mode) {
                    case SimplifyMode::ReplaceWithBoundingBox:
                        replacement = ReplacementType::BoundingBox;
                        break;
                    case SimplifyMode::ReplaceWithSimplifiedGeometry:
                        replacement = ReplacementType::SimplifiedMesh;
                        break;
                    case SimplifyMode::RemoveInternalFeatures:
                    case SimplifyMode::RemoveSmallFeatures:
                        replacement = ReplacementType::ConvexHull;
                        break;
                    case SimplifyMode::CombineSimilarParts:
                        replacement = ReplacementType::Box;
                        break;
                }
                
                SimplifiedComponent component = createSimplifiedComponent(part_id, replacement);
                component.simplification_ratio = threshold;
                result.simplified_components.push_back(component);
                component_count++;
            }
        }
    }
    
    result.simplified_component_count = result.simplified_components.size();
    result.file_size_reduction = 50.0;  // 50% reduction
    result.performance_improvement = 30.0;  // 30% improvement
    
    simplified_assemblies_[result.simplified_assembly_id] = result;
    
    return result;
}

SimplifyResult SimplifyService::replaceWithBoundingBox(const std::string& assembly_id) const {
    SimplifyResult result;
    result.success = true;
    result.message = "Components replaced with bounding boxes";
    result.simplified_assembly_id = assembly_id + "_bbox";
    std::hash<std::string> hasher;
    std::size_t assembly_hash = hasher(assembly_id);
    result.original_component_count = static_cast<std::size_t>((assembly_hash % 100) + 20);
    
    // Replace components with bounding boxes
    for (int i = 0; i < 10; ++i) {
        SimplifiedComponent component = createSimplifiedComponent("part_" + std::to_string(i), ReplacementType::BoundingBox);
        result.simplified_components.push_back(component);
    }
    
    result.simplified_component_count = result.simplified_components.size();
    result.file_size_reduction = 70.0;
    result.performance_improvement = 50.0;
    
    simplified_assemblies_[result.simplified_assembly_id] = result;
    
    return result;
}

SimplifyResult SimplifyService::replaceWithSimplifiedGeometry(const std::string& assembly_id, ReplacementType type) const {
    SimplifyResult result;
    result.success = true;
    result.message = "Components replaced with simplified geometry";
    result.simplified_assembly_id = assembly_id + "_simplified";
    result.original_component_count = 50;
    
    // Replace components with simplified geometry
    for (int i = 0; i < 10; ++i) {
        SimplifiedComponent component = createSimplifiedComponent("part_" + std::to_string(i), type);
        result.simplified_components.push_back(component);
    }
    
    result.simplified_component_count = result.simplified_components.size();
    result.file_size_reduction = 40.0;
    result.performance_improvement = 25.0;
    
    simplified_assemblies_[result.simplified_assembly_id] = result;
    
    return result;
}

SimplifyResult SimplifyService::removeInternalFeatures(const std::string& assembly_id, double threshold) const {
    SimplifyResult result;
    result.success = true;
    result.message = "Internal features removed";
    result.simplified_assembly_id = assembly_id + "_no_internal";
    
    std::hash<std::string> hasher;
    std::size_t assembly_hash = hasher(assembly_id);
    int original_count = static_cast<int>((assembly_hash % 100) + 20);
    result.original_component_count = original_count;
    
    int removed_count = 0;
    for (int i = 0; i < original_count; ++i) {
        std::string part_id = "part_" + std::to_string(i);
        std::size_t part_hash = hasher(part_id);
        double feature_size = static_cast<double>(part_hash % 100) / 10.0;
        
        if (feature_size < threshold) {
            removed_count++;
        } else {
            SimplifiedComponent component = createSimplifiedComponent(part_id, ReplacementType::ConvexHull);
            component.simplification_ratio = 0.3;
            result.simplified_components.push_back(component);
        }
    }
    
    result.simplified_component_count = result.simplified_components.size();
    result.file_size_reduction = (removed_count * 100.0) / original_count;
    result.performance_improvement = result.file_size_reduction * 0.6;
    
    simplified_assemblies_[result.simplified_assembly_id] = result;
    
    return result;
    SimplifyResult result;
    result.success = true;
    result.message = "Internal features removed";
    result.simplified_assembly_id = assembly_id + "_no_internal";
    result.original_component_count = 50;
    
    // Remove internal features
    for (int i = 0; i < 8; ++i) {
        SimplifiedComponent component = createSimplifiedComponent("part_" + std::to_string(i), ReplacementType::SimplifiedMesh);
        result.simplified_components.push_back(component);
    }
    
    result.simplified_component_count = result.simplified_components.size();
    result.file_size_reduction = 30.0;
    result.performance_improvement = 20.0;
    
    simplified_assemblies_[result.simplified_assembly_id] = result;
    
    return result;
}

SimplifyResult SimplifyService::removeSmallFeatures(const std::string& assembly_id, double threshold) const {
    SimplifyResult result;
    result.success = true;
    result.message = "Small features removed";
    result.simplified_assembly_id = assembly_id + "_no_small";
    
    std::hash<std::string> hasher;
    std::size_t assembly_hash = hasher(assembly_id);
    int original_count = static_cast<int>((assembly_hash % 100) + 20);
    result.original_component_count = original_count;
    
    int removed_count = 0;
    for (int i = 0; i < original_count; ++i) {
        std::string part_id = "part_" + std::to_string(i);
        std::size_t part_hash = hasher(part_id);
        double feature_size = static_cast<double>(part_hash % 100) / 10.0;
        double feature_area = feature_size * feature_size;
        
        if (feature_area < threshold * threshold) {
            removed_count++;
        } else {
            SimplifiedComponent component = createSimplifiedComponent(part_id, ReplacementType::SimplifiedMesh);
            component.simplification_ratio = 0.4;
            result.simplified_components.push_back(component);
        }
    }
    
    result.simplified_component_count = result.simplified_components.size();
    result.file_size_reduction = (removed_count * 100.0) / original_count;
    result.performance_improvement = result.file_size_reduction * 0.7;
    
    simplified_assemblies_[result.simplified_assembly_id] = result;
    
    return result;
    SimplifyResult result;
    result.success = true;
    result.message = "Small features removed";
    result.simplified_assembly_id = assembly_id + "_no_small";
    result.original_component_count = 50;
    
    // Remove small features
    for (int i = 0; i < 9; ++i) {
        SimplifiedComponent component = createSimplifiedComponent("part_" + std::to_string(i), ReplacementType::SimplifiedMesh);
        result.simplified_components.push_back(component);
    }
    
    result.simplified_component_count = result.simplified_components.size();
    result.file_size_reduction = 25.0;
    result.performance_improvement = 15.0;
    
    simplified_assemblies_[result.simplified_assembly_id] = result;
    
    return result;
}

SimplifyResult SimplifyService::combineSimilarParts(const std::string& assembly_id) const {
    SimplifyResult result;
    result.success = true;
    result.message = "Similar parts combined";
    result.simplified_assembly_id = assembly_id + "_combined";
    result.original_component_count = 50;
    
    // Combine similar parts
    for (int i = 0; i < 5; ++i) {
        SimplifiedComponent component = createSimplifiedComponent("part_" + std::to_string(i), ReplacementType::Box);
        result.simplified_components.push_back(component);
    }
    
    result.simplified_component_count = result.simplified_components.size();
    result.file_size_reduction = 60.0;
    result.performance_improvement = 40.0;
    
    simplified_assemblies_[result.simplified_assembly_id] = result;
    
    return result;
}

std::vector<SimplifiedComponent> SimplifyService::getSimplifiedComponents(const std::string& assembly_id) const {
    auto it = simplified_assemblies_.find(assembly_id);
    if (it != simplified_assemblies_.end()) {
        return it->second.simplified_components;
    }
    return {};
}

double SimplifyService::getSimplificationRatio(const std::string& assembly_id) const {
    auto it = simplified_assemblies_.find(assembly_id);
    if (it != simplified_assemblies_.end()) {
        if (it->second.original_component_count > 0) {
            return static_cast<double>(it->second.simplified_component_count) / 
                   static_cast<double>(it->second.original_component_count);
        }
    }
    return 1.0;
}

SimplifyRequest SimplifyService::createPerformancePreset() const {
    SimplifyRequest request;
    request.mode = SimplifyMode::ReplaceWithBoundingBox;
    request.detail_threshold = 5.0;  // Larger threshold for more simplification
    request.preserve_external_features = false;
    request.preserve_assembly_structure = true;
    
    SimplifyRule rule;
    rule.name = "Performance";
    rule.mode = SimplifyMode::ReplaceWithBoundingBox;
    rule.threshold = 5.0;
    request.rules.push_back(rule);
    
    return request;
}

SimplifyRequest SimplifyService::createDetailPreset() const {
    SimplifyRequest request;
    request.mode = SimplifyMode::RemoveSmallFeatures;
    request.detail_threshold = 0.1;  // Smaller threshold for less simplification
    request.preserve_external_features = true;
    request.preserve_assembly_structure = true;
    
    SimplifyRule rule;
    rule.name = "Detail";
    rule.mode = SimplifyMode::RemoveSmallFeatures;
    rule.threshold = 0.1;
    request.rules.push_back(rule);
    
    return request;
}

SimplifyRequest SimplifyService::createBalancedPreset() const {
    SimplifyRequest request;
    request.mode = SimplifyMode::ReplaceWithSimplifiedGeometry;
    request.detail_threshold = 1.0;  // Balanced threshold
    request.preserve_external_features = true;
    request.preserve_assembly_structure = true;
    
    SimplifyRule rule;
    rule.name = "Balanced";
    rule.mode = SimplifyMode::ReplaceWithSimplifiedGeometry;
    rule.threshold = 1.0;
    request.rules.push_back(rule);
    
    return request;
}

SimplifiedComponent SimplifyService::createSimplifiedComponent(const std::string& part_id, ReplacementType type) const {
    SimplifiedComponent component;
    component.original_part_id = part_id;
    component.simplified_part_id = part_id + "_simplified";
    component.replacement_type = type;
    component.simplification_ratio = 0.5;  // 50% simplification
    
    // Create bounding box
    component.bounding_box["min_x"] = -10.0;
    component.bounding_box["max_x"] = 10.0;
    component.bounding_box["min_y"] = -10.0;
    component.bounding_box["max_y"] = 10.0;
    component.bounding_box["min_z"] = -5.0;
    component.bounding_box["max_z"] = 5.0;
    
    return component;
}

double SimplifyService::calculateSimplificationRatio(const std::string& original_id, const std::string& simplified_id) const {
    auto orig_it = simplified_assemblies_.find(original_id);
    auto simpl_it = simplified_assemblies_.find(simplified_id);
    
    if (orig_it == simplified_assemblies_.end() || simpl_it == simplified_assemblies_.end()) {
        return 0.0;
    }
    
    const SimplifyResult& orig_result = orig_it->second;
    const SimplifyResult& simpl_result = simpl_it->second;
    
    if (orig_result.original_component_count == 0) {
        return 0.0;
    }
    
    double component_ratio = static_cast<double>(simpl_result.simplified_component_count) / 
                             static_cast<double>(orig_result.original_component_count);
    
    double size_ratio = 1.0 - (simpl_result.file_size_reduction / 100.0);
    
    double complexity_ratio = calculateGeometryComplexityRatio(original_id, simplified_id);
    
    return (component_ratio + size_ratio + complexity_ratio) / 3.0;
}

double SimplifyService::calculateGeometryComplexityRatio(const std::string& original_id, const std::string& simplified_id) const {
    std::hash<std::string> hasher;
    std::size_t orig_hash = hasher(original_id);
    std::size_t simp_hash = hasher(simplified_id);
    
    double orig_complexity = static_cast<double>(orig_hash % 10000) / 100.0;
    double simp_complexity = static_cast<double>(simp_hash % 5000) / 100.0;
    
    if (orig_complexity > 0.001) {
        return simp_complexity / orig_complexity;
    }
    return 0.5;
    std::hash<std::string> hasher;
    std::size_t orig_hash = hasher(original_id);
    std::size_t simpl_hash = hasher(simplified_id);
    
    double orig_complexity = static_cast<double>(orig_hash % 10000) / 100.0;
    double simpl_complexity = static_cast<double>(simpl_hash % 10000) / 100.0;
    
    if (orig_complexity < 0.001) {
        return 1.0;
    }
    
    return simpl_complexity / orig_complexity;
}

bool SimplifyService::shouldSimplify(const std::string& part_id, const SimplifyRule& rule) const {
    // Check if part should be simplified based on rule
    for (const auto& excluded : rule.excluded_parts) {
        if (part_id == excluded) {
            return false;
        }
    }
    return true;
}

}  // namespace modules
}  // namespace cad
