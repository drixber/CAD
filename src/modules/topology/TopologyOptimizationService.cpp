#include "TopologyOptimizationService.h"

namespace cad {
namespace modules {

TopologyOptimizationService::OptimizationResult
TopologyOptimizationService::run(const std::string& part_id, const OptimizationParams& params) const {
    (void)part_id;
    (void)params;
    OptimizationResult r;
    r.success = false;
    r.message = "Topology optimization stub (not implemented)";
    return r;
}

}  // namespace modules
}  // namespace cad
