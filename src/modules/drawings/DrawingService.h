#pragma once

#include <string>

#include "model/DrawingDocument.h"
#include "model/DrawingStyles.h"

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
    cad::drawings::DrawingDocument buildDocumentSkeleton(const std::string& title) const;
    cad::drawings::DrawingStyleSet defaultStyles() const;
    cad::drawings::DrawingStyleSet createStylePreset(cad::drawings::StylePreset preset) const;
    cad::drawings::DrawingStyleSet isoStyles() const;
    cad::drawings::DrawingStyleSet ansiStyles() const;
    cad::drawings::DrawingStyleSet jisStyles() const;
    
private:
    cad::drawings::DrawingStyleSet defaultStyleSet() const;
    cad::drawings::LineStyle createLineStyle(const std::string& name, double thickness, cad::drawings::LineType type, const std::string& color = "black") const;
    cad::drawings::TextStyle createTextStyle(const std::string& name, double size, const std::string& font = "Arial", cad::drawings::FontWeight weight = cad::drawings::FontWeight::Normal) const;
    cad::drawings::DimensionStyle createDimensionStyle(const std::string& name, double text_height, double arrow_size) const;
};

}  // namespace modules
}  // namespace cad
