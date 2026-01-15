#include "SimplifyService.h"

namespace cad {
namespace modules {

SimplifyResult SimplifyService::simplify(const SimplifyRequest& request) const {
    SimplifyResult result;
    if (!request.targetAssembly.empty()) {
        result.success = true;
        result.message = "Simplify queued";
    }
    return result;
}

}  // namespace modules
}  // namespace cad
