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

DrawingDocument DrawingService::buildDocumentSkeleton(const std::string& title) const {
    DrawingDocument document;
    document.title = title;
    document.source_model_id = title;
    document.styles = defaultStyles();
    DrawingSheet sheet;
    sheet.name = "Sheet1";
    sheet.template_name = "A3";
    sheet.views.push_back({"Front", "Front", 1.0, document.source_model_id});
    sheet.views.push_back({"Top", "Top", 1.0, document.source_model_id});
    sheet.views.push_back({"Side", "Right", 1.0, document.source_model_id});
    sheet.views.push_back({"Iso", "Isometric", 1.0, document.source_model_id});
    document.sheets.push_back(sheet);
    return document;
}

DrawingStyleSet DrawingService::defaultStyles() const {
    DrawingStyleSet styles;
    styles.name = "Default";
    styles.line_styles.push_back({"Visible", 0.35});
    styles.line_styles.push_back({"Hidden", 0.18});
    styles.text_styles.push_back({"Annotation", 2.5});
    styles.text_styles.push_back({"Title", 3.5});
    return styles;
}

}  // namespace modules
}  // namespace cad
