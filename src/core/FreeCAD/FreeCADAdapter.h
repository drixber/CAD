#pragma once

#include <string>

#include "core/Modeler/Modeler.h"

namespace cad {
namespace core {

class FreeCADAdapter {
public:
    FreeCADAdapter();

    bool initializeSession();
    bool createDocument(const std::string& name);
    bool createSketchStub(const std::string& name) const;
    bool syncSketch(const Sketch& sketch) const;
    bool syncConstraints(const Sketch& sketch) const;
    bool syncGeometry(const Sketch& sketch) const;
    bool createDrawingStub(const std::string& name) const;
    const std::string& activeDocument() const;

    Part buildPartFromSketch(const Sketch& sketch) const;
    bool isAvailable() const;
    
    // Part feature synchronization
    bool syncExtrude(const Part& part, const std::string& feature_name) const;
    bool syncRevolve(const Part& part, const std::string& feature_name) const;
    bool syncHole(const Part& part, const std::string& feature_name) const;
    bool syncFillet(const Part& part, const std::string& feature_name) const;
    bool syncLoft(const Part& part, const std::string& feature_name) const;

private:
    bool initialized_{false};
    std::string active_document_;
};

}  // namespace core
}  // namespace cad
