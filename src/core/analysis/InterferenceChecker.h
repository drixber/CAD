#pragma once

#include <string>

namespace cad {
namespace core {

struct InterferenceResult {
    bool has_interference{false};
    int overlap_count{0};
    std::string message;
};

class InterferenceChecker {
public:
    InterferenceResult check(const std::string& assembly_id) const;
};

}  // namespace core
}  // namespace cad
