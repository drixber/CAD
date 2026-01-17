#pragma once

#include "CADApplication.h"
#include "core/Modeler/Modeler.h"
#include "core/FreeCAD/FreeCADAdapter.h"
#include "core/FreeCAD/TechDrawBridge.h"
#include "core/analysis/InterferenceChecker.h"
#include "core/assembly/AssemblyManager.h"
#include "core/perf/PerfSpan.h"
#include "ui/MainWindow.h"
#include "modules/drawings/DrawingService.h"
#include "modules/drawings/BomService.h"
#include "modules/drawings/AnnotationService.h"
#include "modules/drawings/AssociativeLinkService.h"
#include "modules/sheetmetal/SheetMetalService.h"
#include "modules/simulation/SimulationService.h"
#include "modules/patterns/PatternService.h"
#include "modules/direct/DirectEditService.h"
#include "modules/routing/RoutingService.h"
#include "modules/visualization/VisualizationService.h"
#include "modules/mbd/MbdService.h"
#include "modules/simplify/SimplifyService.h"
#include "interop/ImportExportService.h"
#include "interop/IoPipeline.h"

namespace cad {
namespace app {

class AppController {
public:
    AppController();

    void initialize();
    void setActiveSketch(const cad::core::Sketch& sketch);
    cad::ui::MainWindow& mainWindow();
    cad::core::Modeler& modeler();
    cad::core::FreeCADAdapter& freecad();

private:
    std::string buildParameterSummary(const cad::core::Sketch& sketch) const;
    void initializeAssembly();
    void bindCommands();
    void executeCommand(const std::string& command);

    cad::ui::MainWindow main_window_;
    cad::core::Modeler modeler_;
    cad::core::FreeCADAdapter freecad_;
    cad::core::TechDrawBridge techdraw_bridge_;
    cad::core::InterferenceChecker interference_checker_;
    cad::core::AssemblyManager assembly_manager_;
    cad::modules::DrawingService drawing_service_;
    cad::drawings::BomService bom_service_;
    cad::drawings::AnnotationService annotation_service_;
    cad::drawings::AssociativeLinkService associative_link_service_;
    cad::modules::SheetMetalService sheet_metal_service_;
    cad::modules::SimulationService simulation_service_;
    cad::modules::PatternService pattern_service_;
    cad::modules::DirectEditService direct_edit_service_;
    cad::modules::RoutingService routing_service_;
    cad::modules::VisualizationService visualization_service_;
    cad::modules::MbdService mbd_service_;
    cad::modules::SimplifyService simplify_service_;
    cad::interop::ImportExportService io_service_;
    cad::interop::IoPipeline io_pipeline_;
    cad::core::Sketch active_sketch_{"Sketch"};
    cad::core::Assembly active_assembly_;
};

}  // namespace app
}  // namespace cad
