#pragma once

#include <string>
#include <vector>

namespace cad {
namespace drawings {

struct LineStyle {
    std::string name;
    double thickness{0.25};
};

struct TextStyle {
    std::string name;
    double size{2.5};
};

struct DrawingStyleSet {
    std::string name;
    std::vector<LineStyle> line_styles;
    std::vector<TextStyle> text_styles;
};

struct DrawingStyleProfile {
    std::string name;
    std::string line_style;
    std::string text_style;
};

}  // namespace drawings
}  // namespace cad
