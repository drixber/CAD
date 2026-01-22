#include "DrawingService.h"

namespace cad {
namespace modules {

using namespace cad::drawings;

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
    DrawingStyleProfile profile;
    profile.name = "Default";
    profile.line_style = "Visible";
    profile.text_style = "Annotation";
    profile.dimension_style = "Standard";
    profile.hatch_style = "Solid";
    document.profiles.push_back(profile);
    DrawingSheet sheet;
    sheet.name = "Sheet1";
    sheet.template_name = "A3";
    sheet.associative = true;
    sheet.scale_label = "1:1";
    sheet.views.push_back({"Front", "Front", 1.0, document.source_model_id, true, "Default"});
    sheet.views.push_back({"Top", "Top", 1.0, document.source_model_id, true, "Default"});
    sheet.views.push_back({"Side", "Right", 1.0, document.source_model_id, true, "Default"});
    sheet.views.push_back({"Iso", "Isometric", 1.0, document.source_model_id, true, "Default"});
    document.sheets.push_back(sheet);
    return document;
}

DrawingStyleSet DrawingService::defaultStyles() const {
    return createStylePreset(StylePreset::Default);
}

DrawingStyleSet DrawingService::createStylePreset(StylePreset preset) const {
    switch (preset) {
        case StylePreset::ISO:
            return isoStyles();
        case StylePreset::ANSI:
            return ansiStyles();
        case StylePreset::JIS:
            return jisStyles();
        case StylePreset::Default:
        case StylePreset::Custom:
        default:
            return defaultStyleSet();
    }
}

DrawingStyleSet DrawingService::isoStyles() const {
    DrawingStyleSet styles;
    styles.name = "ISO";
    
    // ISO line styles (thickness in mm)
    styles.line_styles.push_back(createLineStyle("Visible", 0.5, LineType::Solid, "black"));
    styles.line_styles.push_back(createLineStyle("Hidden", 0.25, LineType::Dashed, "black"));
    styles.line_styles.push_back(createLineStyle("Center", 0.25, LineType::CenterLine, "black"));
    styles.line_styles.push_back(createLineStyle("Phantom", 0.25, LineType::Phantom, "black"));
    
    // ISO text styles
    styles.text_styles.push_back(createTextStyle("Annotation", 2.5, "Arial", FontWeight::Normal));
    styles.text_styles.push_back(createTextStyle("Title", 5.0, "Arial", FontWeight::Bold));
    styles.text_styles.push_back(createTextStyle("Dimension", 2.5, "Arial", FontWeight::Normal));
    
    // ISO dimension style
    styles.dimension_styles.push_back(createDimensionStyle("Standard", 2.5, 3.0));
    
    // ISO hatch styles
    HatchStyle solid_hatch;
    solid_hatch.name = "Solid";
    solid_hatch.pattern = "Solid";
    solid_hatch.color = "black";
    styles.hatch_styles.push_back(solid_hatch);
    
    HatchStyle steel_hatch;
    steel_hatch.name = "Steel";
    steel_hatch.pattern = "ANSI31";
    steel_hatch.scale = 1.0;
    steel_hatch.angle = 45.0;
    steel_hatch.color = "black";
    styles.hatch_styles.push_back(steel_hatch);
    
    return styles;
}

DrawingStyleSet DrawingService::ansiStyles() const {
    DrawingStyleSet styles;
    styles.name = "ANSI";
    
    // ANSI line styles (thickness in inches, converted to mm)
    styles.line_styles.push_back(createLineStyle("Visible", 0.014, LineType::Solid, "black"));  // 0.014" ≈ 0.35mm
    styles.line_styles.push_back(createLineStyle("Hidden", 0.007, LineType::Dashed, "black"));  // 0.007" ≈ 0.18mm
    styles.line_styles.push_back(createLineStyle("Center", 0.007, LineType::CenterLine, "black"));
    styles.line_styles.push_back(createLineStyle("Phantom", 0.007, LineType::Phantom, "black"));
    
    // ANSI text styles (heights in inches)
    styles.text_styles.push_back(createTextStyle("Annotation", 0.1, "Arial", FontWeight::Normal));  // 0.1" ≈ 2.5mm
    styles.text_styles.push_back(createTextStyle("Title", 0.14, "Arial", FontWeight::Bold));  // 0.14" ≈ 3.5mm
    styles.text_styles.push_back(createTextStyle("Dimension", 0.1, "Arial", FontWeight::Normal));
    
    // ANSI dimension style
    DimensionStyle dim_style;
    dim_style.name = "Standard";
    dim_style.text_height = 0.1;
    dim_style.arrow_size = 0.12;
    dim_style.units = "in";
    dim_style.decimal_places = 3;
    styles.dimension_styles.push_back(dim_style);
    
    // ANSI hatch styles
    HatchStyle solid_hatch;
    solid_hatch.name = "Solid";
    solid_hatch.pattern = "Solid";
    styles.hatch_styles.push_back(solid_hatch);
    
    return styles;
}

DrawingStyleSet DrawingService::jisStyles() const {
    DrawingStyleSet styles;
    styles.name = "JIS";
    
    // JIS line styles (similar to ISO but with JIS-specific values)
    styles.line_styles.push_back(createLineStyle("Visible", 0.5, LineType::Solid, "black"));
    styles.line_styles.push_back(createLineStyle("Hidden", 0.25, LineType::Dashed, "black"));
    styles.line_styles.push_back(createLineStyle("Center", 0.25, LineType::CenterLine, "black"));
    
    // JIS text styles (typically use Japanese fonts, but Arial as fallback)
    styles.text_styles.push_back(createTextStyle("Annotation", 2.5, "Arial", FontWeight::Normal));
    styles.text_styles.push_back(createTextStyle("Title", 5.0, "Arial", FontWeight::Bold));
    
    // JIS dimension style
    styles.dimension_styles.push_back(createDimensionStyle("Standard", 2.5, 3.0));
    
    // JIS hatch styles
    HatchStyle solid_hatch;
    solid_hatch.name = "Solid";
    solid_hatch.pattern = "Solid";
    styles.hatch_styles.push_back(solid_hatch);
    
    return styles;
}

LineStyle DrawingService::createLineStyle(const std::string& name, double thickness, LineType type, const std::string& color) const {
    LineStyle style;
    style.name = name;
    style.thickness = thickness;
    style.line_type = type;
    style.color = color;
    style.pattern_scale = 1.0;
    return style;
}

TextStyle DrawingService::createTextStyle(const std::string& name, double size, const std::string& font, FontWeight weight) const {
    TextStyle style;
    style.name = name;
    style.size = size;
    style.font_family = font;
    style.weight = weight;
    style.italic = false;
    style.color = "black";
    return style;
}

DimensionStyle DrawingService::createDimensionStyle(const std::string& name, double text_height, double arrow_size) const {
    DimensionStyle style;
    style.name = name;
    style.text_height = text_height;
    style.arrow_size = arrow_size;
    style.extension_line_offset = 1.0;
    style.extension_line_extension = 2.0;
    style.units = "mm";
    style.decimal_places = 2;
    style.color = "black";
    return style;
}

DrawingStyleSet DrawingService::defaultStyleSet() const {
    DrawingStyleSet styles;
    styles.name = "Default";
    
    styles.line_styles.push_back(createLineStyle("Visible", 0.35, LineType::Solid, "black"));
    styles.line_styles.push_back(createLineStyle("Hidden", 0.18, LineType::Dashed, "black"));
    
    styles.text_styles.push_back(createTextStyle("Annotation", 2.5, "Arial", FontWeight::Normal));
    styles.text_styles.push_back(createTextStyle("Title", 3.5, "Arial", FontWeight::Bold));
    
    styles.dimension_styles.push_back(createDimensionStyle("Standard", 2.5, 2.0));
    
    HatchStyle solid_hatch;
    solid_hatch.name = "Solid";
    solid_hatch.pattern = "Solid";
    styles.hatch_styles.push_back(solid_hatch);
    
    return styles;
}

}  // namespace modules
}  // namespace cad
