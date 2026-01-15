#pragma once

#include <string>

namespace cad {
namespace modules {

enum class RoutingType {
    RigidPipe,
    FlexibleHose,
    BentTube
};

struct RoutingRequest {
    std::string targetAssembly;
    RoutingType type;
};

struct RoutingResult {
    bool success{false};
    std::string message;
};

class RoutingService {
public:
    RoutingResult createRoute(const RoutingRequest& request) const;
};

}  // namespace modules
}  // namespace cad
