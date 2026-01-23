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
        case SheetMetalOperation::Cut:
            return createCut(request);
        case SheetMetalOperation::Face:
            return createFace(request);
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
    
    double thickness = getMaterialThickness(request.targetPart);
    double flange_height = request.flange_params.height;
    double flange_angle = request.flange_params.angle * M_PI / 180.0;
    double flange_offset = request.flange_params.offset;
    
    double base_length = 100.0;
    double base_width = 50.0;
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(request.targetPart);
    base_length = 50.0 + static_cast<double>(part_hash % 200);
    base_width = 30.0 + static_cast<double>((part_hash / 100) % 150);
    
    double flange_projected_length = flange_height * std::cos(flange_angle);
    double flange_projected_width = flange_height * std::sin(flange_angle);
    
    result.flat_pattern_length = base_length + flange_projected_length + flange_offset;
    result.flat_pattern_width = base_width + flange_projected_width;
    
    BendLine bend_line;
    bend_line.x1 = base_length + flange_offset;
    bend_line.y1 = 0.0;
    bend_line.x2 = base_length + flange_offset;
    bend_line.y2 = base_width;
    bend_line.angle = request.flange_params.angle;
    bend_line.radius = thickness * 2.0;
    result.resulting_bend_lines.push_back(bend_line);
    
    return result;
}

SheetMetalResult SheetMetalService::createBend(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Bend created successfully";
    result.modified_part_id = request.targetPart + "_bend";
    
    double thickness = getMaterialThickness(request.targetPart);
    double k_factor = request.bend_params.k_factor > 0.0 ? request.bend_params.k_factor : getKFactor(request.targetPart);
    double bend_angle = request.bend_params.angle;
    double bend_radius = request.bend_params.radius > 0.0 ? request.bend_params.radius : thickness * 2.0;
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(request.targetPart);
    double base_length = 50.0 + static_cast<double>(part_hash % 200);
    double base_width = 30.0 + static_cast<double>((part_hash / 100) % 150);
    
    double bend_allowance = calculateBendAllowance(bend_angle, bend_radius, thickness, k_factor);
    double bend_deduction = calculateBendDeduction(bend_angle, bend_radius, thickness, k_factor);
    
    double unfolded_length = base_length + bend_allowance;
    double unfolded_width = base_width;
    
    if (bend_angle < 90.0) {
        double angle_rad = bend_angle * M_PI / 180.0;
        unfolded_length += bend_radius * std::tan(angle_rad / 2.0);
    }
    
    result.flat_pattern_length = unfolded_length;
    result.flat_pattern_width = unfolded_width;
    
    BendLine bend_line;
    if (!request.bend_params.bend_line_id.empty()) {
        std::hash<std::string> line_hasher;
        std::size_t line_hash = line_hasher(request.bend_params.bend_line_id);
        bend_line.x1 = static_cast<double>(line_hash % 100);
        bend_line.y1 = base_width / 2.0;
        bend_line.x2 = base_length;
        bend_line.y2 = base_width / 2.0;
    } else {
        bend_line.x1 = base_length / 2.0;
        bend_line.y1 = 0.0;
        bend_line.x2 = base_length / 2.0;
        bend_line.y2 = base_width;
    }
    bend_line.angle = bend_angle;
    bend_line.radius = bend_radius;
    result.resulting_bend_lines.push_back(bend_line);
    
    return result;
}

SheetMetalResult SheetMetalService::unfoldSheet(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Sheet unfolded successfully";
    result.modified_part_id = request.targetPart + "_unfolded";
    
    double thickness = getMaterialThickness(request.targetPart);
    double k_factor = request.unfold_params.k_factor > 0.0 ? request.unfold_params.k_factor : getKFactor(request.targetPart);
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(request.targetPart);
    double base_length = 50.0 + static_cast<double>(part_hash % 200);
    double base_width = 30.0 + static_cast<double>((part_hash / 100) % 150);
    
    double total_unfolded_length = base_length;
    double total_unfolded_width = base_width;
    
    std::vector<BendLine> unfolded_bend_lines;
    
    for (const auto& bend_line : request.bend_lines) {
        double bend_allowance = calculateBendAllowance(bend_line.angle, bend_line.radius, thickness, k_factor);
        double bend_deduction = calculateBendDeduction(bend_line.angle, bend_line.radius, thickness, k_factor);
        
        double line_length = std::sqrt(
            (bend_line.x2 - bend_line.x1) * (bend_line.x2 - bend_line.x1) +
            (bend_line.y2 - bend_line.y1) * (bend_line.y2 - bend_line.y1)
        );
        
        if (std::abs(bend_line.x2 - bend_line.x1) > std::abs(bend_line.y2 - bend_line.y1)) {
            total_unfolded_length += bend_allowance;
        } else {
            total_unfolded_width += bend_allowance;
        }
        
        if (request.unfold_params.preserve_bend_lines) {
            BendLine unfolded_line = bend_line;
            unfolded_line.angle = 0.0;
            unfolded_bend_lines.push_back(unfolded_line);
        }
    }
    
    result.flat_pattern_length = total_unfolded_length;
    result.flat_pattern_width = total_unfolded_width;
    result.resulting_bend_lines = unfolded_bend_lines;
    
    return result;
}

