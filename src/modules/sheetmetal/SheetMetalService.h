#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

enum class SheetMetalOperation {
    Flange,
    Face,
    Cut,
    Bend,
    Unfold,
    Refold
};

struct BendLine {
    double x1{0.0};
    double y1{0.0};
    double x2{0.0};
    double y2{0.0};
    double angle{90.0};  // Bend angle in degrees
    double radius{1.0};  // Bend radius
};

struct FlangeParameters {
    double height{10.0};
    double angle{90.0};
    double offset{0.0};
    std::string edge_id;
};

struct BendParameters {
    double angle{90.0};
    double radius{1.0};
    double k_factor{0.5};
    std::string bend_line_id;
};

struct UnfoldParameters {
    double k_factor{0.5};
    bool preserve_bend_lines{true};
};

struct SheetMetalRequest {
    std::string targetPart;
    SheetMetalOperation operation;
    FlangeParameters flange_params;
    BendParameters bend_params;
    UnfoldParameters unfold_params;
    std::vector<BendLine> bend_lines;
};

struct SheetMetalResult {
    bool success{false};
    std::string message;
    std::string modified_part_id;
    double flat_pattern_length{0.0};
    double flat_pattern_width{0.0};
    std::vector<BendLine> resulting_bend_lines;
};

class SheetMetalService {
public:
    SheetMetalResult applyOperation(const SheetMetalRequest& request) const;
    SheetMetalResult createFlange(const SheetMetalRequest& request) const;
    SheetMetalResult createBend(const SheetMetalRequest& request) const;
    SheetMetalResult createCut(const SheetMetalRequest& request) const;
    SheetMetalResult createFace(const SheetMetalRequest& request) const;
    SheetMetalResult unfoldSheet(const SheetMetalRequest& request) const;
    SheetMetalResult refoldSheet(const SheetMetalRequest& request) const;
    
    // Flat pattern generation
    SheetMetalResult generateFlatPattern(const std::string& part_id) const;
    double calculateBendDeduction(double angle, double radius, double thickness, double k_factor) const;
    double calculateBendAllowance(double angle, double radius, double thickness, double k_factor) const;
    
    // Material properties
    void setMaterialThickness(const std::string& part_id, double thickness);
    double getMaterialThickness(const std::string& part_id) const;
    void setKFactor(const std::string& part_id, double k_factor);
    double getKFactor(const std::string& part_id) const;
    
private:
    std::map<std::string, double> material_thicknesses_;
    std::map<std::string, double> k_factors_;
    
    double calculateBendLength(double angle, double radius, double thickness, double k_factor) const;
};

}  // namespace modules
}  // namespace cad
