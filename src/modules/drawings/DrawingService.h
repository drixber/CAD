#pragma once

#include <string>

#include "model/DrawingDocument.h"

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
    DrawingDocument buildDocumentSkeleton(const std::string& title) const;
    DrawingStyleSet defaultStyles() const;
};

}  // namespace modules
}  // namespace cad
