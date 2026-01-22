#include "InterferenceChecker.h"

#include <algorithm>
#include <sstream>

namespace cad {
namespace core {

InterferenceResult InterferenceChecker::check(const std::string& assembly_id) const {
    InterferenceResult result;
    if (assembly_id.empty()) {
        result.message = "No assembly loaded";
        return result;
    }
    result.message = "No interference detected";
    return result;
}

InterferenceResult InterferenceChecker::checkAssembly(const Assembly& assembly) const {
    InterferenceResult result;
    result.has_interference = false;
    result.overlap_count = 0;
    
    const auto& components = assembly.components();
    if (components.size() < 2) {
        result.message = "Assembly has fewer than 2 components";
        return result;
    }
    
    // Calculate bounding boxes for all components
    std::vector<BoundingBox> boxes;
    boxes.reserve(components.size());
    for (const auto& component : components) {
        boxes.push_back(estimateBoundingBox(component.part, component.transform));
    }
    
    // Check all pairs for overlaps
    for (std::size_t i = 0; i < components.size(); ++i) {
        for (std::size_t j = i + 1; j < components.size(); ++j) {
            if (boxesOverlap(boxes[i], boxes[j])) {
                InterferencePair pair;
                pair.component_a_id = components[i].id;
                pair.component_b_id = components[j].id;
                pair.part_a_name = components[i].part.name();
                pair.part_b_name = components[j].part.name();
                pair.overlap_volume = calculateOverlapVolume(boxes[i], boxes[j]);
                
                result.interference_pairs.push_back(pair);
                result.overlap_count++;
                result.has_interference = true;
            }
        }
    }
    
    // Build message
    if (result.has_interference) {
        std::ostringstream oss;
        oss << "Interference detected: " << result.overlap_count << " overlap(s)";
        if (!result.interference_pairs.empty()) {
            oss << " (e.g., " << result.interference_pairs[0].part_a_name 
                << " <-> " << result.interference_pairs[0].part_b_name << ")";
        }
        result.message = oss.str();
    } else {
        result.message = "No interference detected";
    }
    
    return result;
}

BoundingBox InterferenceChecker::estimateBoundingBox(const Part& part, const Transform& transform) const {
    // Simple heuristic: estimate bounding box based on part name and transform
    // In a real implementation, this would query actual geometry bounds
    BoundingBox box;
    
    // Default size based on part name (heuristic)
    double default_size = 10.0;
    if (part.name().find("Plate") != std::string::npos) {
        default_size = 5.0;  // Plates are typically thinner
    } else if (part.name().find("Bracket") != std::string::npos) {
        default_size = 8.0;
    }
    
    // Apply transform
    box.min_x = transform.tx - default_size / 2.0;
    box.max_x = transform.tx + default_size / 2.0;
    box.min_y = transform.ty - default_size / 2.0;
    box.max_y = transform.ty + default_size / 2.0;
    box.min_z = transform.tz - default_size / 2.0;
    box.max_z = transform.tz + default_size / 2.0;
    
    return box;
}

bool InterferenceChecker::boxesOverlap(const BoundingBox& a, const BoundingBox& b) const {
    // Check if boxes overlap in all three dimensions
    bool x_overlap = (a.min_x <= b.max_x) && (a.max_x >= b.min_x);
    bool y_overlap = (a.min_y <= b.max_y) && (a.max_y >= b.min_y);
    bool z_overlap = (a.min_z <= b.max_z) && (a.max_z >= b.min_z);
    
    return x_overlap && y_overlap && z_overlap;
}

double InterferenceChecker::calculateOverlapVolume(const BoundingBox& a, const BoundingBox& b) const {
    if (!boxesOverlap(a, b)) {
        return 0.0;
    }
    
    // Calculate overlap dimensions
    double overlap_x = std::min(a.max_x, b.max_x) - std::max(a.min_x, b.min_x);
    double overlap_y = std::min(a.max_y, b.max_y) - std::max(a.min_y, b.min_y);
    double overlap_z = std::min(a.max_z, b.max_z) - std::max(a.min_z, b.min_z);
    
    // Ensure non-negative
    overlap_x = std::max(0.0, overlap_x);
    overlap_y = std::max(0.0, overlap_y);
    overlap_z = std::max(0.0, overlap_z);
    
    return overlap_x * overlap_y * overlap_z;
}

}  // namespace core
}  // namespace cad
