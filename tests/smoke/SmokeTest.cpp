#include "kernel/KernelFacade.h"
#include "core/perf/PerformanceMonitor.h"
#include "core/analysis/InterferenceChecker.h"
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

    cad::core::PerformanceMonitor monitor;
    monitor.recordFrame(16.0);
    if (monitor.fps() <= 0.0) {
        return 1;
    }

    cad::core::InterferenceChecker checker;
    if (checker.check("AssemblyA").message.empty()) {
        return 1;
    }

    return 0;
}
