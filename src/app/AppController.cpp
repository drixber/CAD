#include "AppController.h"

#include <sstream>

namespace cad {
namespace app {

AppController::AppController() = default;

void AppController::initialize() {
    main_window_.initializeLayout();
    main_window_.setViewportStatus("3D viewport ready");
    main_window_.setWorkspaceMode("General");
    main_window_.setDocumentLabel("MainDocument");
    if (freecad_.initializeSession()) {
        freecad_.createDocument("MainDocument");
        main_window_.setIntegrationStatus("FreeCAD on");
    } else {
        main_window_.setIntegrationStatus("FreeCAD off");
    }
    techdraw_bridge_.initialize();
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
    
    // Register assembly in BOM registry for UI integration
    bom_service_.registerAssembly("MainAssembly", active_assembly_);

    assembly_manager_.setCacheLimit(300);
    assembly_manager_.enableBackgroundLoading(true);
    assembly_manager_.setLodMode(cad::core::LodMode::Simplified);
    assembly_manager_.setTargetFps(30.0);
    main_window_.setTargetFps(30);
    if (assembly_manager_.recommendedLod() == cad::core::LodMode::BoundingBoxes) {
        main_window_.setViewportStatus("LOD recommendation: bounding boxes");
    } else if (assembly_manager_.recommendedLod() == cad::core::LodMode::Simplified) {
        main_window_.setViewportStatus("LOD recommendation: simplified");
    } else {
        main_window_.setViewportStatus("LOD recommendation: full");
    }
    cad::core::PerfTimer timer("AssemblyLoad");
    cad::core::AssemblyLoadStats load_stats = assembly_manager_.loadAssembly("MainAssembly");
    cad::core::PerfSpan span = timer.finish();
    cad::core::CacheStats cache = assembly_manager_.cacheStats();
    main_window_.setIntegrationStatus("Cache: " + std::to_string(cache.entries) + "/" +
                                       std::to_string(cache.max_entries) +
                                       (load_stats.used_background_loading ? " (bg)" : " (fg)"));
    main_window_.setCacheStats(static_cast<int>(cache.entries),
                               static_cast<int>(cache.max_entries));
    main_window_.setViewportStatus(load_stats.used_background_loading
                                       ? "Background loading enabled"
                                       : "Background loading disabled");
    main_window_.setBackgroundLoading(load_stats.used_background_loading);
    main_window_.setViewportStatus("Assembly load " + std::to_string(span.elapsed_ms) + " ms");
    assembly_manager_.enqueueLoad("MainAssembly");
}

void AppController::bindCommands() {
    main_window_.setCommandHandler([this](const std::string& command) {
        executeCommand(command);
    });
    main_window_.setLodModeHandler([this](const std::string& mode) {
        if (mode == "full") {
            assembly_manager_.setLodMode(cad::core::LodMode::Full);
        } else if (mode == "simplified") {
            assembly_manager_.setLodMode(cad::core::LodMode::Simplified);
        } else {
            assembly_manager_.setLodMode(cad::core::LodMode::BoundingBoxes);
        }
        main_window_.setViewportStatus("LOD: " + mode);
    });
    main_window_.setBackgroundLoadingHandler([this](bool enabled) {
        assembly_manager_.enableBackgroundLoading(enabled);
        main_window_.setViewportStatus(enabled ? "Background loading enabled"
                                               : "Background loading disabled");
    });
    main_window_.setTargetFpsHandler([this](int fps) {
        assembly_manager_.setTargetFps(static_cast<double>(fps));
        main_window_.setViewportStatus("Target FPS " + std::to_string(fps));
    });
}

void AppController::executeCommand(const std::string& command) {
    if (command == "Parameters") {
        main_window_.setParameterSummary(buildParameterSummary(active_sketch_));
        main_window_.setParameterCount(static_cast<int>(active_sketch_.parameters().size()));
        main_window_.setViewportStatus("Parameters: " + std::to_string(active_sketch_.parameters().size()) + " items");
    } else if (command == "Flange" || command == "Bend" || command == "Unfold" || command == "Refold") {
        cad::modules::SheetMetalRequest request;
        request.targetPart = "Bracket";
        request.operation = cad::modules::SheetMetalOperation::Flange;
        cad::modules::SheetMetalResult result = sheet_metal_service_.applyOperation(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Sheet metal command queued");
    } else if (command == "Rectangular Pattern" || command == "Circular Pattern" || command == "Curve Pattern") {
        cad::modules::PatternRequest request;
        request.targetFeature = "Hole1";
        request.type = cad::modules::PatternType::Rectangular;
        request.instanceCount = 6;
        cad::modules::PatternResult result = pattern_service_.createPattern(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Pattern command queued");
    } else if (command == "Direct Edit" || command == "Freeform") {
        cad::modules::DirectEditRequest request;
        request.targetFeature = "Face1";
        request.operation = cad::modules::DirectEditOperation::MoveFace;
        cad::modules::DirectEditResult result = direct_edit_service_.applyEdit(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Direct edit queued");
    } else if (command == "Rigid Pipe" || command == "Flexible Hose" || command == "Bent Tube") {
        cad::modules::RoutingRequest request;
        request.targetAssembly = "MainAssembly";
        request.type = cad::modules::RoutingType::RigidPipe;
        cad::modules::RoutingResult result = routing_service_.createRoute(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Routing command queued");
    } else if (command == "Simplify") {
        cad::modules::SimplifyRequest request;
        request.targetAssembly = "MainAssembly";
        request.replacementType = "Cylinder";
        cad::modules::SimplifyResult result = simplify_service_.simplify(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Simplify queued");
    } else if (command == "Illustration" || command == "Rendering" || command == "Animation") {
        cad::modules::VisualizationRequest request;
        request.targetPart = "Bracket";
        request.mode = cad::modules::VisualizationMode::Rendering;
        cad::modules::VisualizationResult result = visualization_service_.runVisualization(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Visualization queued");
    } else if (command == "MBD Note" || command == "MbdNote") {
        cad::modules::MbdRequest request;
        request.targetPart = "Bracket";
        request.note = "GD&T: profile tolerance";
        cad::modules::MbdResult result = mbd_service_.applyMbd(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("MBD annotation queued");
        cad::mbd::PmiDataSet pmi = mbd_service_.buildDefaultPmi("Bracket");
        (void)pmi;
    } else if (command == "Simulation" || command == "Stress Analysis") {
        cad::modules::SimulationRequest request;
        request.targetAssembly = "MainAssembly";
        request.type = cad::modules::SimulationType::FEA;
        cad::modules::SimulationResult result = simulation_service_.runSimulation(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Simulation queued");
    } else if (command == "Interference") {
        cad::core::InterferenceResult result = interference_checker_.checkAssembly(active_assembly_);
        main_window_.setIntegrationStatus(result.message);
        if (result.has_interference) {
            std::string status = "Interference: " + std::to_string(result.overlap_count) + " overlap(s)";
            if (!result.interference_pairs.empty()) {
                status += " (" + result.interference_pairs[0].part_a_name + 
                         " <-> " + result.interference_pairs[0].part_b_name + ")";
            }
            main_window_.setViewportStatus(status);
        } else {
            main_window_.setViewportStatus("No interference detected");
        }
    } else if (command == "Base View" || command == "BaseView") {
        cad::modules::DrawingRequest request;
        request.sourcePart = "Bracket";
        request.templateName = "ISO";
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            if (freecad_.isAvailable()) {
                freecad_.createDrawingStub(result.drawingId);
            }
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId);
            document.bom = bom_service_.getBomForAssembly("MainAssembly");
            if (!document.sheets.empty()) {
                document.annotations =
                    annotation_service_.buildDefaultAnnotations(document.sheets.front().name);
                document.dimensions =
                    annotation_service_.buildDefaultDimensions(document.sheets.front().name);
            }
            techdraw_bridge_.syncDrawing(document);
            techdraw_bridge_.syncDimensions(document);
            techdraw_bridge_.syncAssociativeLinks(document);
            associative_link_service_.updateFromModel(document, document.source_model_id);
            main_window_.setIntegrationStatus("Drawing created");
            main_window_.setViewportStatus("Drawing view created");
        } else {
            main_window_.setIntegrationStatus("Drawing failed");
            main_window_.setViewportStatus("Drawing view failed");
        }
    } else if (command == "Parts List" || command == "PartsList") {
        if (bom_service_.hasAssembly("MainAssembly")) {
            std::vector<cad::drawings::BillOfMaterialsItem> bom = 
                bom_service_.getBomForAssembly("MainAssembly");
            std::string bom_summary = "BOM: " + std::to_string(bom.size()) + " items";
            for (const auto& item : bom) {
                bom_summary += ", " + item.part_name + " x" + std::to_string(item.quantity);
            }
            main_window_.setIntegrationStatus(bom_summary);
            main_window_.setViewportStatus("BOM displayed: " + std::to_string(bom.size()) + " items");
            
            // Update property panel with BOM items
            #ifdef CAD_USE_QT
            cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
            if (qt_window) {
                cad::ui::QtPropertyPanel* panel = qt_window->propertyPanel();
                if (panel) {
                    QList<cad::ui::BomItem> bom_items;
                    for (const auto& item : bom) {
                        cad::ui::BomItem bom_item;
                        bom_item.part_name = QString::fromStdString(item.part_name);
                        bom_item.quantity = item.quantity;
                        bom_item.part_number = QString::fromStdString(item.part_number);
                        bom_items.append(bom_item);
                    }
                    panel->setBomItems(bom_items);
                }
            }
            #endif
        } else {
            main_window_.setIntegrationStatus("BOM: No assembly registered");
            main_window_.setViewportStatus("BOM not available");
        }
    } else if (command == "Dimension") {
        cad::modules::DrawingRequest request;
        request.sourcePart = "Bracket";
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId);
            if (!document.sheets.empty()) {
                document.dimensions = 
                    annotation_service_.buildDefaultDimensions(document.sheets.front().name);
                techdraw_bridge_.applyDimensions(document);
                main_window_.setIntegrationStatus("Dimensions added");
                main_window_.setViewportStatus("Dimensions: " + 
                    std::to_string(document.dimensions.size()) + " items");
            }
        }
    } else if (command == "Section" || command == "SectionView") {
        cad::modules::DrawingRequest request;
        request.sourcePart = "Bracket";
        request.templateName = "ISO";
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId);
            main_window_.setIntegrationStatus("Section view created");
            main_window_.setViewportStatus("Section view: " + result.drawingId);
        }
    } else if (command == "Styles") {
        cad::drawings::DrawingStyleSet iso_styles = drawing_service_.isoStyles();
        cad::drawings::DrawingStyleSet ansi_styles = drawing_service_.ansiStyles();
        cad::drawings::DrawingStyleSet default_styles = drawing_service_.defaultStyles();
        std::string style_summary = "Styles: ISO (" + 
            std::to_string(iso_styles.line_styles.size()) + " lines, " +
            std::to_string(iso_styles.text_styles.size()) + " texts), ANSI (" +
            std::to_string(ansi_styles.line_styles.size()) + " lines, " +
            std::to_string(ansi_styles.text_styles.size()) + " texts)";
        main_window_.setIntegrationStatus(style_summary);
        main_window_.setViewportStatus("Style editor available");
        
        // Update property panel with style information
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window) {
            cad::ui::QtPropertyPanel* panel = qt_window->propertyPanel();
            if (panel) {
                QStringList presets = {"Default", "ISO", "ANSI", "JIS"};
                panel->setStylePresets(presets);
                
                QString style_info = QString("Line styles: %1, Text styles: %2, Dimension styles: %3, Hatch styles: %4")
                    .arg(default_styles.line_styles.size())
                    .arg(default_styles.text_styles.size())
                    .arg(default_styles.dimension_styles.size())
                    .arg(default_styles.hatch_styles.size());
                panel->setCurrentStylePreset(QString::fromStdString("Default"));
                panel->setStyleInfo(style_info);
            }
        }
        #endif
    } else if (command == "MBD View" || command == "MbdView") {
        cad::modules::MbdRenderRequest render_request;
        render_request.part_id = "Bracket";
        render_request.show_annotations = true;
        render_request.show_datums = true;
        render_request.show_tolerances = true;
        cad::modules::MbdRenderResult render_result = mbd_service_.prepareForRendering(render_request);
        if (render_result.success) {
            std::string mbd_summary = "MBD: " + 
                std::to_string(render_result.visible_annotations.size()) + " annotations, " +
                std::to_string(render_result.visible_datums.size()) + " datums, " +
                std::to_string(render_result.visible_tolerances.size()) + " tolerances";
            main_window_.setIntegrationStatus(mbd_summary);
            main_window_.setViewportStatus("MBD view prepared for rendering");
        } else {
            main_window_.setIntegrationStatus("MBD: No PMI data available");
            main_window_.setViewportStatus("MBD view unavailable");
        }
    } else if (command == "Import") {
        cad::interop::IoJob job;
        job.path = "C:/temp/model.step";
        job.format = "STEP";
        if (!io_pipeline_.supportsFormat(job.format, false)) {
            main_window_.setIntegrationStatus("Import format unsupported");
        } else {
            cad::interop::IoJobResult result = io_pipeline_.importJob(job);
            main_window_.setIntegrationStatus(result.message);
            main_window_.setViewportStatus("Import queued");
        }
    } else if (command == "Export") {
        cad::interop::IoJob job;
        job.path = "C:/temp/model.step";
        job.format = io_pipeline_.supportedFormats().front().format;
        if (!io_pipeline_.supportsFormat(job.format, true)) {
            main_window_.setIntegrationStatus("Export format unsupported");
        } else {
            cad::interop::IoJobResult result = io_pipeline_.exportJob(job);
            main_window_.setIntegrationStatus(result.message + " (" + job.format + ")");
            main_window_.setViewportStatus("Export queued");
        }
    } else if (command == "Export RFA" || command == "ExportRFA") {
        cad::interop::IoResult result = io_service_.exportBimRfa("C:/temp/model.rfa");
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("RFA export queued");
    } else if (command == "Mate") {
        main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
        main_window_.setViewportStatus("Mate command ready");
    } else if (command == "LoadAssembly") {
        assembly_manager_.enqueueLoad("MainAssembly");
        main_window_.setLoadProgress(0);
        main_window_.setViewportStatus("Assembly load queued");
    } else if (command == "Place") {
        cad::core::AssemblyLoadJob job = assembly_manager_.pollLoadProgress();
        if (!job.path.empty()) {
            main_window_.setViewportStatus("Assembly load " + std::to_string(job.progress) + "%");
            main_window_.setLoadProgress(job.progress);
        } else {
            main_window_.setViewportStatus("No assembly load in progress");
        }
    } else if (command == "Measure") {
        main_window_.setIntegrationStatus("Measure tool ready");
        main_window_.setViewportStatus("Select geometry to measure");
    } else if (command == "SectionAnalysis") {
        main_window_.setIntegrationStatus("Section analysis ready");
        main_window_.setViewportStatus("Section analysis: select plane");
    } else if (command == "Line" || command == "Rectangle" || command == "Circle" || command == "Arc") {
        main_window_.setIntegrationStatus("Sketch tool: " + command);
        main_window_.setViewportStatus("Sketch " + command + " command active");
    } else if (command == "Constraint") {
        main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
        main_window_.setViewportStatus("Constraints: " + std::to_string(active_sketch_.constraints().size()) + " items");
    } else if (command == "Extrude" || command == "Revolve" || command == "Loft" || command == "Hole" || command == "Fillet") {
        main_window_.setIntegrationStatus("Feature: " + command);
        main_window_.setViewportStatus("Part feature " + command + " command active");
    } else if (command == "Flush" || command == "Angle") {
        main_window_.setIntegrationStatus("Mate type: " + command);
        main_window_.setViewportStatus("Assembly mate " + command + " command active");
    } else if (command == "Pattern") {
        main_window_.setIntegrationStatus("Pattern command ready");
        main_window_.setViewportStatus("Assembly pattern command active");
    } else if (command == "Visibility" || command == "Appearance" || command == "Environment") {
        main_window_.setIntegrationStatus("View: " + command);
        main_window_.setViewportStatus("View " + command + " command active");
    } else if (command == "AddIns") {
        main_window_.setIntegrationStatus("Add-ins manager");
        main_window_.setViewportStatus("Add-ins panel available");
    } else {
        // Unknown command - provide feedback
        main_window_.setIntegrationStatus("Command: " + command);
        main_window_.setViewportStatus("Command executed: " + command);
    }
}

}  // namespace app
}  // namespace cad
