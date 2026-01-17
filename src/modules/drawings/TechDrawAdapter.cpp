#include "TechDrawAdapter.h"

namespace cad {
namespace drawings {

bool TechDrawAdapter::initialize() {
    return true;
}

bool TechDrawAdapter::createSheet(const DrawingSheet& sheet) {
    (void)sheet;
    return true;
}

bool TechDrawAdapter::createView(const DrawingView& view) {
    (void)view;
    return true;
}

bool TechDrawAdapter::applyAnnotations(const DrawingDocument& document) {
    (void)document;
    return true;
}

bool TechDrawAdapter::applyDimensions(const DrawingDocument& document) {
    (void)document;
    return true;
}

bool TechDrawAdapter::syncAssociativeLinks(const DrawingDocument& document) {
    (void)document;
    return true;
}

}  // namespace drawings
}  // namespace cad
