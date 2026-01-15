#include "SimulationService.h"

namespace cad {
namespace modules {

SimulationResult SimulationService::runSimulation(const SimulationRequest& request) const {
    SimulationResult result;
    if (!request.targetAssembly.empty()) {
        result.success = true;
        result.message = "Simulation queued";
    }
    return result;
}

}  // namespace modules
}  // namespace cad
