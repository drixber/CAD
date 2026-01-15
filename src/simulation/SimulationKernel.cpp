#include "SimulationKernel.h"

#include "core/logging/Logger.h"

namespace cad {
namespace simulation {

bool SimulationKernel::initialize() {
    cad::core::Logger::instance().log(cad::core::Logger::Level::Info,
                                      "SimulationKernel initialized");
    return true;
}

void SimulationKernel::setWorkingDirectory(const std::string& path) {
    working_directory_ = path;
    cad::core::Logger::instance().log(cad::core::Logger::Level::Debug,
                                      "Simulation working directory set");
}

}  // namespace simulation
}  // namespace cad
