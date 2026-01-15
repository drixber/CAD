#include "MbdService.h"

namespace cad {
namespace modules {

MbdResult MbdService::applyMbd(const MbdRequest& request) const {
    MbdResult result;
    if (!request.targetPart.empty()) {
        result.success = true;
        result.message = "MBD applied";
    }
    return result;
}

}  // namespace modules
}  // namespace cad
