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
    
    // Part feature operations
    Part applyExtrude(Part& part, const std::string& sketch_id, double depth, bool symmetric = false) const;
    Part applyRevolve(Part& part, const std::string& sketch_id, double angle, const std::string& axis = "Z") const;
    Part applyLoft(Part& part, const std::vector<std::string>& sketch_ids) const;
    Part applyHole(Part& part, double diameter, double depth = 0.0, bool through_all = false) const;
    Part applyFillet(Part& part, double radius, const std::vector<std::string>& edge_ids) const;
};

}  // namespace core
}  // namespace cad
