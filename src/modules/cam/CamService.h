#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** CAM / Manufacture (§18.14): 2D/3D/5-Achs, Adaptive, Contour, Pocket, Drilling, Turning; G-Code. Stub. */
class CamService {
public:
    struct ToolSpec {
        std::string id;
        double diameter_mm{0.0};
        std::string type{"flat"};
    };
    bool addTool(const ToolSpec& tool);
    std::string generateToolpath(const std::string& part_id, const std::string& strategy);
    bool exportGCode(const std::string& toolpath_id, const std::string& file_path);
};

}  // namespace modules
}  // namespace cad
