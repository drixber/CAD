#include "PatternService.h"

namespace cad {
namespace modules {

PatternResult PatternService::createPattern(const PatternRequest& request) const {
    PatternResult result;
    if (!request.targetFeature.empty()) {
        result.success = true;
        result.message = "Pattern queued";
    }
    return result;
}

}  // namespace modules
}  // namespace cad
