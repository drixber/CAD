#pragma once

#include "modules/drawings/model/DrawingDocument.h"

namespace cad {
namespace core {

class TechDrawBridge {
public:
    bool initialize();
    bool syncDrawing(const cad::drawings::DrawingDocument& document);
    bool syncAssociativeLinks(const cad::drawings::DrawingDocument& document);
    bool syncDimensions(const cad::drawings::DrawingDocument& document);
};

}  // namespace core
}  // namespace cad
