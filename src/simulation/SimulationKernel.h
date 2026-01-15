#pragma once

#include <string>

namespace cad {
namespace simulation {

class SimulationKernel {
public:
    bool initialize();
    void setWorkingDirectory(const std::string& path);

private:
    std::string working_directory_{};
};

}  // namespace simulation
}  // namespace cad
