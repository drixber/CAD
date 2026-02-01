#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Freiform / T-Splines (§17.8, §18.6): Subdivision, Ziehen, Glätten, Verdicken zu Solid. Stub. */
class FreeformService {
public:
    std::string createFromPrimitive(const std::string& type);
    bool pull(const std::string& body_id, int face_index, double dx, double dy, double dz);
    bool smooth(const std::string& body_id);
    bool thickenToSolid(const std::string& body_id, double thickness_mm);
};

}  // namespace modules
}  // namespace cad
