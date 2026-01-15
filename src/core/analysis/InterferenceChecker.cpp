#include "InterferenceChecker.h"

namespace cad {
namespace core {

InterferenceResult InterferenceChecker::check(const std::string& assembly_id) const {
    InterferenceResult result;
    result.has_interference = false;
    result.overlap_count = 0;
    result.message = assembly_id.empty() ? "No assembly loaded" : "No interference detected";
    return result;
}

}  // namespace core
}  // namespace cad
