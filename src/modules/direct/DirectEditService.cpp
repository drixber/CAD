#include "DirectEditService.h"

namespace cad {
namespace modules {

DirectEditResult DirectEditService::applyEdit(const DirectEditRequest& request) const {
    DirectEditResult result;
    if (!request.targetFeature.empty()) {
        result.success = true;
        result.message = "Direct edit queued";
    }
    return result;
}

}  // namespace modules
}  // namespace cad
