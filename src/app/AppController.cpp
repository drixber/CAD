#include "AppController.h"

#include <sstream>

namespace cad {
namespace app {

AppController::AppController() = default;

void AppController::initialize() {
    main_window_.initializeLayout();
    if (freecad_.initializeSession()) {
        freecad_.createDocument("MainDocument");
        main_window_.setIntegrationStatus("FreeCAD on");
    } else {
        main_window_.setIntegrationStatus("FreeCAD off");
    }
    cad::core::Sketch sketch("Sketch1");
    sketch.addConstraint({cad::core::ConstraintType::Distance, "line1", "line2", 25.0});
    sketch.addParameter({"Width", 0.0, "100"});
    sketch.addParameter({"Height", 0.0, "50"});
    setActiveSketch(sketch);
    modeler_.evaluateParameters(active_sketch_);
    main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
    main_window_.setParameterCount(static_cast<int>(active_sketch_.parameters().size()));
    main_window_.setParameterSummary(buildParameterSummary(active_sketch_));
    if (freecad_.isAvailable()) {
        freecad_.syncSketch(active_sketch_);
        freecad_.syncConstraints(active_sketch_);
    }
    initializeAssembly();
    bindCommands();
}

void AppController::setActiveSketch(const cad::core::Sketch& sketch) {
    active_sketch_ = sketch;
}

cad::ui::MainWindow& AppController::mainWindow() {
    return main_window_;
}

cad::core::Modeler& AppController::modeler() {
    return modeler_;
}

cad::core::FreeCADAdapter& AppController::freecad() {
    return freecad_;
}

std::string AppController::buildParameterSummary(const cad::core::Sketch& sketch) const {
    std::ostringstream summary;
    bool first = true;
    for (const auto& parameter : sketch.parameters()) {
        if (!first) {
            summary << ", ";
        }
        first = false;
        summary << parameter.name << "=" << parameter.value;
    }
    return summary.str();
}

void AppController::initializeAssembly() {
    active_assembly_ = modeler_.createAssembly();
    cad::core::Part partA("Bracket");
    cad::core::Part partB("Plate");
    cad::core::Transform transformA;
    cad::core::Transform transformB;
    transformB.tx = 10.0;
    std::uint64_t idA = active_assembly_.addComponent(partA, transformA);
    std::uint64_t idB = active_assembly_.addComponent(partB, transformB);
    active_assembly_.addMate({idA, idB, cad::core::MateType::Mate, 0.0});
    main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
    main_window_.setAssemblySummary("2 components, 1 mate");
    main_window_.setMatesSummary("1 mate");
}

void AppController::bindCommands() {
    main_window_.setCommandHandler([this](const std::string& command) {
        executeCommand(command);
    });
}

void AppController::executeCommand(const std::string& command) {
    if (command == "Parameters") {
        main_window_.setParameterSummary(buildParameterSummary(active_sketch_));
    } else if (command == "Flange" || command == "Bend" || command == "Unfold" || command == "Refold") {
        cad::modules::SheetMetalRequest request;
        request.targetPart = "Bracket";
        request.operation = cad::modules::SheetMetalOperation::Flange;
        cad::modules::SheetMetalResult result = sheet_metal_service_.applyOperation(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Rectangular Pattern" || command == "Circular Pattern" || command == "Curve Pattern") {
        cad::modules::PatternRequest request;
        request.targetFeature = "Hole1";
        request.type = cad::modules::PatternType::Rectangular;
        request.instanceCount = 6;
        cad::modules::PatternResult result = pattern_service_.createPattern(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Direct Edit" || command == "Freeform") {
        cad::modules::DirectEditRequest request;
        request.targetFeature = "Face1";
        request.operation = cad::modules::DirectEditOperation::MoveFace;
        cad::modules::DirectEditResult result = direct_edit_service_.applyEdit(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Rigid Pipe" || command == "Flexible Hose" || command == "Bent Tube") {
        cad::modules::RoutingRequest request;
        request.targetAssembly = "MainAssembly";
        request.type = cad::modules::RoutingType::RigidPipe;
        cad::modules::RoutingResult result = routing_service_.createRoute(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Simplify") {
        cad::modules::SimplifyRequest request;
        request.targetAssembly = "MainAssembly";
        request.replacementType = "Cylinder";
        cad::modules::SimplifyResult result = simplify_service_.simplify(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Illustration" || command == "Rendering" || command == "Animation") {
        cad::modules::VisualizationRequest request;
        request.targetPart = "Bracket";
        request.mode = cad::modules::VisualizationMode::Rendering;
        cad::modules::VisualizationResult result = visualization_service_.runVisualization(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "MBD Note" || command == "MbdNote") {
        cad::modules::MbdRequest request;
        request.targetPart = "Bracket";
        request.note = "GD&T: profile tolerance";
        cad::modules::MbdResult result = mbd_service_.applyMbd(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Simulation" || command == "Stress Analysis") {
        cad::modules::SimulationRequest request;
        request.targetAssembly = "MainAssembly";
        request.type = cad::modules::SimulationType::FEA;
        cad::modules::SimulationResult result = simulation_service_.runSimulation(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Base View") {
        cad::modules::DrawingRequest request;
        request.sourcePart = "Bracket";
        request.templateName = "ISO";
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            if (freecad_.isAvailable()) {
                freecad_.createDrawingStub(result.drawingId);
            }
            main_window_.setIntegrationStatus("Drawing created");
        } else {
            main_window_.setIntegrationStatus("Drawing failed");
        }
    } else if (command == "Import") {
        cad::interop::ImportRequest request;
        request.path = "C:/temp/model.step";
        request.format = cad::interop::FileFormat::Step;
        cad::interop::IoResult result = io_service_.importModel(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Export") {
        cad::interop::ExportRequest request;
        request.path = "C:/temp/model.step";
        request.format = cad::interop::FileFormat::Step;
        cad::interop::IoResult result = io_service_.exportModel(request);
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Export RFA" || command == "ExportRFA") {
        cad::interop::IoResult result = io_service_.exportBimRfa("C:/temp/model.rfa");
        main_window_.setIntegrationStatus(result.message);
    } else if (command == "Mate") {
        main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
    }
}

}  // namespace app
}  // namespace cad
