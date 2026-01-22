#pragma once

#include "modules/drawings/model/DrawingDocument.h"
#include <vector>
#include <string>

namespace cad {
namespace core {

class TechDrawBridge {
public:
    bool initialize();
    bool syncDrawing(const cad::drawings::DrawingDocument& document);
    bool syncAssociativeLinks(const cad::drawings::DrawingDocument& document);
    bool syncDimensions(const cad::drawings::DrawingDocument& document);

private:
    std::vector<double> parseOrientation(const std::string& orientation) const;
};

}  // namespace core
}  // namespace cad
