#include "DrawingService.h"

namespace cad {
namespace modules {

DrawingResult DrawingService::createDrawing(const DrawingRequest& request) const {
    DrawingResult result;
    if (!request.sourcePart.empty()) {
        result.drawingId = request.sourcePart + "_Drawing";
        result.success = true;
    }
    return result;
}

}  // namespace modules
}  // namespace cad
