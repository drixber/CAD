#pragma once

#include <string>
#include <vector>

namespace cad {
namespace drawings {

enum class LineType {
    Solid,
    Dashed,
    Dotted,
    DashDot,
    DashDotDot,
    CenterLine,
    Phantom
};

enum class FontWeight {
    Normal,
    Bold,
    Light
};

struct LineStyle {
    std::string name;
    double thickness{0.25};
    LineType line_type{LineType::Solid};
    std::string color{"black"};
    double pattern_scale{1.0};
};

struct TextStyle {
    std::string name;
    double size{2.5};
    std::string font_family{"Arial"};
    FontWeight weight{FontWeight::Normal};
    bool italic{false};
    std::string color{"black"};
};

struct DimensionStyle {
    std::string name;
    double text_height{2.5};
    double arrow_size{2.0};
    double extension_line_offset{1.0};
    double extension_line_extension{2.0};
    std::string units{"mm"};
    int decimal_places{2};
    std::string color{"black"};
};

struct HatchStyle {
    std::string name;
    std::string pattern{"Solid"};
    double scale{1.0};
    double angle{0.0};
    std::string color{"black"};
};

struct DrawingStyleSet {
    std::string name;
    std::vector<LineStyle> line_styles;
    std::vector<TextStyle> text_styles;
    std::vector<DimensionStyle> dimension_styles;
    std::vector<HatchStyle> hatch_styles;
};

struct DrawingStyleProfile {
    std::string name;
    std::string line_style;
    std::string text_style;
    std::string dimension_style;
    std::string hatch_style;
};

enum class StylePreset {
    Default,
    ISO,
    ANSI,
    JIS,
    Custom
};

}  // namespace drawings
}  // namespace cad
