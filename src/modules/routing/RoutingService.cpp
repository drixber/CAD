#include "RoutingService.h"

namespace cad {
namespace modules {

RoutingResult RoutingService::createRoute(const RoutingRequest& request) const {
    RoutingResult result;
    if (!request.targetAssembly.empty()) {
        result.success = true;
        result.message = "Routing queued";
    }
    return result;
}

}  // namespace modules
}  // namespace cad
