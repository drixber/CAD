#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Surface Workspace (§18.5): Patch, Extend, Trim, Stitch, Thicken, Loft/Boundary als Fläche. Stub. */
class SurfaceService {
public:
    bool patch(const std::string& profile_id);
    bool trim(const std::string& surface_id, const std::string& trim_curve_id);
    bool stitch(const std::vector<std::string>& surface_ids);
    bool thicken(const std::string& surface_id, double thickness_mm);
};

}  // namespace modules
}  // namespace cad
