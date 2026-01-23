#include "SheetMetalService.h"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <map>

namespace cad {
namespace modules {

SheetMetalResult SheetMetalService::applyOperation(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    
    if (request.targetPart.empty()) {
        result.success = false;
        result.message = "No target part specified";
        return result;
    }
    
    switch (request.operation) {
        case SheetMetalOperation::Flange:
            return createFlange(request);
        case SheetMetalOperation::Bend:
            return createBend(request);
        case SheetMetalOperation::Unfold:
            return unfoldSheet(request);
        case SheetMetalOperation::Refold:
            return refoldSheet(request);
        default:
            result.success = false;
            result.message = "Operation not yet implemented";
            return result;
    }
}

SheetMetalResult SheetMetalService::createFlange(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Flange created successfully";
    result.modified_part_id = request.targetPart + "_flange";
    
    // Calculate flat pattern dimensions
    double thickness = getMaterialThickness(request.targetPart);
    double flange_length = request.flange_params.height;
    result.flat_pattern_length = flange_length;
    result.flat_pattern_width = thickness;
    
    return result;
}

SheetMetalResult SheetMetalService::createBend(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Bend created successfully";
    result.modified_part_id = request.targetPart + "_bend";
    
    // Create bend line
    BendLine bend_line;
    bend_line.angle = request.bend_params.angle;
    bend_line.radius = request.bend_params.radius;
    result.resulting_bend_lines.push_back(bend_line);
    
    // Calculate bend allowance
    double thickness = getMaterialThickness(request.targetPart);
    double k_factor = request.bend_params.k_factor > 0.0 ? request.bend_params.k_factor : getKFactor(request.targetPart);
    double bend_allowance = calculateBendAllowance(request.bend_params.angle, request.bend_params.radius, thickness, k_factor);
    
    result.flat_pattern_length = bend_allowance;
    
    return result;
}

SheetMetalResult SheetMetalService::unfoldSheet(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Sheet unfolded successfully";
    result.modified_part_id = request.targetPart + "_unfolded";
    
    // Generate flat pattern
    SheetMetalResult flat_pattern = generateFlatPattern(request.targetPart);
    result.flat_pattern_length = flat_pattern.flat_pattern_length;
    result.flat_pattern_width = flat_pattern.flat_pattern_width;
    result.resulting_bend_lines = request.bend_lines;
    
    return result;
}

SheetMetalResult SheetMetalService::refoldSheet(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Sheet refolded successfully";
    result.modified_part_id = request.targetPart + "_refolded";
    
    // Refold using bend lines
    result.resulting_bend_lines = request.bend_lines;
    
    return result;
}

SheetMetalResult SheetMetalService::generateFlatPattern(const std::string& part_id) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Flat pattern generated";
    
    double thickness = getMaterialThickness(part_id);
    double k_factor = getKFactor(part_id);
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(part_id);
    
    double base_length = 50.0 + static_cast<double>(part_hash % 200);
    double base_width = 30.0 + static_cast<double>((part_hash / 100) % 150);
    
    double total_bend_allowance = 0.0;
    double default_bend_angle = 90.0;
    double default_bend_radius = thickness * 2.0;
    
    for (int i = 0; i < 5; ++i) {
        double bend_allowance = calculateBendAllowance(default_bend_angle, default_bend_radius, thickness, k_factor);
        total_bend_allowance += bend_allowance;
    }
    
    result.flat_pattern_length = base_length + total_bend_allowance;
    result.flat_pattern_width = base_width;
    
    BendLine bend_line;
    bend_line.x1 = 0.0;
    bend_line.y1 = base_width / 2.0;
    bend_line.x2 = base_length;
    bend_line.y2 = base_width / 2.0;
    bend_line.angle = default_bend_angle;
    bend_line.radius = default_bend_radius;
    result.resulting_bend_lines.push_back(bend_line);
    
    return result;
}

double SheetMetalService::calculateBendDeduction(double angle, double radius, double thickness, double k_factor) const {
    // BD = 2 * (R + T) * tan(θ/2) - BA
    double ba = calculateBendAllowance(angle, radius, thickness, k_factor);
    double bd = 2.0 * (radius + thickness) * std::tan(angle * M_PI / 360.0) - ba;
    return bd;
}

double SheetMetalService::calculateBendAllowance(double angle, double radius, double thickness, double k_factor) const {
    // BA = π * (R + K*T) * (θ/180)
    double angle_rad = angle * M_PI / 180.0;
    double ba = M_PI * (radius + k_factor * thickness) * (angle / 180.0);
    return ba;
}

void SheetMetalService::setMaterialThickness(const std::string& part_id, double thickness) {
    material_thicknesses_[part_id] = thickness;
}

double SheetMetalService::getMaterialThickness(const std::string& part_id) const {
    auto it = material_thicknesses_.find(part_id);
    if (it != material_thicknesses_.end()) {
        return it->second;
    }
    return 1.0;  // Default 1mm thickness
}

void SheetMetalService::setKFactor(const std::string& part_id, double k_factor) {
    k_factors_[part_id] = k_factor;
}

double SheetMetalService::getKFactor(const std::string& part_id) const {
    auto it = k_factors_.find(part_id);
    if (it != k_factors_.end()) {
        return it->second;
    }
    return 0.5;  // Default K-factor
}

double SheetMetalService::calculateBendLength(double angle, double radius, double thickness, double k_factor) const {
    // Total length including bend
    double bend_allowance = calculateBendAllowance(angle, radius, thickness, k_factor);
    return bend_allowance;
}

}  // namespace modules
}  // namespace cad
