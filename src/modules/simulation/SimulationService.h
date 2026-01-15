#pragma once

#include <string>

namespace cad {
namespace modules {

enum class SimulationType {
    Motion,
    FEA,
    Deflection,
    Optimization
};

struct SimulationRequest {
    std::string targetAssembly;
    SimulationType type;
};

struct SimulationResult {
    bool success{false};
    std::string message;
};

class SimulationService {
public:
    SimulationResult runSimulation(const SimulationRequest& request) const;
};

}  // namespace modules
}  // namespace cad
