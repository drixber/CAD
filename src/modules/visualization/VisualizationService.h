#pragma once

#include <string>

namespace cad {
namespace modules {

enum class VisualizationMode {
    Illustration,
    Rendering,
    Animation
};

struct VisualizationRequest {
    std::string targetPart;
    VisualizationMode mode;
};

struct VisualizationResult {
    bool success{false};
    std::string message;
};

class VisualizationService {
public:
    VisualizationResult runVisualization(const VisualizationRequest& request) const;
};

}  // namespace modules
}  // namespace cad
