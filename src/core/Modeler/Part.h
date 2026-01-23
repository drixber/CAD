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
    Sweep,
    Helix,
    Hole,
    Fillet,
    Chamfer,
    Shell,
    Draft,
    Mirror,
    Thread,
    Rib,
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
    
    // For Sweep
    std::string path_sketch_id;  // Path sketch for sweep
    double twist_angle{0.0};     // Twist angle in degrees
    double scale_factor{1.0};    // Scale factor along path
    
    // For Helix/Spiral
    double pitch{0.0};            // Pitch for helix
    double revolutions{1.0};       // Number of revolutions
    bool clockwise{false};        // Clockwise or counter-clockwise
    
    // For Shell
    double wall_thickness{1.0};   // Wall thickness
    std::vector<std::string> face_ids;  // Faces to remove (empty = all)
    
    // For Draft
    double draft_angle{0.0};      // Draft angle in degrees
    std::string draft_plane;      // Draft plane reference
    
    // For Mirror
    std::string mirror_plane;     // Mirror plane reference
    bool merge_result{true};      // Merge mirrored geometry
    
    // For Thread
    std::string thread_standard{"ISO"};  // ISO, UNC, UNF, etc.
    double thread_pitch{1.0};      // Thread pitch
    bool internal{false};         // Internal or external thread
    
    // For Rib
    double rib_thickness{1.0};    // Rib thickness
    std::string rib_plane;        // Rib plane reference
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
    std::string createSweep(const std::string& profile_sketch_id, const std::string& path_sketch_id,
                           double twist_angle = 0.0, double scale_factor = 1.0);
    std::string createHelix(double radius, double pitch, double revolutions, bool clockwise = false);
    std::string createHole(double diameter, double depth = 0.0, bool through_all = false);
    std::string createFillet(double radius, const std::vector<std::string>& edge_ids);
    std::string createChamfer(double distance1, double distance2 = 0.0, double angle = 45.0,
                             const std::vector<std::string>& edge_ids = {});
    std::string createShell(double wall_thickness, const std::vector<std::string>& face_ids = {});
    std::string createDraft(double angle, const std::string& draft_plane,
                          const std::vector<std::string>& face_ids = {});
    std::string createMirror(const std::string& base_feature, const std::string& mirror_plane,
                            bool merge_result = true);
    std::string createThread(const std::string& thread_standard, double pitch, bool internal = false);
    std::string createRib(double thickness, const std::string& rib_plane, const std::string& sketch_id);
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
