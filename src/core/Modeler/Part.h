#pragma once

#include <string>
#include <vector>
#include <map>
#include "ReferenceGeometry.h"
#include "Sketch.h"

namespace cad {
namespace core {

enum class ExtrudeMode {
    Join,
    Cut,
    Intersect,
    NewBody
};

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
    Pattern,
    CircularPattern,
    PathPattern
};

struct Feature {
    std::string name;
    FeatureType type{FeatureType::Extrude};
    std::string sketch_id;  // Reference to source sketch
    /** Inventor: Unterdrückung – Feature wird nicht ausgewertet. */
    bool suppressed{false};

    // Feature parameters
    std::map<std::string, double> parameters;
    
    // For Extrude
    double depth{0.0};
    bool symmetric{false};
    ExtrudeMode extrude_mode{ExtrudeMode::Join};
    /** Thin Feature (SolidWorks): dünnwandige Extrusion, Wandstärke. */
    bool thin_wall{false};
    double thin_thickness{1.0};
    
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
    // For CircularPattern
    int circular_count{4};
    double circular_angle{360.0};  // Total angle in degrees
    std::string circular_axis{"Z"};  // "X", "Y", "Z" or axis id
    // For PathPattern / Sweep (shared: path sketch id)
    std::string path_sketch_id;
    int path_count{4};
    bool path_equal_spacing{true};
    
    // For Sweep
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

/** iLogic-ähnliche Regel: Wenn condition erfüllt, dann setze then_parameter = then_value_expression. */
struct Rule {
    std::string name;
    std::string trigger{"ParameterChange"};  // "ParameterChange", "DocumentOpen"
    std::string condition_expression;       // z.B. "Width > 10"
    std::string then_parameter;             // z.B. "Height"
    std::string then_value_expression;       // z.B. "20" oder "Width*2"
};

/** Konfiguration (§19.6): Variante mit Parameter-Overrides (Maß-/Feature-Steuerung). */
struct Configuration {
    std::string name;
    std::map<std::string, double> parameter_overrides;
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
    std::string createExtrude(const std::string& sketch_id, double depth, bool symmetric = false,
                              ExtrudeMode mode = ExtrudeMode::Join);
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
    std::string createCircularPattern(const std::string& base_feature, int count, double angle_deg = 360.0,
                                     const std::string& axis = "Z");
    /** Pfadmuster (darius): Feature entlang Pfad-Skizze. */
    std::string createPathPattern(const std::string& base_feature, const std::string& path_sketch_id,
                                  int count = 4, bool equal_spacing = true);
    /** Thin Feature (SolidWorks): dünnwandige Extrusion. */
    std::string createThinExtrude(const std::string& sketch_id, double depth, double wall_thickness,
                                  bool symmetric = false, ExtrudeMode mode = ExtrudeMode::Join);

    // Reference geometry (Arbeitselemente)
    std::string addWorkPlane(const std::string& name, const Point3D& origin, const Vector3D& normal);
    std::string addWorkPlaneOffset(const std::string& name, const std::string& base_plane, double offset);
    std::string addWorkAxis(const std::string& name, const Point3D& point, const Vector3D& direction);
    std::string addWorkAxisBase(const std::string& name, const std::string& base_axis);
    std::string addWorkPoint(const std::string& name, const Point3D& point);
    std::string addCoordinateSystem(const std::string& name, const Point3D& origin,
                                    const Vector3D& dir_x, const Vector3D& dir_y);
    const std::vector<WorkPlane>& workPlanes() const;
    const std::vector<WorkAxis>& workAxes() const;
    const std::vector<WorkPoint>& workPoints() const;
    const std::vector<CoordinateSystem>& coordinateSystems() const;
    WorkPlane* findWorkPlane(const std::string& id);
    WorkAxis* findWorkAxis(const std::string& id);
    WorkPoint* findWorkPoint(const std::string& id);

    // Benutzerparameter (zentrale Parameter-Tabelle)
    void addUserParameter(const Parameter& p);
    std::vector<Parameter>& userParameters();
    const std::vector<Parameter>& userParameters() const;
    bool setParameterValue(const std::string& name, double value);
    bool removeParameter(const std::string& name);
    Parameter* findParameter(const std::string& name);
    const Parameter* findParameter(const std::string& name) const;

    // iLogic-ähnliche Regeln (Wenn-Dann)
    void addRule(const Rule& rule);
    std::vector<Rule>& rules();
    const std::vector<Rule>& rules() const;

    // Konfigurationen (§19.6): Varianten, Maß-/Feature-Steuerung, Stücklistenlogik
    void addConfiguration(const Configuration& config);
    const std::vector<Configuration>& configurations() const;
    void setActiveConfiguration(int index);
    int activeConfigurationIndex() const;

    /** Top-Down / Skeleton (Creo §20.2): Referenz auf Skeleton-Part für Referenzsteuerung. */
    void setSkeletonPartId(const std::string& part_id);
    std::string skeletonPartId() const;

    /** Inventor: Rollback-Leiste – Position bis zu der Features ausgewertet werden. */
    void setRollbackPosition(int feature_index);
    int rollbackPosition() const;
    /** Inventor: Feature-Unterdrückung. */
    bool setFeatureSuppressed(const std::string& feature_name, bool suppressed);
    bool isFeatureSuppressed(const std::string& feature_name) const;
    /** Fusion: Timeline – Feature-Reihenfolge ändern. */
    bool reorderFeature(std::size_t from_index, std::size_t to_index);

private:
    std::string name_;
    std::vector<Feature> features_;
    std::vector<WorkPlane> work_planes_;
    std::vector<WorkAxis> work_axes_;
    std::vector<WorkPoint> work_points_;
    std::vector<CoordinateSystem> coordinate_systems_;
    std::vector<Parameter> user_parameters_;
    std::vector<Rule> rules_;
    std::vector<Configuration> configurations_;
    int active_configuration_index_{0};
    std::string skeleton_part_id_;
    int rollback_position_{-1};  /** -1 = alle Features auswerten */
    int next_feature_id_{1};
    int next_ref_id_{1};

    std::string generateFeatureName(FeatureType type);
    std::string generateReferenceId(const char* prefix);
};

}  // namespace core
}  // namespace cad
