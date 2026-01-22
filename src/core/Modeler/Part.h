#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace core {

enum class FeatureType {
    Extrude,
    Revolve,
    Loft,
    Hole,
    Fillet,
    Chamfer,
    Shell,
    Pattern
};

struct Feature {
    std::string name;
    FeatureType type{FeatureType::Extrude};
    std::string sketch_id;  // Reference to source sketch
    
    // Feature parameters
    std::map<std::string, double> parameters;
    
    // For Extrude
    double depth{0.0};
    bool symmetric{false};
    
    // For Revolve
    double angle{360.0};
    std::string axis;  // "X", "Y", "Z" or custom
    
    // For Hole
    double diameter{0.0};
    double hole_depth{0.0};
    bool through_all{false};
    
    // For Fillet/Chamfer
    double radius{0.0};
    std::vector<std::string> edge_ids;
    
    // For Pattern
    int count_x{1};
    int count_y{1};
    int count_z{1};
    double spacing_x{0.0};
    double spacing_y{0.0};
    double spacing_z{0.0};
};

class Part {
public:
    explicit Part(std::string name);

    const std::string& name() const;
    void addFeature(const Feature& feature);
    const std::vector<Feature>& features() const;
    Feature* findFeature(const std::string& name);
    const Feature* findFeature(const std::string& name) const;
    bool removeFeature(const std::string& name);
    
    // Feature creation helpers
    std::string createExtrude(const std::string& sketch_id, double depth, bool symmetric = false);
    std::string createRevolve(const std::string& sketch_id, double angle, const std::string& axis = "Z");
    std::string createLoft(const std::vector<std::string>& sketch_ids);
    std::string createHole(double diameter, double depth = 0.0, bool through_all = false);
    std::string createFillet(double radius, const std::vector<std::string>& edge_ids);
    std::string createPattern(const std::string& base_feature, int count_x, int count_y, int count_z,
                             double spacing_x, double spacing_y, double spacing_z);

private:
    std::string name_;
    std::vector<Feature> features_;
    int next_feature_id_{1};
    
    std::string generateFeatureName(FeatureType type);
};

}  // namespace core
}  // namespace cad
