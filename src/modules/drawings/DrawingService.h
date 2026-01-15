#pragma once

#include <string>

namespace cad {
namespace modules {

struct DrawingRequest {
    std::string sourcePart;
    std::string templateName;
};

struct DrawingResult {
    std::string drawingId;
    bool success{false};
};

class DrawingService {
public:
    DrawingResult createDrawing(const DrawingRequest& request) const;
};

}  // namespace modules
}  // namespace cad
