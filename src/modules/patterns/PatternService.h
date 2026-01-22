#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

enum class PatternType {
    Rectangular,
    Circular,
    CurveDriven
};

struct PatternInstance {
    std::string instance_id;
    double x{0.0};
    double y{0.0};
    double z{0.0};
    double rotation{0.0};
    bool suppressed{false};
};

struct RectangularPatternParams {
    int count_x{2};
    int count_y{2};
    double spacing_x{10.0};
    double spacing_y{10.0};
    double angle{0.0};
};

struct CircularPatternParams {
    int count{4};
    double radius{10.0};
    double angle{360.0};
    double axis_x{0.0};
    double axis_y{0.0};
    double axis_z{1.0};
    double center_x{0.0};
    double center_y{0.0};
    double center_z{0.0};
};

struct CurvePatternParams {
    std::string curve_id;
    int count{5};
    double spacing{10.0};
    bool equal_spacing{true};
    bool align_to_curve{true};
};

struct PatternRequest {
    std::string targetFeature;
    PatternType type;
    int instanceCount{0};
    RectangularPatternParams rectangular_params;
    CircularPatternParams circular_params;
    CurvePatternParams curve_params;
    bool vary_instances{false};
    std::map<std::string, std::vector<double>> instance_variations;
};

struct PatternResult {
    bool success{false};
    std::string message;
    std::string pattern_id;
    std::vector<PatternInstance> instances;
    int total_instances{0};
    PatternType pattern_type;
};

class PatternService {
public:
    PatternResult createPattern(const PatternRequest& request) const;
    PatternResult createRectangularPattern(const PatternRequest& request) const;
    PatternResult createCircularPattern(const PatternRequest& request) const;
    PatternResult createCurvePattern(const PatternRequest& request) const;
    
    // Pattern editing
    PatternResult editPattern(const std::string& pattern_id, const PatternRequest& request) const;
    PatternResult suppressInstance(const std::string& pattern_id, const std::string& instance_id) const;
    PatternResult unsuppressInstance(const std::string& pattern_id, const std::string& instance_id) const;
    PatternResult deletePattern(const std::string& pattern_id) const;
    
    // Pattern queries
    std::vector<PatternInstance> getPatternInstances(const std::string& pattern_id) const;
    PatternType getPatternType(const std::string& pattern_id) const;
    
private:
    mutable std::map<std::string, PatternResult> patterns_;
    
    std::vector<PatternInstance> generateRectangularInstances(const RectangularPatternParams& params) const;
    std::vector<PatternInstance> generateCircularInstances(const CircularPatternParams& params) const;
    std::vector<PatternInstance> generateCurveInstances(const CurvePatternParams& params) const;
    
    double evaluateCurveX(const std::string& curve_id, double t) const;
    double evaluateCurveY(const std::string& curve_id, double t) const;
    double evaluateCurveZ(const std::string& curve_id, double t) const;
    double calculateCurveTangentAngle(const std::string& curve_id, double t) const;
};

}  // namespace modules
}  // namespace cad
