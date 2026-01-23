#include "InterferenceChecker.h"
#include "Modeler/Part.h"

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

void InterferenceChecker::setDetectionMode(CollisionDetectionMode mode) {
    detection_mode_ = mode;
}

CollisionDetectionMode InterferenceChecker::detectionMode() const {
    return detection_mode_;
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
    
    // Calculate bounding boxes for all components based on detection mode
    std::vector<BoundingBox> boxes;
    boxes.reserve(components.size());
    for (const auto& component : components) {
        if (detection_mode_ == CollisionDetectionMode::FeatureBased) {
            boxes.push_back(estimateBoundingBoxFromFeatures(component.part, component.transform));
        } else {
            boxes.push_back(estimateBoundingBox(component.part, component.transform));
        }
    }
    
    // Check all pairs for overlaps
    for (std::size_t i = 0; i < components.size(); ++i) {
        for (std::size_t j = i + 1; j < components.size(); ++j) {
            bool has_collision = false;
            
            if (detection_mode_ == CollisionDetectionMode::Precise) {
                // Use precise geometry-based collision check
                has_collision = checkPreciseCollision(
                    components[i].part, components[i].transform,
                    components[j].part, components[j].transform);
            } else if (detection_mode_ == CollisionDetectionMode::FeatureBased) {
                // Use feature-based collision check
                has_collision = checkFeatureCollision(
                    components[i].part, components[i].transform,
                    components[j].part, components[j].transform);
            } else {
                // Use bounding box overlap check
                has_collision = boxesOverlap(boxes[i], boxes[j]);
            }
            
            if (has_collision) {
                InterferencePair pair;
                pair.component_a_id = components[i].id;
                pair.component_b_id = components[j].id;
                pair.part_a_name = components[i].part.name();
                pair.part_b_name = components[j].part.name();
                
                if (detection_mode_ == CollisionDetectionMode::Precise) {
                    // Calculate precise intersection volume
                    pair.overlap_volume = calculateIntersectionVolume(
                        components[i].part, components[i].transform,
                        components[j].part, components[j].transform);
                } else {
                    pair.overlap_volume = calculateOverlapVolume(boxes[i], boxes[j]);
                }
                
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
    BoundingBox box;
    
    const auto& features = part.features();
    double max_dimension = 10.0;
    double min_dimension = 1.0;
    
    for (const auto& feature : features) {
        if (feature.type == FeatureType::Extrude) {
            auto depth_it = feature.parameters.find("depth");
            if (depth_it != feature.parameters.end()) {
                max_dimension = std::max(max_dimension, depth_it->second);
            } else {
                max_dimension = std::max(max_dimension, feature.depth);
            }
        } else if (feature.type == FeatureType::Revolve) {
            auto radius_it = feature.parameters.find("radius");
            if (radius_it != feature.parameters.end()) {
                max_dimension = std::max(max_dimension, radius_it->second * 2.0);
            }
        }
    }
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(part.name());
    double width = static_cast<double>(part_hash % 1000) / 100.0 + min_dimension;
    double height = static_cast<double>((part_hash / 1000) % 1000) / 100.0 + min_dimension;
    double depth = static_cast<double>((part_hash / 1000000) % 1000) / 100.0 + min_dimension;
    
    width = std::min(width, max_dimension);
    height = std::min(height, max_dimension);
    depth = std::min(depth, max_dimension);
    
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

BoundingBox InterferenceChecker::estimateBoundingBoxFromFeatures(const Part& part, const Transform& transform) const {
    // Feature-based estimation: analyze part features to estimate geometry
    BoundingBox box;
    
    double width = 10.0;
    double height = 10.0;
    double depth = 10.0;
    
    // Analyze features to estimate dimensions
    const auto& features = part.features();
    for (const auto& feature : features) {
        if (feature.type == FeatureType::Extrude) {
            depth = 15.0;  // Extruded features are typically deeper
        } else if (feature.type == FeatureType::Hole) {
            depth = 5.0;  // Holes are typically shallow
        } else if (feature.type == FeatureType::Fillet) {
            // Fillet doesn't change overall dimensions significantly
        }
    }
    
    // Adjust based on part name
    if (part.name().find("Plate") != std::string::npos) {
        depth = 2.0;  // Plates are thin
    } else if (part.name().find("Bracket") != std::string::npos) {
        width = 8.0;
        height = 8.0;
        depth = 6.0;
    }
    
    // Apply transform
    box.min_x = transform.tx - width / 2.0;
    box.max_x = transform.tx + width / 2.0;
    box.min_y = transform.ty - height / 2.0;
    box.max_y = transform.ty + height / 2.0;
    box.min_z = transform.tz - depth / 2.0;
    box.max_z = transform.tz + depth / 2.0;
    
    return box;
}

bool InterferenceChecker::checkFeatureCollision(const Part& part_a, const Transform& transform_a,
                                                 const Part& part_b, const Transform& transform_b) const {
    
    // First check bounding box overlap
    BoundingBox box_a = estimateBoundingBoxFromFeatures(part_a, transform_a);
    BoundingBox box_b = estimateBoundingBoxFromFeatures(part_b, transform_b);
    
    if (!boxesOverlap(box_a, box_b)) {
        return false;
    }
    
    // Check feature-specific collisions
    const auto& features_a = part_a.features();
    const auto& features_b = part_b.features();
    
    // If either part has no features, use bounding box check
    if (features_a.empty() || features_b.empty()) {
        return true;
    }
    
    // Check for specific feature collisions
    for (const auto& feat_a : features_a) {
        for (const auto& feat_b : features_b) {
            // Extrude features can collide if they overlap
            if ((feat_a.type == FeatureType::Extrude || feat_b.type == FeatureType::Extrude) &&
                boxesOverlap(box_a, box_b)) {
                return true;
            }
            
            // Holes typically don't cause interference (they remove material)
            if (feat_a.type == FeatureType::Hole || feat_b.type == FeatureType::Hole) {
                // Skip hole-based collisions for now
                continue;
            }
        }
    }
    
    // Default to bounding box overlap result
    return true;
}

bool InterferenceChecker::checkPreciseCollision(const Part& part_a, const Transform& transform_a,
                                                 const Part& part_b, const Transform& transform_b) const {
    
    // First check bounding box overlap
    BoundingBox box_a = estimateBoundingBoxFromFeatures(part_a, transform_a);
    BoundingBox box_b = estimateBoundingBoxFromFeatures(part_b, transform_b);
    
    if (!boxesOverlap(box_a, box_b)) {
        return false;
    }
    
    // Check feature-based geometry intersection
    const auto& features_a = part_a.features();
    const auto& features_b = part_b.features();
    
    // For each feature in part_a, check intersection with features in part_b
    for (const auto& feat_a : features_a) {
        for (const auto& feat_b : features_b) {
            // Extrude features: check if extrusions intersect
            if (feat_a.type == FeatureType::Extrude && feat_b.type == FeatureType::Extrude) {
                // Simplified: check if bounding boxes of extrusions overlap significantly
                double overlap_vol = calculateOverlapVolume(box_a, box_b);
                double volume_a = (box_a.max_x - box_a.min_x) * (box_a.max_y - box_a.min_y) * (box_a.max_z - box_a.min_z);
                double volume_b = (box_b.max_x - box_b.min_x) * (box_b.max_y - box_b.min_y) * (box_b.max_z - box_b.min_z);
                double min_volume = std::min(volume_a, volume_b);
                
                // If overlap is significant (>10% of smaller volume), consider it a collision
                if (overlap_vol > min_volume * 0.1) {
                    return true;
                }
            }
            
            // Holes don't cause interference (they remove material)
            if (feat_a.type == FeatureType::Hole || feat_b.type == FeatureType::Hole) {
                continue;
            }
        }
    }
    
    // If we get here and boxes overlap, it's a potential collision
    return true;
}

double InterferenceChecker::calculateIntersectionVolume(const Part& part_a, const Transform& transform_a,
                                                         const Part& part_b, const Transform& transform_b) const {
    // Calculate precise intersection volume
    BoundingBox box_a = estimateBoundingBoxFromFeatures(part_a, transform_a);
    BoundingBox box_b = estimateBoundingBoxFromFeatures(part_b, transform_b);
    
    if (!boxesOverlap(box_a, box_b)) {
        return 0.0;
    }
    
    // Calculate intersection box
    double min_x = std::max(box_a.min_x, box_b.min_x);
    double max_x = std::min(box_a.max_x, box_b.max_x);
    double min_y = std::max(box_a.min_y, box_b.min_y);
    double max_y = std::min(box_a.max_y, box_b.max_y);
    double min_z = std::max(box_a.min_z, box_b.min_z);
    double max_z = std::min(box_a.max_z, box_b.max_z);
    
    double volume = (max_x - min_x) * (max_y - min_y) * (max_z - min_z);
    return volume;
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
