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
};

}  // namespace core
}  // namespace cad