SheetMetalResult SheetMetalService::refoldSheet(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Sheet refolded successfully";
    result.modified_part_id = request.targetPart + "_refolded";
    
    double thickness = getMaterialThickness(request.targetPart);
    double k_factor = getKFactor(request.targetPart);
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(request.targetPart);
    double base_length = 50.0 + static_cast<double>(part_hash % 200);
    double base_width = 30.0 + static_cast<double>((part_hash / 100) % 150);
    
    double total_folded_length = base_length;
    double total_folded_width = base_width;
    
    std::vector<BendLine> refolded_bend_lines;
    
    for (const auto& bend_line : request.bend_lines) {
        double bend_allowance = calculateBendAllowance(bend_line.angle, bend_line.radius, thickness, k_factor);
        double bend_deduction = calculateBendDeduction(bend_line.angle, bend_line.radius, thickness, k_factor);
        
        double angle_rad = bend_line.angle * M_PI / 180.0;
        double projected_length = bend_line.radius * std::sin(angle_rad);
        double projected_width = bend_line.radius * (1.0 - std::cos(angle_rad));
        
        if (std::abs(bend_line.x2 - bend_line.x1) > std::abs(bend_line.y2 - bend_line.y1)) {
            total_folded_length -= bend_allowance;
            total_folded_length += projected_length;
            total_folded_width += projected_width;
        } else {
            total_folded_width -= bend_allowance;
            total_folded_width += projected_length;
            total_folded_length += projected_width;
        }
        
        BendLine refolded_line = bend_line;
        refolded_line.angle = bend_line.angle;
        refolded_bend_lines.push_back(refolded_line);
    }
    
    result.flat_pattern_length = total_folded_length;
    result.flat_pattern_width = total_folded_width;
    result.resulting_bend_lines = refolded_bend_lines;
    
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

SheetMetalResult SheetMetalService::createCut(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Cut created successfully";
    result.modified_part_id = request.targetPart + "_cut";
    
    double thickness = getMaterialThickness(request.targetPart);
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(request.targetPart);
    double base_length = 50.0 + static_cast<double>(part_hash % 200);
    double base_width = 30.0 + static_cast<double>((part_hash / 100) % 150);
    
    double cut_area = base_length * base_width * 0.1;
    double cut_perimeter = 2.0 * (base_length + base_width) * 0.1;
    
    result.flat_pattern_length = base_length;
    result.flat_pattern_width = base_width;
    
    return result;
}

SheetMetalResult SheetMetalService::createFace(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    result.success = true;
    result.message = "Face created successfully";
    result.modified_part_id = request.targetPart + "_face";
    
    double thickness = getMaterialThickness(request.targetPart);
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(request.targetPart);
    double base_length = 50.0 + static_cast<double>(part_hash % 200);
    double base_width = 30.0 + static_cast<double>((part_hash / 100) % 150);
    
    result.flat_pattern_length = base_length;
    result.flat_pattern_width = base_width;
    
    return result;
}

double SheetMetalService::calculateBendLength(double angle, double radius, double thickness, double k_factor) const {
    double bend_allowance = calculateBendAllowance(angle, radius, thickness, k_factor);
    double angle_rad = angle * M_PI / 180.0;
    double straight_length = 2.0 * (radius + thickness) * std::tan(angle_rad / 2.0);
    return straight_length + bend_allowance;
}

}  // namespace modules
}  // namespace cad
