#pragma once

#include <string>
#include <vector>

namespace cad {
namespace mbd {

struct PmiDatum {
    std::string id;
    std::string type;
    std::string description;
};

struct PmiTolerance {
    std::string label;
    double upper{0.0};
    double lower{0.0};
    std::string units;
};

enum class PmiAnnotationVisibility {
    Always,
    OnSelection,
    OnHover,
    Hidden
};

struct PmiAnnotation {
    std::string text;
    double x{0.0};
    double y{0.0};
    double z{0.0};
    PmiAnnotationVisibility visibility{PmiAnnotationVisibility::Always};
    double font_size{2.5};
    std::string color{"black"};
    bool show_leader{true};
    double leader_x{0.0};
    double leader_y{0.0};
    double leader_z{0.0};
};

struct PmiDataSet {
    std::string part_id;
    std::vector<PmiDatum> datums;
    std::vector<PmiTolerance> tolerances;
    std::vector<PmiAnnotation> annotations;
};

}  // namespace mbd
}  // namespace cad
