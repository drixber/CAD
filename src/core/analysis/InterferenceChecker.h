#pragma once

#include <string>
#include <vector>

#include "../Modeler/Assembly.h"

namespace cad {
namespace core {

struct BoundingBox {
    double min_x{0.0};
    double min_y{0.0};
    double min_z{0.0};
    double max_x{0.0};
    double max_y{0.0};
    double max_z{0.0};
};

struct InterferencePair {
    std::uint64_t component_a_id{0};
    std::uint64_t component_b_id{0};
    std::string part_a_name;
    std::string part_b_name;
    double overlap_volume{0.0};
};

struct InterferenceResult {
    bool has_interference{false};
    int overlap_count{0};
    std::string message;
    std::vector<InterferencePair> interference_pairs;
};

class InterferenceChecker {
public:
    InterferenceResult check(const std::string& assembly_id) const;
    InterferenceResult checkAssembly(const Assembly& assembly) const;
    
private:
    BoundingBox estimateBoundingBox(const Part& part, const Transform& transform) const;
    bool boxesOverlap(const BoundingBox& a, const BoundingBox& b) const;
    double calculateOverlapVolume(const BoundingBox& a, const BoundingBox& b) const;
};

}  // namespace core
}  // namespace cad
