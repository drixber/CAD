#include "SheetMetalService.h"

namespace cad {
namespace modules {

SheetMetalResult SheetMetalService::applyOperation(const SheetMetalRequest& request) const {
    SheetMetalResult result;
    if (!request.targetPart.empty()) {
        result.success = true;
        result.message = "Sheet metal op queued";
    }
    return result;
}

}  // namespace modules
}  // namespace cad
