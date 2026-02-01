#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Part.h"
#include "ReferenceGeometry.h"
#include "Transform.h"

namespace cad {
namespace core {

struct AssemblyComponent {
    std::uint64_t id{0};
    Part part{"Part"};
    Transform transform{};
    /** SpeedPak / Large Assembly Mode: bei true nur vereinfachte Darstellung (LOD). */
    bool lightweight_display{false};
    /** Flexible Subassembly (SolidWorks): Unterbaugruppe kann intern bewegt werden. */
    bool flexible_subassembly{false};
};

enum class MateType {
    Mate,       /** Coincident / Bündig */
    Flush,      /** Coplanar / Fluchtend (parallel planes) */
    Concentric, /** Concentric / Fluchtend (cylindrical) */
    Tangent,    /** Tangential */
    Parallel,   /** Parallel (SolidWorks): Ebenen/Achsen parallel, value = Abstand optional */
    Distance,   /** Abstand (SolidWorks): value = Abstand */
    Angle,      /** Winkel */
    Insert,     /** Einfügen (hole/axis) */
    Gear,       /** Zahnrad: Übersetzungsverhältnis (value = ratio) */
    Cam         /** Kurvengetriebe: Phasenbeziehung (value = phase offset) */
};

struct MateConstraint {
    std::uint64_t component_a{0};
    std::uint64_t component_b{0};
    MateType type{MateType::Mate};
    double value{0.0};
};

/** Joint type for kinematic motion (§13): defines allowed DOF between two components. */
enum class JointType {
    Rigid,      /** No relative motion */
    Revolute,   /** Drehgelenk: rotation about one axis */
    Slider,     /** Schiebegelenk: translation along one axis */
    Cylindrical,/** Zylindergelenk: rotation + translation along same axis */
    Planar,     /** Planargelenk: motion in a plane */
    PinSlot     /** Pin-Slot (Fusion): rotation + translation in plane (slot direction) */
};

/** Joint between two components (for simulation / MBD). */
struct Joint {
    std::uint64_t component_a{0};
    std::uint64_t component_b{0};
    JointType type{JointType::Rigid};
    /** Axis direction (normalized); for Revolute/Slider/Cylindrical/PinSlot. */
    Vector3D axis_direction{0.0, 0.0, 1.0};
    /** Axis origin (e.g. pivot point); optional. */
    Point3D axis_origin{0.0, 0.0, 0.0};
    /** Slot direction (PinSlot only); translation along this axis. */
    Vector3D slot_direction{1.0, 0.0, 0.0};
    /** Lower limit (angle in rad or distance); NaN = no limit. */
    double limit_low{0.0};
    /** Upper limit (angle in rad or distance); NaN = no limit. */
    double limit_high{0.0};
};

/** Baugruppen-Konfiguration (§19.6): Variante (welche Teil-Konfig pro Komponente, Sichtbarkeit). */
struct AssemblyConfiguration {
    std::string name;
    /** Komponenten-ID -> Konfigurationsindex im jeweiligen Part (optional). */
    std::map<std::uint64_t, int> component_config_index;
};

class Assembly {
public:
    std::uint64_t addComponent(const Part& part, const Transform& transform);
    const std::vector<AssemblyComponent>& components() const;
    void addMate(const MateConstraint& mate);
    const std::vector<MateConstraint>& mates() const;
    
    // Mate creation helpers
    std::string createMate(std::uint64_t component_a, std::uint64_t component_b, double offset = 0.0);
    std::string createFlush(std::uint64_t component_a, std::uint64_t component_b, double offset = 0.0);
    std::string createConcentric(std::uint64_t component_a, std::uint64_t component_b, double offset = 0.0);
    std::string createTangent(std::uint64_t component_a, std::uint64_t component_b, double offset = 0.0);
    /** Parallel-Mate (SolidWorks): Ebenen/Achsen parallel; value = optionaler Abstand. */
    std::string createParallel(std::uint64_t component_a, std::uint64_t component_b, double distance = 0.0);
    /** Distance-Mate (SolidWorks): value = Abstand. */
    std::string createDistance(std::uint64_t component_a, std::uint64_t component_b, double distance);
    std::string createAngle(std::uint64_t component_a, std::uint64_t component_b, double angle);
    std::string createInsert(std::uint64_t component_a, std::uint64_t component_b);
    /** Gear-Mate (SolidWorks): value = Übersetzungsverhältnis (z.B. -0.5). */
    std::string createGear(std::uint64_t component_a, std::uint64_t component_b, double ratio);
    /** Cam-Mate (SolidWorks): value = Phasenversatz (rad). */
    std::string createCam(std::uint64_t component_a, std::uint64_t component_b, double phase_offset = 0.0);
    
