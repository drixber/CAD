#include "PatternService.h"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <map>

namespace cad {
namespace modules {

PatternResult PatternService::createPattern(const PatternRequest& request) const {
    PatternResult result;
    
    if (request.targetFeature.empty()) {
        result.success = false;
        result.message = "No target feature specified";
        return result;
    }
    
    switch (request.type) {
        case PatternType::Rectangular:
            return createRectangularPattern(request);
        case PatternType::Circular:
            return createCircularPattern(request);
        case PatternType::CurveDriven:
            return createCurvePattern(request);
        default:
            result.success = false;
            result.message = "Unknown pattern type";
            return result;
    }
}

PatternResult PatternService::createRectangularPattern(const PatternRequest& request) const {
    PatternResult result;
    result.success = true;
    result.message = "Rectangular pattern created";
    result.pattern_type = PatternType::Rectangular;
    result.pattern_id = request.targetFeature + "_rect_pattern";
    
    // Generate instances
    result.instances = generateRectangularInstances(request.rectangular_params);
    result.total_instances = result.instances.size();
    
    // Apply variations if specified
    if (request.vary_instances && !request.instance_variations.empty()) {
        for (auto& instance : result.instances) {
            auto it = request.instance_variations.find(instance.instance_id);
            if (it != request.instance_variations.end() && it->second.size() >= 3) {
                instance.x += it->second[0];
                instance.y += it->second[1];
                instance.z += it->second[2];
            }
        }
    }
    
    patterns_[result.pattern_id] = result;
    
    return result;
}

PatternResult PatternService::createCircularPattern(const PatternRequest& request) const {
    PatternResult result;
    result.success = true;
    result.message = "Circular pattern created";
    result.pattern_type = PatternType::Circular;
    result.pattern_id = request.targetFeature + "_circ_pattern";
    
    // Generate instances
    result.instances = generateCircularInstances(request.circular_params);
    result.total_instances = result.instances.size();
    
    patterns_[result.pattern_id] = result;
    
    return result;
}

PatternResult PatternService::createCurvePattern(const PatternRequest& request) const {
    PatternResult result;
    result.success = true;
    result.message = "Curve pattern created";
    result.pattern_type = PatternType::CurveDriven;
    result.pattern_id = request.targetFeature + "_curve_pattern";
    
    // Generate instances
    result.instances = generateCurveInstances(request.curve_params);
    result.total_instances = result.instances.size();
    
    patterns_[result.pattern_id] = result;
    
    return result;
}

PatternResult PatternService::editPattern(const std::string& pattern_id, const PatternRequest& request) const {
    PatternResult result;
    
    auto it = patterns_.find(pattern_id);
    if (it == patterns_.end()) {
        result.success = false;
        result.message = "Pattern not found";
        return result;
    }
    
    // Recreate pattern with new parameters
    return createPattern(request);
}

PatternResult PatternService::suppressInstance(const std::string& pattern_id, const std::string& instance_id) const {
    PatternResult result;
    
    auto it = patterns_.find(pattern_id);
    if (it == patterns_.end()) {
        result.success = false;
        result.message = "Pattern not found";
        return result;
    }
    
    PatternResult& pattern = const_cast<PatternResult&>(it->second);
    for (auto& instance : pattern.instances) {
        if (instance.instance_id == instance_id) {
            instance.suppressed = true;
            result.success = true;
            result.message = "Instance suppressed";
            return result;
        }
    }
    
    result.success = false;
    result.message = "Instance not found";
    return result;
}

PatternResult PatternService::unsuppressInstance(const std::string& pattern_id, const std::string& instance_id) const {
    PatternResult result;
    
    auto it = patterns_.find(pattern_id);
    if (it == patterns_.end()) {
        result.success = false;
        result.message = "Pattern not found";
        return result;
    }
    
    PatternResult& pattern = const_cast<PatternResult&>(it->second);
    for (auto& instance : pattern.instances) {
        if (instance.instance_id == instance_id) {
            instance.suppressed = false;
            result.success = true;
            result.message = "Instance unsuppressed";
            return result;
        }
    }
    
    result.success = false;
    result.message = "Instance not found";
    return result;
}

PatternResult PatternService::deletePattern(const std::string& pattern_id) const {
    PatternResult result;
    
    auto it = patterns_.find(pattern_id);
    if (it == patterns_.end()) {
        result.success = false;
        result.message = "Pattern not found";
        return result;
    }
    
    patterns_.erase(it);
    result.success = true;
    result.message = "Pattern deleted";
    return result;
}

std::vector<PatternInstance> PatternService::getPatternInstances(const std::string& pattern_id) const {
    auto it = patterns_.find(pattern_id);
    if (it != patterns_.end()) {
        return it->second.instances;
    }
    return {};
}

PatternType PatternService::getPatternType(const std::string& pattern_id) const {
    auto it = patterns_.find(pattern_id);
    if (it != patterns_.end()) {
        return it->second.pattern_type;
    }
    return PatternType::Rectangular;
}

std::vector<PatternInstance> PatternService::generateRectangularInstances(const RectangularPatternParams& params) const {
    std::vector<PatternInstance> instances;
    
    for (int i = 0; i < params.count_x; ++i) {
        for (int j = 0; j < params.count_y; ++j) {
            PatternInstance instance;
            instance.instance_id = "instance_" + std::to_string(i) + "_" + std::to_string(j);
            
            // Calculate position with rotation
            double x = i * params.spacing_x;
            double y = j * params.spacing_y;
            double angle_rad = params.angle * M_PI / 180.0;
            
            instance.x = x * std::cos(angle_rad) - y * std::sin(angle_rad);
            instance.y = x * std::sin(angle_rad) + y * std::cos(angle_rad);
            instance.z = 0.0;
            instance.rotation = params.angle;
            
            instances.push_back(instance);
        }
    }
    
    return instances;
}

std::vector<PatternInstance> PatternService::generateCircularInstances(const CircularPatternParams& params) const {
    std::vector<PatternInstance> instances;
    
    double angle_step = params.angle / params.count;
    
    for (int i = 0; i < params.count; ++i) {
        PatternInstance instance;
        instance.instance_id = "instance_" + std::to_string(i);
        
        double angle = i * angle_step * M_PI / 180.0;
        instance.x = params.center_x + params.radius * std::cos(angle);
        instance.y = params.center_y + params.radius * std::sin(angle);
        instance.z = params.center_z;
        instance.rotation = i * angle_step;
        
        instances.push_back(instance);
    }
    
    return instances;
}

std::vector<PatternInstance> PatternService::generateCurveInstances(const CurvePatternParams& params) const {
    std::vector<PatternInstance> instances;
    
    // Simplified: generate instances along curve with spacing
    for (int i = 0; i < params.count; ++i) {
        PatternInstance instance;
        instance.instance_id = "instance_" + std::to_string(i);
        
        // Simplified: assume linear curve
        double t = static_cast<double>(i) / (params.count - 1);
        instance.x = t * params.spacing * params.count;
        instance.y = 0.0;
        instance.z = 0.0;
        
        if (params.align_to_curve) {
            instance.rotation = t * 45.0;  // Simplified rotation
        }
        
        instances.push_back(instance);
    }
    
    return instances;
}

}  // namespace modules
}  // namespace cad
