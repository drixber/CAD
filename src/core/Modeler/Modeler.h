#pragma once

#include "Assembly.h"
#include "Part.h"
#include "Sketch.h"

namespace cad {
namespace core {

class Modeler {
public:
    Part createPart(const Sketch& sketch) const;
    Assembly createAssembly() const;

    bool validateSketch(const Sketch& sketch) const;
    bool evaluateParameters(Sketch& sketch) const;
    bool solveConstraints(Sketch& sketch) const;
    
    // Constraint validation
    bool validateConstraints(const Sketch& sketch) const;
    bool isOverConstrained(const Sketch& sketch) const;
    bool isUnderConstrained(const Sketch& sketch) const;
    int getDegreesOfFreedom(const Sketch& sketch) const;
    
    // Part feature operations
    Part applyExtrude(Part& part, const std::string& sketch_id, double depth, bool symmetric = false) const;
    Part applyRevolve(Part& part, const std::string& sketch_id, double angle, const std::string& axis = "Z") const;
    Part applyLoft(Part& part, const std::vector<std::string>& sketch_ids) const;
    Part applySweep(Part& part, const std::string& profile_sketch_id, const std::string& path_sketch_id,
                   double twist_angle = 0.0, double scale_factor = 1.0) const;
    Part applyHelix(Part& part, double radius, double pitch, double revolutions, bool clockwise = false) const;
    Part applyHole(Part& part, double diameter, double depth = 0.0, bool through_all = false) const;
    Part applyFillet(Part& part, double radius, const std::vector<std::string>& edge_ids) const;
    Part applyChamfer(Part& part, double distance1, double distance2 = 0.0, double angle = 45.0,
                     const std::vector<std::string>& edge_ids = {}) const;
    Part applyShell(Part& part, double wall_thickness, const std::vector<std::string>& face_ids = {}) const;
    Part applyDraft(Part& part, double angle, const std::string& draft_plane,
                   const std::vector<std::string>& face_ids = {}) const;
    Part applyMirror(Part& part, const std::string& base_feature, const std::string& mirror_plane,
                    bool merge_result = true) const;
    Part applyThread(Part& part, const std::string& thread_standard, double pitch, bool internal = false) const;
    Part applyRib(Part& part, double thickness, const std::string& rib_plane, const std::string& sketch_id) const;
};

}  // namespace core
}  // namespace cad
