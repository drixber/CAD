#include "kernel/KernelFacade.h"
#include "core/perf/PerformanceMonitor.h"
#include "core/analysis/InterferenceChecker.h"
#include "interop/IoPipeline.h"
#include "simulation/SimulationKernel.h"
#include "core/assembly/AssemblyManager.h"

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

    cad::interop::IoPipeline pipeline;
    cad::interop::IoJob job;
    job.path = "C:/temp/model.step";
    job.format = "STEP";
    if (!pipeline.importJob(job).success) {
        return 1;
    }

    cad::core::AssemblyManager manager;
    manager.setCacheLimit(100);
    manager.enableBackgroundLoading(true);
    cad::core::AssemblyLoadStats load_stats = manager.loadAssembly("AssemblyA");
    cad::core::CacheStats cache_stats = manager.cacheStats();
    if (cache_stats.max_entries == 0) {
        return 1;
    }
    if (!load_stats.used_background_loading) {
        return 1;
    }
    manager.enqueueLoad("AssemblyA");
    cad::core::AssemblyLoadJob job = manager.pollLoadProgress();
    if (job.path.empty()) {
        return 1;
    }

    return 0;
}
