#include "VisualizationService.h"

namespace cad {
namespace modules {

VisualizationResult VisualizationService::runVisualization(const VisualizationRequest& request) const {
    VisualizationResult result;
    if (!request.targetPart.empty()) {
        result.success = true;
        result.message = "Visualization queued";
    }
    return result;
}

}  // namespace modules
}  // namespace cad
