#pragma once

#include <string>
#include <vector>

#include "DrawingStyles.h"

namespace cad {
namespace drawings {

struct DrawingView {
    std::string name;
    std::string orientation;
    double scale{1.0};
    std::string source_model_id;
    bool associative{true};
    std::string style_profile{"Default"};
};

struct DrawingSheet {
    std::string name;
    std::string template_name;
    std::vector<DrawingView> views;
    bool associative{true};
    std::string scale_label{"1:1"};
};

enum class AnnotationType {
    Text,
    Note,
    Callout,
    Leader,
    Balloon,
    Revision,
    Title
};

enum class TextAlignment {
    Left,
    Center,
    Right,
    Justified
};

struct Annotation {
    std::string text;
    double x{0.0};
    double y{0.0};
    AnnotationType type{AnnotationType::Text};
    TextAlignment alignment{TextAlignment::Left};
    double rotation{0.0};
    std::string style_name{"Annotation"};
    double font_size{2.5};
    std::string view_name;
    bool attached_to_view{false};
};

struct Dimension {
    std::string label;
    double value{0.0};
    std::string units;
    std::string tolerance;
};

struct BillOfMaterialsItem {
    std::string part_name;
    int quantity{1};
    std::string part_number;
};

struct DrawingDocument {
    std::string title;
    DrawingStyleSet styles;
    std::vector<DrawingStyleProfile> profiles;
    std::vector<DrawingSheet> sheets;
    std::vector<Annotation> annotations;
    std::vector<Dimension> dimensions;
    std::vector<BillOfMaterialsItem> bom;
    std::string source_model_id;
    int revision{0};
    std::string last_update_id;
};

}  // namespace drawings
}  // namespace cad