    // Mate solving (updates component transforms based on mates)
    bool solveMates();
    
    // Mate validation
    bool validateMates() const;
    int getDegreesOfFreedom() const;
    bool isOverConstrained() const;
    bool isUnderConstrained() const;
    
    AssemblyComponent* findComponent(std::uint64_t id);
    const AssemblyComponent* findComponent(std::uint64_t id) const;

    // Joints (§13): kinematic connections for simulation
    // Creo: Mechanisms/Kinematik-UI nutzt dieselbe Joint-API (Revolute, Slider, Cylindrical, Planar).
    void addJoint(const Joint& joint);
    const std::vector<Joint>& joints() const;
    /** DOF count from joints only (Revolute/Slider=1, Cylindrical=2, Planar=3, Rigid=0). */
    int getJointDegreesOfFreedom() const;
    /** Create revolute joint (rotation about axis). */
    std::string createRevolute(std::uint64_t comp_a, std::uint64_t comp_b,
                               double axis_x, double axis_y, double axis_z,
                               double limit_lo = 0.0, double limit_hi = 0.0);
    /** Create slider joint (translation along axis). */
    std::string createSlider(std::uint64_t comp_a, std::uint64_t comp_b,
                             double axis_x, double axis_y, double axis_z,
                             double limit_lo = 0.0, double limit_hi = 0.0);
    /** Create cylindrical joint (rotation + translation along same axis). */
    std::string createCylindrical(std::uint64_t comp_a, std::uint64_t comp_b,
                                  double axis_x, double axis_y, double axis_z);
    /** Create planar joint (motion in plane). */
    std::string createPlanar(std::uint64_t comp_a, std::uint64_t comp_b,
                             double normal_x, double normal_y, double normal_z);
    /** Pin-Slot Joint (Fusion): rotation about axis + translation along slot direction. */
    std::string createPinSlot(std::uint64_t comp_a, std::uint64_t comp_b,
                              double axis_x, double axis_y, double axis_z,
                              double slot_x, double slot_y, double slot_z,
                              double limit_lo = 0.0, double limit_hi = 0.0);

    // Explosion / presentation view (§15)
    void setExplosionOffset(std::uint64_t component_id, double dx, double dy, double dz);
    Vector3D getExplosionOffset(std::uint64_t component_id) const;
    void setExplosionFactor(double factor);
    double getExplosionFactor() const;
    void clearExplosionOffsets();
    bool hasExplosionOffsets() const;
    /** Effective transform for display (base transform + explosion_offset * factor). */
    Transform getDisplayTransform(std::uint64_t component_id) const;

    /** SpeedPak / Large Assembly: Leichte Darstellung (nur Bounding-Box/LOD). */
    void setComponentLightweight(std::uint64_t component_id, bool lightweight);
    bool isComponentLightweight(std::uint64_t component_id) const;
    /** Flexible Subassembly (SolidWorks): Unterbaugruppe beweglich. */
    void setComponentFlexible(std::uint64_t component_id, bool flexible);
    bool isComponentFlexible(std::uint64_t component_id) const;

    // Konfigurationen (§19.6): Varianten, Stücklistenlogik
    void addConfiguration(const AssemblyConfiguration& config);
    const std::vector<AssemblyConfiguration>& configurations() const;
    void setActiveConfiguration(int index);
    int activeConfigurationIndex() const;
    /** NX Arrangement Manager: Alias für Konfigurationen. */
    void addArrangement(const AssemblyConfiguration& config);
    const std::vector<AssemblyConfiguration>& arrangements() const;

    /** Creo: Component Interfaces – Schnittstellen für automatische Zuordnung. */
    void addComponentInterface(std::uint64_t component_id, const std::string& interface_name);
    std::vector<std::string> getComponentInterfaces(std::uint64_t component_id) const;

private:
    std::uint64_t next_id_{1};
    std::vector<AssemblyComponent> components_{};
    std::vector<MateConstraint> mates_{};
    std::vector<Joint> joints_{};
    std::vector<AssemblyConfiguration> configurations_{};
    int active_configuration_index_{0};
    std::set<std::uint64_t> lightweight_components_{};
    std::set<std::uint64_t> flexible_components_{};
    std::map<std::uint64_t, Vector3D> explosion_offsets_{};
    double explosion_factor_{0.0};
    std::map<std::uint64_t, std::vector<std::string>> component_interfaces_{};
};

}  // namespace core
}  // namespace cad
