#pragma once

#include <string>
#include <vector>

#include "DrawingStyles.h"

namespace cad {
namespace drawings {

/** Blattformat (ISO/DIN/ANSI): Name, Breite/Höhe in mm, Ausrichtung. */
struct SheetFormat {
    std::string name;
    double width_mm{297.0};
    double height_mm{210.0};
    bool landscape{true};
    bool has_border{true};
    bool has_title_block{true};
};

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
    std::string format_id;
    double width_mm{0.0};
    double height_mm{0.0};
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

struct LeaderPoint {
    double x{0.0};
    double y{0.0};
};

struct AttachmentPoint {
    double x{0.0};
    double y{0.0};
    std::string entity_id;
    bool snap_to_geometry{true};
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
    std::vector<LeaderPoint> leader_points;
    AttachmentPoint attachment_point;
    bool has_leader{false};
    bool has_attachment{false};
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
