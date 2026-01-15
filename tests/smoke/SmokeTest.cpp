#include "kernel/KernelFacade.h"
#include "simulation/SimulationKernel.h"

int main() {
    cad::kernel::KernelFacade kernel;
    if (!kernel.initialize()) {
        return 1;
    }

    cad::simulation::SimulationKernel simulation;
    if (!simulation.initialize()) {
        return 1;
    }

    return 0;
}
