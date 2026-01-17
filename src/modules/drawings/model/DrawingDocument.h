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
};

struct DrawingSheet {
    std::string name;
    std::string template_name;
    std::vector<DrawingView> views;
};

struct Annotation {
    std::string text;
    double x{0.0};
    double y{0.0};
};

struct Dimension {
    std::string label;
    double value{0.0};
    std::string units;
};

struct BillOfMaterialsItem {
    std::string part_name;
    int quantity{1};
};

struct DrawingDocument {
    std::string title;
    DrawingStyleSet styles;
    std::vector<DrawingSheet> sheets;
    std::vector<Annotation> annotations;
    std::vector<Dimension> dimensions;
    std::vector<BillOfMaterialsItem> bom;
    std::string source_model_id;
};

}  // namespace drawings
}  // namespace cad
