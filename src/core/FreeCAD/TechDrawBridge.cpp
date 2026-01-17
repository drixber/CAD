#include "TechDrawBridge.h"

namespace cad {
namespace core {

bool TechDrawBridge::initialize() {
    return true;
}

bool TechDrawBridge::syncDrawing(const cad::drawings::DrawingDocument& document) {
    (void)document;
    return true;
}

bool TechDrawBridge::syncAssociativeLinks(const cad::drawings::DrawingDocument& document) {
    (void)document;
    return true;
}

bool TechDrawBridge::syncDimensions(const cad::drawings::DrawingDocument& document) {
    (void)document;
    return true;
}

}  // namespace core
}  // namespace cad
