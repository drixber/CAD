#pragma once

#include <string>

#include "model/DrawingDocument.h"

namespace cad {
namespace drawings {

class TechDrawAdapter {
public:
    bool initialize();
    bool createSheet(const DrawingSheet& sheet);
    bool createView(const DrawingView& view);
    bool applyAnnotations(const DrawingDocument& document);
    bool syncAssociativeLinks(const DrawingDocument& document);
    bool applyDimensions(const DrawingDocument& document);
};

}  // namespace drawings
}  // namespace cad
