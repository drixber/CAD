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

private:
    bool initialized_{false};
    std::string active_document_;
};

}  // namespace core
}  // namespace cad
